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

#include "common/value.h"  // for Value, Value::Type

#include "core/db_key.h"       // for NDbKValue, ttl_t

#include "common/qt/gui/base/table_item.h"   // for TableItem
#include "common/qt/gui/base/table_model.h"  // for TableModel

namespace fastonosql {
namespace gui {

class KeyTableItem : public common::qt::gui::TableItem {
 public:
  enum eColumn { kKey = 0, kType = 1, kTTL = 2, kCountColumns = 3 };

  explicit KeyTableItem(const core::NDbKValue& key);

  QString key() const;
  QString typeText() const;
  core::ttl_t ttl() const;
  common::Value::Type type() const;

  core::NDbKValue dbv() const;
  void setDbv(const core::NDbKValue& val);

 private:
  core::NDbKValue key_;
};

class KeysTableModel : public common::qt::gui::TableModel {
  Q_OBJECT
 public:
  explicit KeysTableModel(QObject* parent = 0);
  virtual ~KeysTableModel();

  virtual QVariant data(const QModelIndex& index, int role) const;
  virtual bool setData(const QModelIndex& index, const QVariant& value, int role);
  virtual Qt::ItemFlags flags(const QModelIndex& index) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

  virtual int columnCount(const QModelIndex& parent) const;
  void clear();

  void changeValue(const core::NDbKValue& value);

 Q_SIGNALS:
  void changedTTL(const core::NDbKValue& value, int ttl);
};

}  // namespace gui
}  // namespace fastonosql
