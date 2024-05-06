#ifndef TMUXRUNNER_H
#define TMUXRUNNER_H

#include "core/TmuxRunnerAPI.h"
#include <KRunner/AbstractRunner>
#include <KSharedConfig>
#include <QIcon>

class TmuxRunner : public Plasma::AbstractRunner
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
    QList<QAction *> actionList;

    // Reusable variables
    const QLatin1String triggerWord{"tmux"};
    const QRegularExpression triggerWordRegex{"tmux *"};
    const QIcon icon = QIcon::fromTheme("utilities-terminal");
    const QLatin1String tmuxinatorQuery{"inator"};

    std::unique_ptr<TmuxRunnerAPI> api;

public:
    void match(Plasma::RunnerContext &context) override;
    void reloadConfiguration() override;
    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) override;

    Plasma::QueryMatch createMatch(const QString &text, const QMap<QString, QVariant> &data, float relevance);
    QList<Plasma::QueryMatch> addTmuxAttachMatches(QString &term, const QString &openIn, const QString &program, QStringList &attached, bool *exactMatch);
    QList<Plasma::QueryMatch> addTmuxNewSessionMatches(QString &term, const QString &openIn, const QString &program, bool tmuxinator);
    QList<Plasma::QueryMatch> addTmuxinatorMatches(QString &term, const QString &openIn, const QString &program, QStringList &attached);
};

#endif
