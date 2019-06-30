/*
   Copyright 2019 by Alex <alexkp12355@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

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
    config = KSharedConfig::openConfig("krunnerrc")->group("Runners").group("TmuxRunner");
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

void TmuxRunner::match(Plasma::RunnerContext &context) {
    if (!context.isValid()) return;
    QString term = context.query();
    if (!term.startsWith("tmux")) return;
    term.replace(QRegExp("tmux *"), "");
    QList<Plasma::QueryMatch> matches;
    bool exactMatch = false;

    QString tmpTerm;
    if (term.contains(' ')) {
        tmpTerm = term.split(' ').first();
    } else {
        tmpTerm = term;
    }

    QMap<QString, QVariant> data;

    // Flags to open other terminal emulator
    QString openIn = "";
    if (config.readEntry("enable_flags", "true") == "true") {
        if (term.contains(QRegExp(" -([a-z])$")) || (term.size() == 2 && term.contains(QRegExp("-([a-z])$")))) {
            QRegExp exp("-([a-z])$");
            exp.indexIn(term);
            QString match = exp.capturedTexts().last();
            if (match == "k") { data.insert("program", "konsole"); }
            else if (match == "y") { data.insert("program", "yakuake"); }
            else if (match == "t") { data.insert("program", "terminator"); }
            else if (match == "s") { data.insert("program", "st"); }
            else if (match == "c") { data.insert("program", "custom"); }
            tmpTerm.remove(QRegExp(" ?-([a-z])$"));
            term.remove(QRegExp(" ?-([a-z])$"));
            openIn = " in " + data.value("program").toString();
        }
    }
    // Attach to session options
    for (const auto &session:tmuxSessions) {
        if (session.startsWith(tmpTerm)) {
            if (session == tmpTerm) exactMatch = true;
            data.insert("action", "attach");
            data.insert("target", session);
            matches.append(
                    addMatch("Attatch to " + session + openIn, data,
                             (float) tmpTerm.length() / session.length()));
        }
    }
    // New session
    if (!exactMatch && (matches.isEmpty() || config.readEntry("add_new_by_part_match", "false") == "true")) {
        // Name, spaces, path
        QRegExp regex(R"(^([\w-]+)(?: +(.+)?)?$)");
        regex.indexIn(term);
        QStringList texts = regex.capturedTexts();
        int relevance = matches.empty() ? 1 : 0;
        data.insert("action", "new");
        data.insert("target", texts.at(1));
        // No path
        if (texts.size() == 2 || texts.at(2).isEmpty()) {
            matches.append(addMatch("New session " + texts.at(1) + openIn, data, relevance));
            // With path
        } else if (texts.size() == 3) {
            data.insert("path", texts.at(2));
            matches.append(
                    addMatch("New session " + texts.at(1) + " in " + texts.at(2) + openIn, data, relevance));
        }
    }

    context.addMatches(matches);
}

void TmuxRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) {
    Q_UNUSED(context)
    QMap<QString, QVariant> data = match.data().toMap();
    QString program = data.value("program", config.readEntry("program", "konsole")).toString();
    const QString target = data.value("target").toString();
    if (data.value("action") == "attach") {
        QStringList args = {"-e", "tmux", "a", "-t", target};
        if (program == "yakuake") {
            program = "yakuake-session";
            args.clear();
            args.append({"-t", target, "-e", "tmux", "attach-session", "-t", target});
        } else if (program == "terminator") {
            args.clear();
            args.append({"-x", "tmux", "a", "-t", target});
        } else if (program == "st") {
            args.clear();
            args.append({"tmux", "attach-session", "-t", target});
        } else if (program == "custom") {
            const auto customConfig = config.group("Custom");
            program = customConfig.readEntry("program");
            args.clear();
            QString arg = customConfig.readEntry("attach_params");
            arg.replace("%name", target);
            args.append(arg.split(' '));
        }
        QProcess::startDetached(program, args);
    } else {
        QStringList args = {"-e", "tmux", "new-session", "-s", target};
        if (program == "yakuake") {
            program = "yakuake-session";
            args.clear();
            args.append({"-t", target, "-e", "tmux", "new-session", "-s", target});
        } else if (program == "terminator") {
            args.clear();
            args.append({"-x", "tmux", "new-session", "-s", target});
        } else if (program == "st") {
            args.clear();
            args.append({"tmux", "new-session", "-s", target});
        } else if (program == "custom") {
            const auto customConfig = config.group("Custom");
            program = customConfig.readEntry("program");
            args.clear();
            QString arg = customConfig.readEntry("new_params");
            arg.replace("%name", target);
            arg.replace("%path", filterPath(data.value("path", "").toString()));

            args.append(arg.split(' '));
        }
        // Add path option
        if (program != "custom") {
            args.append({"-c", filterPath(data.value("path", "").toString())});
        }
        // If new session is started but no name provided
        if (target.isEmpty()) {
            args.removeOne("-s");
            args.removeOne("-t");
            args.removeAll("");
        }
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


Plasma::QueryMatch TmuxRunner::addMatch(const QString &text, const QMap<QString, QVariant> &data, float relevance) {
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
