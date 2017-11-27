/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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

#include <QTableView>
#include <QLabel>

#include <common/value.h>

namespace fastonosql {
namespace core {
  class StreamValue;
}
namespace gui {

class StreamTableModel;

class StreamTypeWidget : public QWidget {
  Q_OBJECT
 public:
  explicit StreamTypeWidget(QWidget* parent = Q_NULLPTR);
  virtual ~StreamTypeWidget();

  void insertEntry(const QString& first, const QString& second);
  void clear();

  core::StreamValue* GetStreamValue() const;  // alocate memory

 private Q_SLOTS:
  void insertRow(const QModelIndex& index);
  void removeRow(const QModelIndex& index);

 private:
  QLabel* entry_label_;
  QLineEdit* id_edit_;
  QTableView* table_;
  StreamTableModel* model_;
};

}  // namespace gui
}  // namespace fastonosql
