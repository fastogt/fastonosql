/*  Copyright (C) 2014-2022 FastoGT. All right reserved.

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
    along with FastoNoSQL. If not, see <http://www.gnu.org/licenses/>.
*/

#include "gui/views/fasto_tree_view.h"

#include <QHeaderView>
#include <QMenu>

namespace fastonosql {
namespace gui {

FastoTreeView::FastoTreeView(QWidget* parent) : QTreeView(parent) {
  QHeaderView* head = header();
  head->setDefaultAlignment(Qt::AlignLeft);
  head->setSectionResizeMode(QHeaderView::Stretch);
}

QPoint FastoTreeView::calculateMenuPoint(const QPoint& point) const {
  QPoint menu_point = mapToGlobal(point);
  QHeaderView* head = header();
  menu_point.setY(menu_point.y() + head->height());
  return menu_point;
}

}  // namespace gui
}  // namespace fastonosql
