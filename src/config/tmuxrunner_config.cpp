#include "tmuxrunner_config.h"
#include <KSharedConfig>
#include <KPluginFactory>
#include <krunner/abstractrunner.h>
#include <QtWidgets/QGridLayout>
#include <QDebug>
#include <QtCore/QStringListModel>
#include <QtCore/QDir>

K_PLUGIN_FACTORY(TmuxRunnerConfigFactory, registerPlugin<TmuxRunnerConfig>("kcm_krunner_tmuxrunner");)

TmuxRunnerConfigForm::TmuxRunnerConfigForm(QWidget *parent) : QWidget(parent) {
    setupUi(this);
}

TmuxRunnerConfig::TmuxRunnerConfig(QWidget *parent, const QVariantList &args) : KCModule(parent, args) {
    m_ui = new TmuxRunnerConfigForm(this);
    auto *layout = new QGridLayout(this);
    layout->addWidget(m_ui, 0, 0);

    config = KSharedConfig::openConfig(QDir::homePath() + "/.config/krunnerplugins/tmuxrunnerrc")->group("Config");
    shortcutConfig = config.group("Shortcuts");
    customTerminalConfig = config.group("Custom");

    if (shortcutConfig.keyList().isEmpty()) {
        m_ui->shortcutList->hide();
    } else {
        for (auto const &key:shortcutConfig.keyList()) {
            m_ui->shortcutList->addItem(key + " ==> " + shortcutConfig.readEntry(key));
        }
    }
    m_ui->flags->setChecked(config.readEntry("enable_flags", "true") == "true");
    m_ui->tmuxinatorEnable->setChecked(config.readEntry("enable_tmuxinator", "true") == "true");
    m_ui->attachSessionProgram->setText(customTerminalConfig.readEntry("program", ""));
    m_ui->attachSessionParameters->setText(customTerminalConfig.readEntry("attach_params", ""));
    m_ui->createSessionParameters->setText(customTerminalConfig.readEntry("new_params", ""));


    const auto program = config.readEntry("program", "konsole");
    if (program == "konsole") m_ui->optionKonsole->setChecked(true);
    else if (program == "yakuake-session") m_ui->optionYakuake->setChecked(true);
    else if (program == "terminator") m_ui->optionTerminator->setChecked(true);
    else if (program == "st") m_ui->optionSimpleTerminal->setChecked(true);
    else if (program == "custom") m_ui->optionCustom->setChecked(true);

    m_ui->partlyMatchesOption->setChecked(config.readEntry("add_new_by_part_match", "false") == "true");
    m_ui->optionCustom->setEnabled(
            !m_ui->createSessionParameters->text().isEmpty() &&
            !m_ui->attachSessionProgram->text().isEmpty() &&
            !m_ui->attachSessionParameters->text().isEmpty()
    );

    m_ui->shortcutAddButton->setEnabled(false);
    m_ui->shortcutDeleteButton->setEnabled(false);

    connect(m_ui->partlyMatchesOption, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->flags, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->tmuxinatorEnable, SIGNAL(clicked(bool)), this, SLOT(changed()));
    // Terminals
    connect(m_ui->optionKonsole, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->optionYakuake, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->optionTerminator, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->optionSimpleTerminal, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->optionCustom, SIGNAL(clicked(bool)), this, SLOT(changed()));
    // Enable/Disable custom option
    connect(m_ui->attachSessionProgram, SIGNAL(textChanged(QString)), this, SLOT(customOptionInsertion()));
    connect(m_ui->attachSessionParameters, SIGNAL(textChanged(QString)), this, SLOT(customOptionInsertion()));
    connect(m_ui->createSessionParameters, SIGNAL(textChanged(QString)), this, SLOT(customOptionInsertion()));
    // Custom Terminal
    connect(m_ui->attachSessionProgram, SIGNAL(textChanged(QString)), this, SLOT(changed()));
    connect(m_ui->attachSessionParameters, SIGNAL(textChanged(QString)), this, SLOT(changed()));
    connect(m_ui->createSessionParameters, SIGNAL(textChanged(QString)), this, SLOT(changed()));
    // Shortcuts
    connect(m_ui->shortcutAddButton, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->shortcutDeleteButton, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->shortcutAddButton, SIGNAL(clicked(bool)), this, SLOT(addShortcut()));
    connect(m_ui->shortcutDeleteButton, SIGNAL(clicked(bool)), this, SLOT(deleteShortcut()));
    connect(m_ui->shortcutKey, SIGNAL(textChanged(QString)), this, SLOT(shortcutInsertion()));
    connect(m_ui->shortcutPath, SIGNAL(textChanged(QString)), this, SLOT(shortcutInsertion()));
    connect(m_ui->shortcutList, SIGNAL(currentTextChanged(QString)), this, SLOT(shortcutInsertion()));


    load();
}


