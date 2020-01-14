/*  Copyright (C) 2014-2020 FastoGT. All right reserved.

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

#pragma once

#include <vector>

#include <QTableView>

#include <fastonosql/core/value.h>

namespace fastonosql {
namespace gui {

class StreamTypeWidget : public QTableView {
  Q_OBJECT

 public:
  explicit StreamTypeWidget(QWidget* parent = Q_NULLPTR);

  core::StreamValue* streamValue() const;  // alocate memory

  void insertStream(const core::StreamValue::Stream& stream);
  void clear();

 Q_SIGNALS:
  void dataChangedSignal();

 private Q_SLOTS:
  void editRow(const QModelIndex& index);
  void addRow(const QModelIndex& index);
  void removeRow(const QModelIndex& index);

 private:
  void updateStream(const QModelIndex& index, const core::StreamValue::Stream& stream);
  class StreamTableModel;
  StreamTableModel* model_;
  std::vector<core::StreamValue::Stream> streams_;
};

}  // namespace gui
}  // namespace fastonosql
