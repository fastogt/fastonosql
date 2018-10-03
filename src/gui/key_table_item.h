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

#include <common/qt/gui/base/table_item.h>  // for TableItem

#include <QString>

#include <fastonosql/core/db_key.h>  // for NDbKValue, ttl_t

namespace fastonosql {
namespace gui {

class KeyTableItem : public common::qt::gui::TableItem {
 public:
  enum eColumn { kKey = 0, kType = 1, kTTL = 2, kCountColumns = 3 };

  explicit KeyTableItem(const core::NDbKValue& dbv);

  QString keyString() const;
  QString typeText() const;
  core::ttl_t TTL() const;
  common::Value::Type type() const;

  core::NDbKValue Dbv() const;
  void setDbv(const core::NDbKValue& val);

  core::NKey key() const;
  void setKey(const core::NKey& key);

 private:
  core::NDbKValue dbv_;
};

}  // namespace gui
}  // namespace fastonosql
