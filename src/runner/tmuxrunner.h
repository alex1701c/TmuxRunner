#ifndef TMUXRUNNER_H
#define TMUXRUNNER_H

#include "core/TmuxRunnerAPI.h"
#include <KRunner/AbstractRunner>
#include <KSharedConfig>
#include <QIcon>

#if QT_VERSION_MAJOR == 6
#include <KRunner/Action>
#endif

class TmuxRunner : public KRunner::AbstractRunner
{
    Q_OBJECT

public:
    TmuxRunner(QObject *parent, const KPluginMetaData &data, const QVariantList &args);

    QList<QString> tmuxSessions;
    QList<QString> tmuxinatorConfigs;

    bool enableTmuxinator, enableFlags;
    // Default program used for launching sessions, can be overwritten using flags
    QString defaultProgram;
    QString actionProgram;
#if KRUNNER_VERSION_MAJOR == 5
    QList<QAction *> actionList;
#else
    KRunner::Actions actionList;
#endif

    // Reusable variables
    const QLatin1String triggerWord{"tmux"};
    const QRegularExpression triggerWordRegex{"tmux *"};
    const QIcon icon = QIcon::fromTheme("utilities-terminal");
    const QLatin1String tmuxinatorQuery{"inator"};

    std::unique_ptr<TmuxRunnerAPI> api;

public:
    void match(KRunner::RunnerContext &context) override;
    void reloadConfiguration() override;
    void run(const KRunner::RunnerContext &context, const KRunner::QueryMatch &match) override;

    KRunner::QueryMatch createMatch(const QString &text, const QMap<QString, QVariant> &data, float relevance);
    QList<KRunner::QueryMatch> addTmuxAttachMatches(QString &term, const QString &openIn, const QString &program, QStringList &attached, bool *exactMatch);
    QList<KRunner::QueryMatch> addTmuxNewSessionMatches(QString &term, const QString &openIn, const QString &program, bool tmuxinator);
    QList<KRunner::QueryMatch> addTmuxinatorMatches(QString &term, const QString &openIn, const QString &program, QStringList &attached);
};

#endif
