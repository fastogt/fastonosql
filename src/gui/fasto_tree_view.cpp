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

#include "gui/fasto_tree_view.h"

#include <QMenu>
#include <QHeaderView>

#include "common/macros.h"

namespace fastonosql {
namespace gui {

FastoTreeView::FastoTreeView(QWidget* parent)
  : QTreeView(parent) {
  setSelectionMode(QAbstractItemView::ExtendedSelection);
  setSelectionBehavior(QAbstractItemView::SelectRows);

  header()->resizeSections(QHeaderView::Stretch);

  setContextMenuPolicy(Qt::CustomContextMenu);
  VERIFY(connect(this, &FastoTreeView::customContextMenuRequested,
                 this, &FastoTreeView::showContextMenu));
}

void FastoTreeView::showContextMenu(const QPoint& point) {
  QPoint menuPoint = mapToGlobal(point);
  menuPoint.setY(menuPoint.y() + header()->height());
  QMenu menu(this);
  menu.exec(menuPoint);
}

void FastoTreeView::resizeEvent(QResizeEvent* event) {
  header()->resizeSections(QHeaderView::Stretch);
  QTreeView::resizeEvent(event);
}

}  // namespace gui
}  // namespace fastonosql
