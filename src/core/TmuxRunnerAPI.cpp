#include "TmuxRunnerAPI.h"

#include <KConfigGroup>
#include <KMacroExpander>
#include <KNotifications/KNotification>
#include <KShell>
#include <QDir>
#include <QProcess>

TmuxRunnerAPI::TmuxRunnerAPI(const KConfigGroup &config)
    : config(config)
{
}

QString TmuxRunnerAPI::filterPath(QString path)
{
    if (path.isEmpty()) {
        return QDir::homePath();
    }
    const auto shortcutConfig = config.group("Shortcuts");
    const auto keyList = shortcutConfig.keyList();
    for (const auto &key : keyList) {
        path.replace(key, shortcutConfig.readEntry(key));
    }
    if (path.startsWith('~')) {
        path.replace('~', QDir::homePath());
    } else if (!path.startsWith('/')) {
        path.insert(0, QDir::homePath() + "/");
    }
    return path;
}

QStringList TmuxRunnerAPI::fetchTmuxSessions()
{
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

void TmuxRunnerAPI::executeAttatchCommand(QString &program, const QString &target)
{
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
        QHash<QString, QString> variables = {{"name", target}};
        QString arg = customConfig.readEntry("attach_params");
        arg = KMacroExpander::expandMacrosShellQuote(arg, variables);
        const auto splitPair = splitArguments(arg);
        if (splitPair.first) {
            args.append(splitPair.second);
        } else {
            showErrorNotification("The command line arguments from the custom config are invalid!");
            return;
        }
    } else {
        args.append({"-e", "tmux", "a", "-t", target});
    }
    QProcess::startDetached(program, args);
}

void TmuxRunnerAPI::executeCreateCommand(QString &program, const QString &target, const QMap<QString, QVariant> &data)
{
    QStringList args;
    const QString path = filterPath(data.value("path").toString());
    if (program == "yakuake") {
        if (data.value("action") != "tmuxinator")
            args.append({"-t", target, "-e", "tmux", "new-session", "-s", target});
        else
            args.append({"-t", target, "-e", "tmuxinator", target});
    } else if (program == "terminator") {
        if (data.value("action") != "tmuxinator")
            args.append({"-x", "tmux", "new-session", "-s", target});
        else
            args.append({"-x", "tmuxinator", target});
    } else if (program == "st") {
        if (data.value("action") != "tmuxinator")
            args.append({"tmux", "new-session", "-s", target});
        else
            args.append({"tmuxinator", target});
    } else if (program == "custom") {
        const auto customConfig = config.group("Custom");
        program = customConfig.readEntry("program");
        QString arg = customConfig.readEntry("new_params");
        QHash<QString, QString> variables = {{"name", target}, {"path", path}};
        arg = KMacroExpander::expandMacrosShellQuote(arg, variables);
        const auto splitPair = splitArguments(arg);
        if (splitPair.first) {
            args.append(splitPair.second);
        } else {
            showErrorNotification("The command line arguments from the custom config are invalid!");
            return;
        }
    } else {
        args.append({"-e", "tmux", "new-session", "-s", target});
    }
    // Add path option
    if (program != "custom") {
        args.append({"-c", path});
    }

    // Remove everything after tmux and replace (workaround for custom)
    if (data.value("action") == "tmuxinator") {
        const int idx = args.indexOf("tmux");
        while (args.size() > idx) {
            args.removeLast();
        }
        args.append({"tmuxinator", target});
        const auto splitPair = splitArguments(data.value("args").toString());
        if (splitPair.first) {
            args.append(splitPair.second);
        } else {
            showErrorNotification("The command extra arguments for tmuxinator are invalid, continuing without!");
        }
    }

    // If new session is started but no name provided
    if (target.isEmpty()) {
        args.removeOne("-s");
        args.removeOne("-t");
    }
    args.removeAll(QString());

    QProcess::startDetached(program, args);
}

QStringList TmuxRunnerAPI::fetchTmuxinatorConfigs()
{
    QStringList tmuxinatorConfigs;
    QProcess isTmuxinatorInstalledProcess;
    isTmuxinatorInstalledProcess.start("whereis", QStringList{"-b", "tmuxinator"});
    isTmuxinatorInstalledProcess.waitForFinished();
    if (QString(isTmuxinatorInstalledProcess.readAll()) == "tmuxinator:\n") {
        // Disable tmuxinator until the user installs and enables it
        config.writeEntry("enable_tmuxinator", false);
        return tmuxinatorConfigs;
    }
    // Fetch the available configurations
    QProcess process;
    process.start("tmuxinator", QStringList{"ls"});
    process.waitForFinished(2000);
    const QString res = process.readAll();
    if (res.split('\n').size() == 2) {
        return tmuxinatorConfigs;
    }
    const auto _entries = res.split('\n');
    if (_entries.size() < 2) {
        return tmuxinatorConfigs;
    }
    const auto entries = _entries.at(1).split(' ');
    for (const auto &entry : entries) {
        if (!entry.isEmpty()) {
            tmuxinatorConfigs.append(entry);
        }
    }
    return tmuxinatorConfigs;
}

QString TmuxRunnerAPI::parseQueryFlags(QString &term, QString &openIn)
{
    // Flag at end of query or just a flag with no session name
    QString program;
    if (term.size() >= 2 && term.contains(queryHasFlag)) {
        const QString flag = queryHasFlag.match(term).captured(1);
        QString flagValue = flags.value(flag);
        if (!flagValue.isEmpty()) {
            program = flagValue;
            openIn = " in " + flagValue.remove("-session");
        } else {
            openIn = " default (invalid flag)";
        }
        term.remove(queryHasFlag);
    }

    return program;
}

QPair<bool, QStringList> TmuxRunnerAPI::splitArguments(const QString &argument)
{
    KShell::Errors splitArgsError;
    const auto args = KShell::splitArgs(argument, KShell::AbortOnMeta, &splitArgsError);
    return {splitArgsError == KShell::Errors::NoError, args};
}

void TmuxRunnerAPI::showErrorNotification(const QString &msg)
{
    KNotification::event(KNotification::Error, QStringLiteral("Tmux Runner"), msg, QStringLiteral("utility-terminal"));
}
