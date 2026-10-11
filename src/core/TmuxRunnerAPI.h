#ifndef TMUXRUNNER_TMUXRUNNERAPI_H
#define TMUXRUNNER_TMUXRUNNERAPI_H

#include <KConfigGroup>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QString>
#include <QStringList>

class TmuxRunnerAPI
{
public:
    TmuxRunnerAPI(const KConfigGroup &config);

    QString filterPath(QString path);

    QStringList fetchTmuxSessions();

    QStringList fetchTmuxinatorConfigs();

    void executeAttachCommand(const QString &program, const QString &target);

    void executeCreateCommand(const QString &program, const QString &target, const QMap<QString, QVariant> &data);

    QString parseQueryFlags(QString &term, QString &openIn);

    QPair<bool, QStringList> splitArguments(const QString &argument);

    void showErrorNotification(const QString &msg);

private:
    // Builds the terminal arguments needed to run the given command
    QStringList wrapInTerminal(const QString &program, const QString &title, const QStringList &command);

    KConfigGroup config;
    const QMap<QString, QString> flags = {
        {"k", "konsole"},
        {"y", "yakuake-session"},
        {"t", "terminator"},
        {"s", "st"},
        {"i", "kitty"},
        {"c", "custom"},
    };
    QRegularExpression queryHasFlag = QRegularExpression(QStringLiteral("(?:^| +)-([a-z])$"));
};

#endif // TMUXRUNNER_TMUXRUNNERAPI_H
