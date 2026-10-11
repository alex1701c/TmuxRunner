#include "TmuxRunnerAPI.h"

#include <KConfigGroup>
#include <KMacroExpander>
#include <KNotification>
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
        path.replace(0, 1, QDir::homePath());
    } else if (!path.startsWith('/')) {
        path.insert(0, QDir::homePath() + "/");
    }
    return path;
}

QStringList TmuxRunnerAPI::fetchTmuxSessions()
{
    QStringList tmuxSessions;
    QProcess process;
    process.start(QStringLiteral("tmux"), {QStringLiteral("list-sessions"), QStringLiteral("-F"), QStringLiteral("#{session_name}")});
    process.waitForFinished(1000);
    while (process.canReadLine()) {
        const QString line = QString::fromUtf8(process.readLine()).trimmed();
        if (!line.isEmpty()) {
            tmuxSessions.append(line);
        }
    }
    return tmuxSessions;
}

QStringList TmuxRunnerAPI::wrapInTerminal(const QString &program, const QString &title, const QStringList &command)
{
    if (program == QLatin1String("yakuake-session")) {
        QStringList args;
        if (!title.isEmpty()) {
            args.append({"-t", title});
        }
        return args << "-e" << command;
    } else if (program == QLatin1String("terminator")) {
        return QStringList{"-x"} + command;
    } else if (program == QLatin1String("st") || program == QLatin1String("kitty")) {
        // These take the command to run as positional arguments
        return command;
    }
    return QStringList{"-e"} + command;
}

void TmuxRunnerAPI::executeAttachCommand(const QString &program, const QString &target)
{
    if (program == QLatin1String("custom")) {
        const auto customConfig = config.group("Custom");
        QHash<QString, QString> variables = {{"name", target}};
        const QString arg = KMacroExpander::expandMacrosShellQuote(customConfig.readEntry("attach_params"), variables);
        const auto splitPair = splitArguments(arg);
        if (!splitPair.first) {
            showErrorNotification("The command line arguments from the custom config are invalid!");
            return;
        }
        QProcess::startDetached(customConfig.readEntry("program"), splitPair.second);
        return;
    }
    // The "=" prefix makes tmux only accept an exact session name match
    QProcess::startDetached(program, wrapInTerminal(program, target, {"tmux", "attach-session", "-t", "=" + target}));
}

void TmuxRunnerAPI::executeCreateCommand(const QString &program, const QString &target, const QMap<QString, QVariant> &data)
{
    const QString path = filterPath(data.value("path").toString());
    const bool tmuxinator = data.value("action") == QLatin1String("tmuxinator");

    QStringList tmuxinatorCommand;
    if (tmuxinator) {
        tmuxinatorCommand = {"tmuxinator", target};
        const auto splitPair = splitArguments(data.value("args").toString());
        if (splitPair.first) {
            tmuxinatorCommand.append(splitPair.second);
        } else {
            showErrorNotification("The command extra arguments for tmuxinator are invalid, continuing without!");
        }
    }

    if (program != QLatin1String("custom")) {
        QStringList command = tmuxinatorCommand;
        if (!tmuxinator) {
            command = {"tmux", "new-session"};
            if (!target.isEmpty()) {
                command.append({"-s", target});
            }
            command.append({"-c", path});
        }
        QProcess::startDetached(program, wrapInTerminal(program, target, command));
        return;
    }

    const auto customConfig = config.group("Custom");
    QHash<QString, QString> variables = {{"name", target}, {"path", path}};
    const QString arg = KMacroExpander::expandMacrosShellQuote(customConfig.readEntry("new_params"), variables);
    const auto splitPair = splitArguments(arg);
    if (!splitPair.first) {
        showErrorNotification("The command line arguments from the custom config are invalid!");
        return;
    }
    QStringList args = splitPair.second;

    // Replace everything from tmux onwards with the tmuxinator command
    if (tmuxinator) {
        const int idx = args.indexOf("tmux");
        if (idx == -1) {
            showErrorNotification("The custom create session parameters must contain \"tmux\" to launch tmuxinator!");
            return;
        }
        args = args.mid(0, idx) + tmuxinatorCommand;
    }

    // If new session is started but no name provided
    if (target.isEmpty()) {
        args.removeOne("-s");
        args.removeOne("-t");
    }
    args.removeAll(QString());

    QProcess::startDetached(customConfig.readEntry("program"), args);
}

QStringList TmuxRunnerAPI::fetchTmuxinatorConfigs()
{
    QStringList tmuxinatorConfigs;
    if (QStandardPaths::findExecutable(QStringLiteral("tmuxinator")).isEmpty()) {
        return tmuxinatorConfigs;
    }
    // Output is a "tmuxinator projects:" header followed by the names, possibly spread over several lines/columns
    QProcess process;
    process.start(QStringLiteral("tmuxinator"), QStringList{"ls"});
    process.waitForFinished(2000);
    const QStringList lines = QString::fromUtf8(process.readAll()).split('\n');
    for (int i = 1; i < lines.size(); ++i) {
        tmuxinatorConfigs.append(lines.at(i).split(' ', Qt::SkipEmptyParts));
    }
    return tmuxinatorConfigs;
}

QString TmuxRunnerAPI::parseQueryFlags(QString &term, QString &openIn)
{
    // Flag at end of query or just a flag with no session name
    QString program;
    const QRegularExpressionMatch flagMatch = queryHasFlag.match(term);
    if (flagMatch.hasMatch()) {
        const QString flagValue = flags.value(flagMatch.captured(1));
        if (!flagValue.isEmpty()) {
            program = flagValue;
            openIn = " in " + QString(flagValue).remove("-session");
        } else {
            openIn = " default (invalid flag)";
        }
        term.remove(flagMatch.capturedStart(), flagMatch.capturedLength());
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
    KNotification::event(KNotification::Error, QStringLiteral("Tmux Runner"), msg, QStringLiteral("utilities-terminal"));
}
