#ifndef TMUXRUNNER_TMUXRUNNERAPI_H
#define TMUXRUNNER_TMUXRUNNERAPI_H

#include <QtCore>

class TmuxRunnerAPI {
public:
    TmuxRunnerAPI(const KConfigGroup &config);

    QString filterPath(QString path);

    QStringList fetchTmuxSessions();

    void executeAttatchCommand(QString &program, const QString &target);

    void executeCreateCommand(QString &program, const QString &target, const QMap<QString, QVariant> &data);

private:
    const QLatin1Char lineSeparator = QLatin1Char(':');
    KConfigGroup config;
    const QString fetchProgram = QStringLiteral("tmux");
    const QStringList fetchArgs = {QStringLiteral("ls")};
};


#endif //TMUXRUNNER_TMUXRUNNERAPI_H
