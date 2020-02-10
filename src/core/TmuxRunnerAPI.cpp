#include <KConfigCore/KConfigGroup>
#include "TmuxRunnerAPI.h"


TmuxRunnerAPI::TmuxRunnerAPI(const KConfigGroup &config) : config(config) {

}

QString TmuxRunnerAPI::filterPath(QString path) {
    if (path.isEmpty()) {
        return QDir::homePath();
    }
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

QStringList TmuxRunnerAPI::fetchTmuxSessions() {
    QStringList tmuxSessions;
    QProcess process;
    process.start(fetchProgram, fetchArgs);
    process.waitForFinished(1000);
    while (process.canReadLine()) {
        const QString line = process.readLine();
        if (line.contains(lineSeparator)) {
            tmuxSessions.append(line.split(lineSeparator).first());
        }
    }
    return tmuxSessions;
}

void TmuxRunnerAPI::executeAttatchCommand(QString &program, const QString &target) {
    QStringList args;
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
}

void TmuxRunnerAPI::executeCreateCommand(QString &program,
                                         const QString &target,
                                         const QMap<QString, QVariant> &data) {
    QStringList args;
    if (program == "yakuake") {
        if (data.value("action") != "tmuxinator")
            args.append({"-t", target, "-e", "tmux", "new-session", "-s", target});
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

QStringList TmuxRunnerAPI::fetchTmuxinatorConfigs() {
    QStringList tmuxinatorConfigs;
    QProcess isTmuxinatorInstalledProcess;
    isTmuxinatorInstalledProcess.start("whereis", QStringList() << "-b" << "tmuxinator");
    isTmuxinatorInstalledProcess.waitForFinished();
    if (QString(isTmuxinatorInstalledProcess.readAll()) == "tmuxinator:\n") {
        // Disable tmuxinator until the user installs and enables it
        config.writeEntry("enable_tmuxinator", false);
        return tmuxinatorConfigs;
    }
    // Fetch the available configurations
    QProcess process;
    process.start("tmuxinator ls");
    process.waitForFinished(2);
    const QString res = process.readAll();
    if (res.split('\n').size() == 2) {
        return tmuxinatorConfigs;
    }
    const auto _entries = res.split('\n');
    if (_entries.size() < 2) {
        return tmuxinatorConfigs;
    }
    const auto entries = _entries.at(1).split(' ');
    for (const auto &entry:entries) {
        if (!entry.isEmpty()) {
            tmuxinatorConfigs.append(entry);
        }
    }
    return tmuxinatorConfigs;
}

void TmuxRunnerAPI::parseQueryFlags(QString &term, QString &openIn, QMap<QString, QVariant> &data) {
    // Flag at end of query or just a flag with no session name
    if (term.size() >= 2 && term.contains(queryHasFlag)) {
        const QString flag = queryHasFlag.match(term).captured(1);
        QString flagValue = flags.value(flag);
        if (!flagValue.isEmpty()) {
            data.insert("program", flagValue);
            openIn = " in " + flagValue.remove("-session");
        } else {
            openIn = " default (invalid flag)";
        }
        term.remove(queryHasFlag);
    }
}
