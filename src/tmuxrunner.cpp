#include "tmuxrunner.h"

// KF
#include <KLocalizedString>
#include <QtGui/QtGui>
#include <KSharedConfig>


TmuxRunner::TmuxRunner(QObject *parent, const QVariantList &args)
        : Plasma::AbstractRunner(parent, args) {
    setObjectName(QStringLiteral("TmuxRunner"));
}

TmuxRunner::~TmuxRunner() = default;


void TmuxRunner::init() {
    reloadConfiguration();
    connect(this, SIGNAL(prepare()), this, SLOT(prepareForMatchSession()));
}

void TmuxRunner::prepareForMatchSession() {
    tmuxSessions.clear();
    QProcess process;
    process.start("tmux", QStringList() << "ls");
    process.waitForFinished();
    while (process.canReadLine()) {
        QString line = process.readLine();
        if (line.contains(':')) {
            tmuxSessions.append(line.split(':').first());
        }
    }

}

void TmuxRunner::reloadConfiguration() {
    config = KSharedConfig::openConfig("krunnerrc")->group("Runners").group("TmuxRunner");
    enableTmuxinator = config.readEntry("enable_tmuxinator", "true") == "true";
    enableFlags = config.readEntry("enable_flags", "true") == "true";
    enableNewSessionByPartlyMatch = config.readEntry("add_new_by_part_match") == "true";
    defaultProgram = config.readEntry("program", "konsole");
    if (enableTmuxinator) {
        // Check if tmuxinator is installed
        QProcess isTmuxinatorInstalledProcess;
        isTmuxinatorInstalledProcess.start("whereis", QStringList() << "-b" << "tmuxinator");
        isTmuxinatorInstalledProcess.waitForFinished();
        if (QString(isTmuxinatorInstalledProcess.readAll()) == "tmuxinator:\n") {
            // Disable tmuxinator until the user installs and enables it
            enableTmuxinator = false;
            config.writeEntry("enable_tmuxinator", "false");
            return;
        }
        // Fetch the available configurations
        QProcess process;
        process.start("tmuxinator ls");
        process.waitForFinished();
        const QString res = process.readAll();
        if (res.split('\n').size() == 2) return;
        const auto _entries = res.split('\n');
        if (_entries.size() < 2) return;
        const auto entries = _entries.at(1).split(' ');
        for (const auto &entry:entries) {
            if (entry.isEmpty()) continue;
            tmuxinatorConfigs.append(entry);
        }
    }
}

void TmuxRunner::match(Plasma::RunnerContext &context) {
    if (!context.isValid()) return;
    QString term = context.query();
    if (!term.startsWith("tmux")) return;
    term.replace(QRegExp("tmux *"), "");
    QList<Plasma::QueryMatch> matches;
    bool exactMatch = false;
    bool tmuxinator = false;

    QMap<QString, QVariant> data = {};
    QStringList attached;

    // Flags to open other terminal emulator
    QString openIn = "";
    if (enableFlags) {
        // Flag at end of query or just a flag with no session name
        if (term.contains(QRegExp(" -([a-z])$")) || (term.size() == 2 && term.contains(QRegExp("-([a-z])$")))) {
            QRegExp exp("-([a-z])$");
            exp.indexIn(term);
            QString match = exp.capturedTexts().last();
            if (match == "k") { data.insert("program", "konsole"); }
            else if (match == "y") { data.insert("program", "yakuake-session"); }
            else if (match == "t") { data.insert("program", "terminator"); }
            else if (match == "s") { data.insert("program", "st"); }
            else if (match == "c") { data.insert("program", "custom"); }
            term.remove(QRegExp(" ?-([a-z])$"));
            openIn = " in " + data.value("program").toString();
        }
    }
    // Session with Tmuxinator
    if (enableTmuxinator && term.startsWith("inator")) {
        QRegExp regExp(R"(inator(?: (\w+) *(.+)?)?)");
        regExp.indexIn(term);
        QString filter = regExp.capturedTexts().at(1);
        term.replace(QRegExp("^inator *"), "");
        tmuxinator = true;
        for (const auto &tmuxinatorConfig:tmuxinatorConfigs) {
            if (tmuxinatorConfig.startsWith(filter)) {
                if (!tmuxSessions.contains(tmuxinatorConfig)) {
                    matches.append(
                            createMatch("Create Tmuxinator  " + tmuxinatorConfig + openIn,
                                        {
                                                {"action",  "tmuxinator"},
                                                {"program", data.value("program", defaultProgram)},
                                                {"args",    regExp.capturedTexts().last()},
                                                {"target",  tmuxinatorConfig}
                                        }, 1)
                    );
                } else {
                    attached.append(tmuxinatorConfig);
                    data.insert("action", "attach");
                    data.insert("target", tmuxinatorConfig);
                    matches.append(
                            createMatch("Attach Tmuxinator  " + tmuxinatorConfig + QString(openIn).replace("-session", ""), data, 0.99)
                    );
                }
            }
        }
    }
    // Attach to session options
    const auto queryName = term.contains(' ') ? term.split(' ').first() : term;
    for (const auto &session:tmuxSessions) {
        if (session.startsWith(queryName)) {
            if (session == queryName) exactMatch = true;
            if (attached.contains(session)) continue;
            data.insert("action", "attach");
            data.insert("target", session);
            matches.append(
                    createMatch("Attach to " + session + QString(openIn).replace("-session", ""), data,
                                (float) queryName.length() / session.length())
            );
        }
    }
    // New session
    if (!exactMatch && (matches.isEmpty() || enableNewSessionByPartlyMatch)) {
        // Name and optional path, Online tester : https://regex101.com/r/FdZcIZ/1
        QRegExp regex(R"(^([\w-]+)(?: +(.+)?)?$)");
        regex.indexIn(term);
        QStringList texts = regex.capturedTexts();
        int relevance = matches.empty() ? 1 : 0;
        data.insert("action", "new");
        data.insert("target", texts.at(1));

        // No path
        if (texts.at(2).isEmpty()) {
            if (!texts.at(1).isEmpty() || !tmuxinator) {
                matches.append(
                        createMatch("New session " + texts.at(1) + QString(openIn).replace("-session", ""), data, relevance)
                );
            }
            // With path
        } else {
            data.insert("path", texts.at(2));
            matches.append(
                    createMatch("New session " + texts.at(1) + " in " + texts.at(2) +
                                QString(openIn).replace("-session", ""), data, relevance)
            );
        }
    }

    context.addMatches(matches);
}

void TmuxRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) {
    Q_UNUSED(context)

    QMap<QString, QVariant> data = match.data().toMap();
    QString program = data.value("program", defaultProgram).toString();
    const QString target = data.value("target").toString();
    QStringList args;

    if (data.value("action") == "attach") {
        if (program == "yakuake-session") {
            args.append({"-t", target, "-e", "tmux", "attach-session", "-t", target});
        } else if (program == "terminator") {
            args.append({"-x", "tmux", "a", "-t", target});
        } else if (program == "st") {
            args.append({"tmux", "attach-session", "-t", target});
        } else if (program == "custom") {
            const auto customConfig = config.group("Custom");
            program = customConfig.readEntry("program");
            QString arg = customConfig.readEntry("attach_params");
            arg.replace("%name", target);
            args.append(arg.split(' '));
        } else {
            args.append({"-e", "tmux", "a", "-t", target});
        }
        QProcess::startDetached(program, args);
    } else {
        if (program == "yakuake") {
            if (data.value("action") != "tmuxinator") args.append({"-t", target, "-e", "tmux", "new-session", "-s", target});
            else args.append({"-t", target, "-e", "tmuxinator", target});
        } else if (program == "terminator") {
            if (data.value("action") != "tmuxinator") args.append({"-x", "tmux", "new-session", "-s", target});
            else args.append({"-x", "tmuxinator", target});
        } else if (program == "st") {
            if (data.value("action") != "tmuxinator") args.append({"tmux", "new-session", "-s", target});
            else args.append({"tmuxinator", target});
        } else if (program == "custom") {
            const auto customConfig = config.group("Custom");
            program = customConfig.readEntry("program");
            QString arg = customConfig.readEntry("new_params");
            arg.replace("%name", target);
            arg.replace("%path", filterPath(data.value("path", "").toString()));
            args.append(arg.split(' '));
        } else {
            args.append({"-e", "tmux", "new-session", "-s", target});
        }
        // Add path option
        if (program != "custom") {
            args.append({"-c", filterPath(data.value("path", "").toString())});
        }

        // Remove everything after tmux and replace (workaround for custom)
        if (data.value("action") == "tmuxinator") {
            const int idx = args.indexOf("tmux");
            while (args.size() > idx) {
                args.removeLast();
            }
            args.append({"tmuxinator", target});
            args.append(data.value("args").toString().split(' '));
        }

        // If new session is started but no name provided
        if (target.isEmpty()) {
            args.removeOne("-s");
            args.removeOne("-t");
        }
        args.removeAll("");

        QProcess::startDetached(program, args);
    }
}


QString TmuxRunner::filterPath(QString path) {
    if (path.isEmpty()) return QDir::homePath();
    const auto shortcutConfig = config.group("Shortcuts");
    for (const auto &key:shortcutConfig.keyList()) {
        path.replace(key, shortcutConfig.readEntry(key));
    }
    if (path.startsWith('~')) {
        path.replace('~', QDir::homePath());
    } else if (!path.startsWith('/')) {
        path.insert(0, QDir::homePath() + "/");
    }
    return path;
}

Plasma::QueryMatch TmuxRunner::createMatch(const QString &text, const QMap<QString, QVariant> &data, float relevance) {
    Plasma::QueryMatch match(this);
    match.setIconName("utilities-terminal");
    match.setText(text);
    match.setData(data);
    match.setRelevance(relevance);
    return match;
}


K_EXPORT_PLASMA_RUNNER(tmuxrunner, TmuxRunner)

// needed for the QObject subclass declared as part of K_EXPORT_PLASMA_RUNNER
#include "tmuxrunner.moc"
