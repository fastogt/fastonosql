/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

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

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QEvent>

#include "gui/explorer/explorer_tree_view.h"
#include "translations/global.h"

namespace fastonosql {
namespace gui {
ExplorerTreeWidget::ExplorerTreeWidget(QWidget* parent) : QWidget(parent) {
  QVBoxLayout* main_layout = new QVBoxLayout;

  filter_edit_ = new QLineEdit;
  view_ = new ExplorerTreeView(this);
  main_layout->addWidget(filter_edit_);
  main_layout->addWidget(view_);

  VERIFY(
      connect(filter_edit_, &QLineEdit::textChanged, view_, &ExplorerTreeView::textFilterChanged));
  VERIFY(
      connect(view_, &ExplorerTreeView::openedConsole, this, &ExplorerTreeWidget::openedConsole));
  VERIFY(connect(view_, &ExplorerTreeView::closeServer, this, &ExplorerTreeWidget::closeServer,
                 Qt::DirectConnection));
  VERIFY(connect(view_, &ExplorerTreeView::closeCluster, this, &ExplorerTreeWidget::closeCluster,
                 Qt::DirectConnection));
  VERIFY(connect(view_, &ExplorerTreeView::closeSentinel, this, &ExplorerTreeWidget::closeSentinel,
                 Qt::DirectConnection));

  setLayout(main_layout);
}

void ExplorerTreeWidget::addServer(core::IServerSPtr server) {
  view_->addServer(server);
}

void ExplorerTreeWidget::removeServer(core::IServerSPtr server) {
  view_->removeServer(server);
}

void ExplorerTreeWidget::addSentinel(core::ISentinelSPtr sentinel) {
  view_->addSentinel(sentinel);
}

void ExplorerTreeWidget::removeSentinel(core::ISentinelSPtr sentinel) {
  view_->removeSentinel(sentinel);
}

void ExplorerTreeWidget::addCluster(core::IClusterSPtr cluster) {
  view_->addCluster(cluster);
}

void ExplorerTreeWidget::removeCluster(core::IClusterSPtr cluster) {
  view_->removeCluster(cluster);
}

void ExplorerTreeWidget::changeEvent(QEvent* e) {
  if (e->type() == QEvent::LanguageChange) {
    retranslateUi();
  }

  QWidget::changeEvent(e);
}

void ExplorerTreeWidget::retranslateUi() {
  filter_edit_->setPlaceholderText(translations::trFilter);
}

}  // namespace gui
}  // namespace fastonosql
