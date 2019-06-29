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

TmuxRunnerConfig::TmuxRunnerConfig(QWidget *parent, const QVariantList &args) : KCModule(parent, args) {
    m_ui = new TmuxRunnerConfigForm(this);
    auto *layout = new QGridLayout(this);
    layout->addWidget(m_ui, 0, 0);

    config = KSharedConfig::openConfig("krunnerrc")->group("Runners").group("TmuxRunner");
    config.writeEntry("program", "konsole");
    auto shortcutConfig = config.group("Shortcuts");
    auto customTerminalConfig = config.group("Custom");

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
    qInfo() << program;
    if (program == "konsole") m_ui->optionKonsole->setChecked(true);
    else if (program == "yakuake") m_ui->optionYakuake->setChecked(true);
    else if (program == "terminator") m_ui->optionTerminator->setChecked(true);
    else if (program == "st") m_ui->optionSucklessTerminal->setChecked(true);
    else if (program == "custom") m_ui->optionCustom->setChecked(true);

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


void TmuxRunnerConfig::save() {

    KCModule::save();

    emit changed();
}

void TmuxRunnerConfig::defaults() {

    emit changed(true);
}

void TmuxRunnerConfig::customOptionInsertion() {
    if (!m_ui->createSessionParameters->text().isEmpty() &&
        !m_ui->attatchSessionProgram->text().isEmpty() && !m_ui->attatchSessionParameters->text().isEmpty()) {
        m_ui->optionCustom->setEnabled(true);
    }
}


#include "tmuxrunner_config.moc"
