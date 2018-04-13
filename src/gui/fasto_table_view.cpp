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

#include "gui/fasto_table_view.h"

#include <QHeaderView>
#include <QMenu>

namespace fastonosql {
namespace gui {

FastoTableView::FastoTableView(QWidget* parent) : QTableView(parent) {
  verticalHeader()->setDefaultAlignment(Qt::AlignLeft);
  horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

  QHeaderView* header = horizontalHeader();
  header->setSectionResizeMode(QHeaderView::Stretch);
}

QPoint FastoTableView::CalculateMenuPoint(const QPoint& point) const {
  QPoint menuPoint = mapToGlobal(point);
  menuPoint.setY(menuPoint.y() + horizontalHeader()->height());
  menuPoint.setX(menuPoint.x() + verticalHeader()->width());
  return menuPoint;
}

}  // namespace gui
}  // namespace fastonosql
