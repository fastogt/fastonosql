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

#include <common/qt/gui/base/table_model.h>
#include <common/value.h>

namespace fastonosql {
namespace gui {

class HashTableModel : public common::qt::gui::TableModel {
  Q_OBJECT

 public:
  explicit HashTableModel(QObject* parent = Q_NULLPTR);
  ~HashTableModel() override;

  QVariant data(const QModelIndex& index, int role) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role) override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

  int columnCount(const QModelIndex& parent) const override;
  void clear();

  common::ZSetValue* zsetValue() const;  // alocate memory
  common::HashValue* hashValue() const;  // alocate memory

  void insertRow(const QString& key, const QString& value);
  void removeRow(int row);

  void setFirstColumnName(const QString& name);
  void setSecondColumnName(const QString& name);

 private:
  using TableModel::insertItem;
  using TableModel::removeItem;

  common::qt::gui::TableItem* createEmptyRow() const;

  QString first_column_name_;
  QString second_column_name_;
};

}  // namespace gui
}  // namespace fastonosql
