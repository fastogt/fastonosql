/*  Copyright (C) 2014-2019 FastoGT. All right reserved.

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

#include <common/qt/gui/base/table_model.h>
#include <common/value.h>

namespace fastonosql {
namespace gui {

class ListTableModel : public common::qt::gui::TableModel {
  Q_OBJECT

 public:
  typedef common::Value::string_t row_t;
  enum eColumn : uint8_t { kValue = 0, kAction = 1, kCountColumns = 2 };

  explicit ListTableModel(QObject* parent = Q_NULLPTR);

  QVariant data(const QModelIndex& index, int role) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role) override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

  int columnCount(const QModelIndex& parent) const override;
  void clear();

  common::ArrayValue* arrayValue() const;  // alocate memory
  common::SetValue* setValue() const;      // alocate memory

  void insertRow(const row_t& value);
  void removeRow(int row);

  void setFirstColumnName(const QString& name);

 private:
  using TableModel::insertItem;
  using TableModel::removeItem;

  common::qt::gui::TableItem* createEmptyRow() const;

  QString first_column_name_;
};

}  // namespace gui
}  // namespace fastonosql
