#ifndef TMUXRUNNER_TMUXRUNNERAPI_H
#define TMUXRUNNER_TMUXRUNNERAPI_H

#include <KConfigGroup>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QString>
#include <QStringList>

class TmuxRunnerAPI {
public:
    TmuxRunnerAPI(const KConfigGroup &config);

    QString filterPath(QString path);

    QStringList fetchTmuxSessions();

    QStringList fetchTmuxinatorConfigs();

    void executeAttatchCommand(QString &program, const QString &target);

    void executeCreateCommand(QString &program, const QString &target, const QMap<QString, QVariant> &data);

    QString parseQueryFlags(QString &term, QString &openIn);

    QPair<bool, QStringList> splitArguments(const QString &argument);

    void showErrorNotification(const QString &msg);

    inline static QString configFileLocation()
    {
        return QStandardPaths::locate(QStandardPaths::ConfigLocation, QStringLiteral("krunnerplugins/tmuxrunnerrc"));
    }

private:
    const QLatin1Char lineSeparator = QLatin1Char(':');
    KConfigGroup config;
    const QString fetchProgram = QStringLiteral("tmux");
    const QStringList fetchArgs = {QStringLiteral("ls")};
    const QMap<QString, QString> flags = {
            {"k", "konsole"},
            {"y", "yakuake-session"},
            {"t", "terminator"},
            {"s", "st"},
            {"c", "custom"},
    };
    QRegularExpression queryHasFlag = QRegularExpression(QStringLiteral(" ?-([a-z])$"));
};


#endif //TMUXRUNNER_TMUXRUNNERAPI_H
