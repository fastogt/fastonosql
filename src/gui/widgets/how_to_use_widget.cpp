/*  Copyright (C) 2014-2018 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    FastoNoSQL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FastoNoSQL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FastoNoSQL.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "gui/widgets/how_to_use_widget.h"

#include <QLabel>
#include <QMovie>

#include "gui/gui_factory.h"

#include "translations/global.h"

namespace fastonosql {
namespace gui {

HowToUseWidget::HowToUseWidget(const QSize& scale_size, QWidget* parent) : QTabWidget(parent) {
  QLabel* connect_tab = new QLabel;
  QMovie* connect_movie = new QMovie(GuiFactory::GetInstance().GetPathToConnectGif());
  connect_tab->setMovie(connect_movie);
  connect_movie->setScaledSize(scale_size);
  connect_movie->start();
  addTab(connect_tab, translations::trConnect);

  QLabel* workflow_tab = new QLabel;
  QMovie* workflow_movie = new QMovie(GuiFactory::GetInstance().GetPathToWorkflowGif());
  workflow_tab->setMovie(workflow_movie);
  workflow_movie->setScaledSize(scale_size);
  workflow_movie->start();
  addTab(workflow_tab, translations::trWorkflow);

  QLabel* individual_tab = new QLabel;
  QMovie* individual_movie = new QMovie(GuiFactory::GetInstance().GetPathToIndividualBuilds());
  individual_tab->setMovie(individual_movie);
  individual_movie->setScaledSize(scale_size);
  individual_movie->start();
  addTab(individual_tab, translations::trIndividualBuilds);
}

}  // namespace gui
}  // namespace fastonosql
