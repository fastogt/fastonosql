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

#include "gui/explorer/explorer_tree_widget.h"

#include <QEvent>
#include <QLineEdit>
#include <QVBoxLayout>

#include "gui/explorer/explorer_tree_view.h"
#include "gui/gui_factory.h"

#include "translations/global.h"

namespace fastonosql {
namespace gui {

ExplorerTreeWidget::ExplorerTreeWidget(QWidget* parent) : QWidget(parent) {
  QVBoxLayout* main_layout = new QVBoxLayout;

  view_ = new ExplorerTreeView(this);
  filter_edit_ = new QLineEdit;
  filter_edit_->setClearButtonEnabled(true);
  filter_edit_->addAction(GuiFactory::GetInstance().GetSearch16Icon(), QLineEdit::LeadingPosition);
  main_layout->addWidget(view_);
  main_layout->addWidget(filter_edit_);

  VERIFY(connect(filter_edit_, &QLineEdit::textChanged, view_, &ExplorerTreeView::changeTextFilter));
  VERIFY(connect(view_, &ExplorerTreeView::consoleOpened, this, &ExplorerTreeWidget::consoleOpened));
  VERIFY(
      connect(view_, &ExplorerTreeView::consoleOpenedAndExecute, this, &ExplorerTreeWidget::consoleOpenedAndExecute));
  VERIFY(
      connect(view_, &ExplorerTreeView::serverClosed, this, &ExplorerTreeWidget::serverClosed, Qt::DirectConnection));
#if defined(PRO_VERSION)
  VERIFY(
      connect(view_, &ExplorerTreeView::clusterClosed, this, &ExplorerTreeWidget::clusterClosed, Qt::DirectConnection));
  VERIFY(connect(view_, &ExplorerTreeView::sentinelClosed, this, &ExplorerTreeWidget::sentinelClosed,
                 Qt::DirectConnection));
#endif
  setLayout(main_layout);
}

void ExplorerTreeWidget::addServer(proxy::IServerSPtr server) {
  view_->addServer(server);
}

void ExplorerTreeWidget::removeServer(proxy::IServerSPtr server) {
  view_->removeServer(server);
}

#if defined(PRO_VERSION)
void ExplorerTreeWidget::addSentinel(proxy::ISentinelSPtr sentinel) {
  view_->addSentinel(sentinel);
}

void ExplorerTreeWidget::removeSentinel(proxy::ISentinelSPtr sentinel) {
  view_->removeSentinel(sentinel);
}

void ExplorerTreeWidget::addCluster(proxy::IClusterSPtr cluster) {
  view_->addCluster(cluster);
}

void ExplorerTreeWidget::removeCluster(proxy::IClusterSPtr cluster) {
  view_->removeCluster(cluster);
}
#endif

void ExplorerTreeWidget::changeEvent(QEvent* e) {
  if (e->type() == QEvent::LanguageChange) {
    retranslateUi();
  }

  QWidget::changeEvent(e);
}

void ExplorerTreeWidget::retranslateUi() {
  filter_edit_->setPlaceholderText(translations::trSearch + "...");
}

}  // namespace gui
}  // namespace fastonosql