void TmuxRunnerConfig::defaults() {

    m_ui->flags->setChecked(true);
    m_ui->partlyMatchesOption->setChecked(false);
    m_ui->optionKonsole->setChecked(true);
    m_ui->tmuxinatorEnable->setChecked(true);

    emit changed(true);
}

void TmuxRunnerConfig::save() {

    if (m_ui->optionKonsole->isChecked()) config.writeEntry("program", "konsole");
    else if (m_ui->optionYakuake->isChecked()) config.writeEntry("program", "yakuake-session");
    else if (m_ui->optionTerminator->isChecked()) config.writeEntry("program", "terminator");
    else if (m_ui->optionSimpleTerminal->isChecked()) config.writeEntry("program", "st");
    else if (m_ui->optionCustom->isChecked() && m_ui->optionCustom->isEnabled()) config.writeEntry("program", "custom");
    else config.writeEntry("program", "konsole");

    config.writeEntry("add_new_by_part_match", m_ui->partlyMatchesOption->isChecked() ? "true" : "false");
    config.writeEntry("enable_flags", m_ui->flags->isChecked() ? "true" : "false");
    config.writeEntry("enable_tmuxinator", m_ui->tmuxinatorEnable->isChecked() ? "true" : "false");

    customTerminalConfig.writeEntry("program", m_ui->attachSessionProgram->text());
    customTerminalConfig.writeEntry("attach_params", m_ui->attachSessionParameters->text());
    customTerminalConfig.writeEntry("new_params", m_ui->createSessionParameters->text());

    for (const auto &key:shortcutConfig.keyList()) {
        shortcutConfig.deleteEntry(key);
    }
    for (int i = 0; i < m_ui->shortcutList->count(); i++) {
        const auto split = m_ui->shortcutList->item(i)->text().split(" ==> ");
        shortcutConfig.writeEntry(split.first(), split.last());
    }
    emit changed();
}

void TmuxRunnerConfig::customOptionInsertion() {
    m_ui->optionCustom->setEnabled(!m_ui->createSessionParameters->text().isEmpty() &&
                                   !m_ui->attachSessionProgram->text().isEmpty() &&
                                   !m_ui->attachSessionParameters->text().isEmpty());
}

void TmuxRunnerConfig::shortcutInsertion() {
    m_ui->shortcutAddButton->setEnabled(!m_ui->shortcutKey->text().isEmpty()
                                        && m_ui->shortcutKey->text().startsWith("$") &&
                                        !m_ui->shortcutPath->text().isEmpty());
    m_ui->shortcutDeleteButton->setEnabled(m_ui->shortcutList->currentIndex().row() != -1);
}

void TmuxRunnerConfig::addShortcut() {
    m_ui->shortcutList->setHidden(false);
    m_ui->shortcutList->addItem(m_ui->shortcutKey->text() + " ==> " + m_ui->shortcutPath->text());
    m_ui->shortcutKey->clear();
    m_ui->shortcutPath->clear();
}

void TmuxRunnerConfig::deleteShortcut() {
    m_ui->shortcutList->model()->removeRow(m_ui->shortcutList->currentRow());
}


#include "tmuxrunner_config.moc"
