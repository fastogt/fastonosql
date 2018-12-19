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

#include <QTableView>

#include <common/value.h>

namespace fastonosql {
namespace gui {

class ListTableModel;

class ListTypeView : public QTableView {
  Q_OBJECT

 public:
  typedef QTableView base_class;
  typedef common::Value::string_t row_t;
  enum Mode : uint8_t { kArray = 0, kSet };

  explicit ListTypeView(QWidget* parent = Q_NULLPTR);

  common::ArrayValue* arrayValue() const;  // alocate memory
  common::SetValue* setValue() const;      // alocate memory

  void insertRow(const row_t& value);
  void clear();

  Mode currentMode() const;
  void setCurrentMode(Mode mode);

 Q_SIGNALS:
  void dataChangedSignal();
  void rowChanged(const row_t& value);

 private Q_SLOTS:
  void addRow(const QModelIndex& index);
  void removeRow(const QModelIndex& index);

 protected:
  void currentChanged(const QModelIndex& current, const QModelIndex& previous) override;

 private:
  ListTableModel* model_;
  Mode mode_;
};

}  // namespace gui
}  // namespace fastonosql
