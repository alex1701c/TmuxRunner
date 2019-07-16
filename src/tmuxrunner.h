#ifndef TMUXRUNNER_H
#define TMUXRUNNER_H

#include <KRunner/AbstractRunner>
#include <KSharedConfig>

class TmuxRunner : public Plasma::AbstractRunner
{
    Q_OBJECT

public:
    TmuxRunner(QObject *parent, const QVariantList &args);
    ~TmuxRunner() override;

    QList<QString> tmuxSessions;
    QList<QString> tmuxinatorConfigs;

    Plasma::QueryMatch createMatch(const QString &text, const QMap<QString, QVariant> &data, float relevance);

    KConfigGroup config;

    QString filterPath(QString path);


protected Q_SLOTS:

    void init() override;

    void prepareForMatchSession();

public: // Plasma::AbstractRunner API
    void match(Plasma::RunnerContext &context) override;
    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) override;
};

#endif
