/******************************************************************************
 *  Copyright %{CURRENT_YEAR} by %{AUTHOR} <%{EMAIL}>                         *
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

#ifndef TmuxRunnerCONFIG_H
#define TmuxRunnerCONFIG_H

#include "ui_tmuxrunner_config.h"
#include <KCModule>
#include <KConfigCore/KConfigGroup>

class TmuxRunnerConfigForm : public QWidget, public Ui::TmuxRunnerConfigUi {
Q_OBJECT

public:
    explicit TmuxRunnerConfigForm(QWidget *parent);
};

class TmuxRunnerConfig : public KCModule {
Q_OBJECT

public:
    explicit TmuxRunnerConfig(QWidget *parent = nullptr, const QVariantList &args = QVariantList());

    KConfigGroup config;

public Q_SLOTS:

    void save() override;

    void defaults() override;

    void customOptionInsertion();

private:
    TmuxRunnerConfigForm *m_ui;

};

#endif
