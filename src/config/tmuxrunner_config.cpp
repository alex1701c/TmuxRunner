#include "tmuxrunner_config.h"
#include <KSharedConfig>
#include <KPluginFactory>
#include <krunner/abstractrunner.h>
#include <QtWidgets/QGridLayout>
#include <QDebug>
#include <QtCore/QStringListModel>
#include <QtCore/QDir>

#include "kcmutils_version.h"

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
    m_ui->flags->setChecked(config.readEntry("enable_flags", true));
    m_ui->tmuxinatorEnable->setChecked(config.readEntry("enable_tmuxinator", true));
    m_ui->attachSessionProgram->setText(customTerminalConfig.readEntry("program"));
    m_ui->attachSessionParameters->setText(customTerminalConfig.readEntry("attach_params"));
    m_ui->createSessionParameters->setText(customTerminalConfig.readEntry("new_params"));


    const auto program = config.readEntry("program", "konsole");
    if (program == "konsole") m_ui->optionKonsole->setChecked(true);
    else if (program == "yakuake-session") m_ui->optionYakuake->setChecked(true);
    else if (program == "terminator") m_ui->optionTerminator->setChecked(true);
    else if (program == "st") m_ui->optionSimpleTerminal->setChecked(true);
    else if (program == "custom") m_ui->optionCustom->setChecked(true);

    m_ui->partlyMatchesOption->setChecked(config.readEntry("add_new_by_part_match", false));
    m_ui->optionCustom->setEnabled(
            !m_ui->createSessionParameters->text().isEmpty() &&
            !m_ui->attachSessionProgram->text().isEmpty() &&
            !m_ui->attachSessionParameters->text().isEmpty()
    );

    m_ui->shortcutAddButton->setEnabled(false);
    m_ui->shortcutDeleteButton->setEnabled(false);

#if KCMUTILS_VERSION >= QT_VERSION_CHECK(5, 64, 0)
    const auto changedSlotPointer = &TmuxRunnerConfig::markAsChanged;
#else
    const auto changedSlotPointer = static_cast<void (TmuxRunnerConfig::*)()>(&TmuxRunnerConfig::changed);
#endif

    connect(m_ui->partlyMatchesOption, &QCheckBox::clicked, this, changedSlotPointer);
    connect(m_ui->flags, &QCheckBox::clicked, this, changedSlotPointer);
    connect(m_ui->tmuxinatorEnable, &QCheckBox::clicked, this, changedSlotPointer);
    // Terminals
    connect(m_ui->optionKonsole, &QCheckBox::clicked, this, changedSlotPointer);
    connect(m_ui->optionYakuake, &QCheckBox::clicked, this, changedSlotPointer);
    connect(m_ui->optionTerminator, &QCheckBox::clicked, this, changedSlotPointer);
    connect(m_ui->optionSimpleTerminal, &QCheckBox::clicked, this, changedSlotPointer);
    connect(m_ui->optionCustom, &QCheckBox::clicked, this, changedSlotPointer);
    // Enable/Disable custom option
    connect(m_ui->attachSessionProgram, &QLineEdit::textChanged, this, &TmuxRunnerConfig::customOptionInsertion);
    connect(m_ui->attachSessionParameters, &QLineEdit::textChanged, this, &TmuxRunnerConfig::customOptionInsertion);
    connect(m_ui->createSessionParameters, &QLineEdit::textChanged, this, &TmuxRunnerConfig::customOptionInsertion);
    // Custom Terminal
    connect(m_ui->attachSessionProgram, &QLineEdit::textChanged, this, changedSlotPointer);
    connect(m_ui->attachSessionParameters, &QLineEdit::textChanged, this, changedSlotPointer);
    connect(m_ui->createSessionParameters, &QLineEdit::textChanged, this, changedSlotPointer);
    // Shortcuts
    connect(m_ui->shortcutAddButton, &QPushButton::clicked, this, changedSlotPointer);
    connect(m_ui->shortcutDeleteButton, &QPushButton::clicked, this, changedSlotPointer);
    connect(m_ui->shortcutAddButton, &QPushButton::clicked, this, &TmuxRunnerConfig::addShortcut);
    connect(m_ui->shortcutDeleteButton, &QPushButton::clicked, this, &TmuxRunnerConfig::deleteShortcut);
    connect(m_ui->shortcutKey, &QLineEdit::textChanged, this, &TmuxRunnerConfig::shortcutInsertion);
    connect(m_ui->shortcutPath, &QLineEdit::textChanged, this, &TmuxRunnerConfig::shortcutInsertion);
    connect(m_ui->shortcutList, &QListWidget::currentTextChanged, this, &TmuxRunnerConfig::shortcutInsertion);
}


void TmuxRunnerConfig::defaults() {

    m_ui->flags->setChecked(true);
    m_ui->partlyMatchesOption->setChecked(false);
    m_ui->optionKonsole->setChecked(true);
    m_ui->tmuxinatorEnable->setChecked(true);

#if KCMUTILS_VERSION >= QT_VERSION_CHECK(5, 64, 0)
    emit markAsChanged();
#else
    emit changed(true);
#endif
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
    for (int i = 0, count = m_ui->shortcutList->count(); i < count; ++i) {
        const auto split = m_ui->shortcutList->item(i)->text().split(" ==> ");
        shortcutConfig.writeEntry(split.first(), split.last());
    }
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
