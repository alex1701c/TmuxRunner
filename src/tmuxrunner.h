#ifndef TMUXRUNNER_H
#define TMUXRUNNER_H

#include <KRunner/AbstractRunner>
#include <QtCore>
#include <KSharedConfig>

class TmuxRunner : public Plasma::AbstractRunner {
Q_OBJECT

public:
    TmuxRunner(QObject *parent, const QVariantList &args);

    ~TmuxRunner() override;

    QFileSystemWatcher watcher;
    QList<QString> tmuxSessions;
    QList<QString> tmuxinatorConfigs;

    Plasma::QueryMatch createMatch(const QString &text, const QMap<QString, QVariant> &data, float relevance);

    KConfigGroup config;

    bool enableTmuxinator, enableFlags, enableNewSessionByPartlyMatch;
    // Default program used for launching sessions, can be overwritten using flags
    QString defaultProgram;

    // Reusable variables
    const QLatin1String triggerWord = QLatin1String("tmux");
    const QRegularExpression formatQueryRegex = QRegularExpression("tmux *");
    const QIcon icon = QIcon::fromTheme("utilities-terminal");
    const QLatin1Char lineSeparator = QLatin1Char(':');
    const QMap<QString, QString> flags = {
            {"k", "konsole"},
            {"y", "yakuake-session"},
            {"t", "terminator"},
            {"s", "st"},
            {"c", "custom"},
    };


protected Q_SLOTS:

    void init() override;

    void reloadPluginConfiguration(const QString &path = "");

    void prepareForMatchSession();

public:
    void match(Plasma::RunnerContext &context) override;

    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) override;

    QString filterPath(QString path);
};

#endif
