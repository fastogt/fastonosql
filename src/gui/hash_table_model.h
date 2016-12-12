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

#include <QTableView>

#include <common/qt/gui/base/table_model.h>
#include <common/qt/gui/base/table_item.h>
#include <common/value.h>

class QSignalMapper;

namespace fastonosql {
namespace gui {

class KeyValueTableItem : public common::qt::gui::TableItem {
 public:
  enum eColumn { kKey = 0, kValue = 1, kAction = 2, kCountColumns = 3 };

  KeyValueTableItem(const QString& key, const QString& value);

  QString key() const;
  void setKey(const QString& key);

  QString value() const;
  void setValue(const QString& val);

 private:
  QString key_;
  QString value_;
};

class HashTableModel : public common::qt::gui::TableModel {
  Q_OBJECT
 public:
  explicit HashTableModel(QObject* parent = Q_NULLPTR);
  virtual ~HashTableModel();

  virtual QVariant data(const QModelIndex& index, int role) const override;
  virtual bool setData(const QModelIndex& index, const QVariant& value, int role) override;
  virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

  virtual int columnCount(const QModelIndex& parent) const override;
  void clear();

  common::ZSetValue* zsetValue() const;  // alocate memory
  common::HashValue* hashValue() const;  // alocate memory

  void insertRow(const QString& key, const QString& value);

 private:
  using TableModel::insertItem;

  KeyValueTableItem* createEmptyRow() const;
};

}  // namespace gui
}  // namespace fastonosql
