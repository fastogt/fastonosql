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

#pragma once

#include <QTableWidget>

#include <common/value.h>

class QSignalMapper;

namespace fastonosql {
namespace gui {

class HashTypeWidget : public QTableWidget {
  Q_OBJECT
 public:
  HashTypeWidget(QWidget* parent = Q_NULLPTR);

  void insertRow(QTableWidgetItem* key, QTableWidgetItem* value);

  common::ZSetValue* zsetValue() const;  // alocate memory
  common::HashValue* hashValue() const;  // alocate memory

 private Q_SLOTS:
  void removeRowInner();
};

}  // namespace gui
}  // namespace fastonosql
