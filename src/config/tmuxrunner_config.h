#ifndef TmuxRunnerCONFIG_H
#define TmuxRunnerCONFIG_H

#include "ui_tmuxrunner_config.h"
#include <KCModule>
#include <KConfigGroup>
#include <KMessageWidget>
#include <QTimer>

class TmuxRunnerConfigForm : public QWidget, public Ui::TmuxRunnerConfigUi
{
    Q_OBJECT

public:
    explicit TmuxRunnerConfigForm(QWidget *parent);
};

class TmuxRunnerConfig : public KCModule
{
    Q_OBJECT

public:
    explicit TmuxRunnerConfig(QObject *parent, const QVariantList &);

    KConfigGroup config;
    KConfigGroup shortcutConfig;
    KConfigGroup customTerminalConfig;

public Q_SLOTS:
    void save() override;
    void defaults() override;
    void addShortcut();
    void deleteShortcut();
    void customOptionInsertion();
    void shortcutInsertion();
    bool splitArguments(const QString &arg);
    void validateCustomArguments();

private:
    TmuxRunnerConfigForm *m_ui;
    KMessageWidget *errorMessageWidget = nullptr;
};

#endif
