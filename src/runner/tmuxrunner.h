#ifndef TMUXRUNNER_H
#define TMUXRUNNER_H

#include <KRunner/AbstractRunner>
#include <QtCore>
#include <KSharedConfig>
#include "core/TmuxRunnerAPI.h"

class TmuxRunner : public Plasma::AbstractRunner {
Q_OBJECT

public:
    TmuxRunner(QObject *parent, const QVariantList &args);

    ~TmuxRunner() override;

    QFileSystemWatcher watcher;
    QList<QString> tmuxSessions;
    QList<QString> tmuxinatorConfigs;
    KConfigGroup config;

    bool enableTmuxinator, enableFlags, enableNewSessionByPartlyMatch;
    // Default program used for launching sessions, can be overwritten using flags
    QString defaultProgram;
    QString actionProgram;
    QList<QAction *> actionList;

    // Reusable variables
    const QLatin1String triggerWord = QLatin1String("tmux");
    const QRegularExpression triggerWordRegex = QRegularExpression("tmux *");
    const QIcon icon = QIcon::fromTheme("utilities-terminal");
    const QLatin1String tmuxinatorQuery = QLatin1String("inator");
    const QRegularExpression tmuxinatorQueryRegex = QRegularExpression(R"(inator(?: (\w+) *(.+)?)?)");
    const QRegularExpression tmuxinatorClearRegex = QRegularExpression("^inator *");

    TmuxRunnerAPI *api;

protected Q_SLOTS:
    void init() override;
    void reloadPluginConfiguration(const QString &path = "");
    void prepareForMatchSession();

public:
    void match(Plasma::RunnerContext &context) override;
    QList<QAction*> actionsForMatch(const Plasma::QueryMatch &match) override;
    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) override;

    Plasma::QueryMatch createMatch(const QString &text, const QMap<QString, QVariant> &data, float relevance);
    QList<Plasma::QueryMatch> addTmuxAttachMatches(QString &term, const QString &openIn, const QString &program,
                                                   QStringList &attached, bool *exactMatch);
    QList<Plasma::QueryMatch> addTmuxNewSessionMatches(QString &term, const QString &openIn, const QString &program,
                                                       bool tmuxinator);
    QList<Plasma::QueryMatch> addTmuxinatorMatches(QString &term, const QString &openIn, const QString &program,
                                                   QStringList &attached);

};

#endif
