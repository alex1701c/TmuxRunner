/******************************************************************************
 *   Copyright %{CURRENT_YEAR} by %{AUTHOR} <%{EMAIL}>                        *
 *                                                                            *
 *  This library is free software; you can redistribute it and/or modify      *
 *  it under the terms of the GNU Lesser General Public License as published  *
 *  by the Free Software Foundation; either version 2 of the License or (at   *
 *  your option) any later version.                                           *
 *                                                                            *
 *  This library is distributed in the hope that it will be useful,           *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 *  Library General Public License for more details.                          *
 *                                                                            *
 *  You should have received a copy of the GNU Lesser General Public License  *
 *  along with this library; see the file COPYING.LIB.                        *
 *  If not, see <http://www.gnu.org/licenses/>.                               *
 *****************************************************************************/

#include "tmuxrunner_config.h"
#include <KSharedConfig>
#include <KPluginFactory>
#include <krunner/abstractrunner.h>
#include <QtWidgets/QGridLayout>
#include <QDebug>

K_PLUGIN_FACTORY(TmuxRunnerConfigFactory, registerPlugin<TmuxRunnerConfig>("kcm_krunner_tmuxrunner");)

TmuxRunnerConfigForm::TmuxRunnerConfigForm(QWidget *parent) : QWidget(parent) {
    setupUi(this);
}
// TODO Handle defaults
// CHECK Hamdle Save
// TODO Handle enable/disable of shortcut add/delete
// TODO Handle click to add new shortcut
// TODO Handle click to delete new shortcut

TmuxRunnerConfig::TmuxRunnerConfig(QWidget *parent, const QVariantList &args) : KCModule(parent, args) {
    m_ui = new TmuxRunnerConfigForm(this);
    auto *layout = new QGridLayout(this);
    layout->addWidget(m_ui, 0, 0);

    config = KSharedConfig::openConfig("krunnerrc")->group("Runners").group("TmuxRunner");
    shortcutConfig = config.group("Shortcuts");
    customTerminalConfig = config.group("Custom");

    if (shortcutConfig.keyList().isEmpty()) {
        m_ui->shortcutList->hide();
    } else {
        for (auto const &key:shortcutConfig.keyList()) {
            m_ui->shortcutList->addItem(key + " ==> " + shortcutConfig.readEntry(key));
        }
    }
    m_ui->attatchSessionProgram->setText(customTerminalConfig.readEntry("program", ""));
    m_ui->attatchSessionParameters->setText(customTerminalConfig.readEntry("attatch_params", ""));
    m_ui->createSessionParameters->setText(customTerminalConfig.readEntry("new_params", ""));


    const auto program = config.readEntry("program", "konsole");
    if (program == "konsole") m_ui->optionKonsole->setChecked(true);
    else if (program == "yakuake") m_ui->optionYakuake->setChecked(true);
    else if (program == "terminator") m_ui->optionTerminator->setChecked(true);
    else if (program == "st") m_ui->optionSucklessTerminal->setChecked(true);
    else if (program == "custom") m_ui->optionCustom->setChecked(true);

    m_ui->partlyMatchesOption->setChecked(config.readEntry("add_new_by_part_match", "false") == "true");
    m_ui->optionCustom->setEnabled(!m_ui->createSessionParameters->text().isEmpty() &&
                                   !m_ui->attatchSessionProgram->text().isEmpty() &&
                                   !m_ui->attatchSessionParameters->text().isEmpty());
    connect(m_ui->attatchSessionProgram, SIGNAL(textChanged(QString)), this, SLOT(changed()));
    connect(m_ui->attatchSessionParameters, SIGNAL(textChanged(QString)), this, SLOT(changed()));
    connect(m_ui->createSessionParameters, SIGNAL(textChanged(QString)), this, SLOT(changed()));
    connect(m_ui->attatchSessionProgram, SIGNAL(textChanged(QString)), this, SLOT(customOptionInsertion()));
    connect(m_ui->attatchSessionParameters, SIGNAL(textChanged(QString)), this, SLOT(customOptionInsertion()));
    connect(m_ui->createSessionParameters, SIGNAL(textChanged(QString)), this, SLOT(customOptionInsertion()));

    connect(m_ui->partlyMatchesOption, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->optionKonsole, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->optionYakuake, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->optionTerminator, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->optionSucklessTerminal, SIGNAL(clicked(bool)), this, SLOT(changed()));

    connect(m_ui->shortcutAddButton, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->shortcutDeleteButton, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->optionSucklessTerminal, SIGNAL(clicked(bool)), this, SLOT(changed()));

    load();
}


void TmuxRunnerConfig::defaults() {

    emit changed(true);
}
void TmuxRunnerConfig::save() {

    KCModule::save();

    if (m_ui->optionKonsole->isChecked()) config.writeEntry("program", "konsole");
    else if (m_ui->optionYakuake->isChecked()) config.writeEntry("program", "yakuake");
    else if (m_ui->optionTerminator->isChecked()) config.writeEntry("program", "terminator");
    else if (m_ui->optionSucklessTerminal->isChecked()) config.writeEntry("program", "st");
    else if (m_ui->optionCustom->isChecked() && m_ui->optionCustom->isEnabled()) config.writeEntry("program", "custom");
    else config.writeEntry("program", "konsole");

    config.writeEntry("add_new_by_part_match", m_ui->partlyMatchesOption->isChecked() ? "true" : "false");

    customTerminalConfig.writeEntry("program", m_ui->attatchSessionProgram->text());
    customTerminalConfig.writeEntry("attatch_params", m_ui->attatchSessionParameters->text());
    customTerminalConfig.writeEntry("new_params", m_ui->createSessionParameters->text());

    for (int i = 0; i < m_ui->shortcutList->count(); i++) {
        const auto split = m_ui->shortcutList->takeItem(i)->text().split(" ==> ");
        shortcutConfig.writeEntry(split.first(), split.last());
    }
    emit changed();
}

void TmuxRunnerConfig::customOptionInsertion() {
    m_ui->optionCustom->setEnabled(!m_ui->createSessionParameters->text().isEmpty() &&
                                   !m_ui->attatchSessionProgram->text().isEmpty() &&
                                   !m_ui->attatchSessionParameters->text().isEmpty());
}

void TmuxRunnerConfig::shortcutInsertion() {

}


#include "tmuxrunner_config.moc"
