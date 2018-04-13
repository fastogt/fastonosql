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

#pragma once

#include <QLabel>
#include <QTableView>

#include "core/value.h"

namespace fastonosql {
namespace gui {

class HashTableModel;

class StreamTypeWidget : public QTableView {
  Q_OBJECT
 public:
  explicit StreamTypeWidget(QWidget* parent = Q_NULLPTR);

  core::StreamValue* GetStreamValue() const;  // alocate memory

  void insertStream(const core::StreamValue::Stream& stream);
  void clear();

 private Q_SLOTS:
  void editRow(const QModelIndex& index);
  void addRow(const QModelIndex& index);
  void removeRow(const QModelIndex& index);

 private:
  void updateStream(const QModelIndex& index, const core::StreamValue::Stream& stream);
  HashTableModel* model_;
  std::vector<core::StreamValue::Stream> streams_;
};

}  // namespace gui
}  // namespace fastonosql
