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

#include <QString>

#include <common/qt/gui/base/table_item.h>

namespace fastonosql {
namespace gui {

class KeyValueTableItem : public common::qt::gui::TableItem {
 public:
  enum Mode { AddAction = 0, EditAction = 1, RemoveAction };
  enum eColumn { kKey = 0, kValue = 1, kAction = 2, kCountColumns = 3 };

  KeyValueTableItem(const QString& key, const QString& value, Mode state);

  QString GetKey() const;
  void SetKey(const QString& key);

  QString GetValue() const;
  void SetValue(const QString& val);

  Mode GetActionState() const;

 private:
  QString key_;
  QString value_;
  const Mode state_;
};

}  // namespace gui
}  // namespace fastonosql
