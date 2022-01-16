/*  Copyright (C) 2014-2022 FastoGT. All right reserved.

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

#include <string>

#include <QString>

#include <common/qt/gui/base/tree_item.h>

#include <fastonosql/core/db_key.h>

namespace fastonosql {
namespace gui {

class FastoCommonItem : public common::qt::gui::TreeItem {
 public:
  FastoCommonItem(const core::NDbKValue& key,
                  const std::string& delimiter,
                  bool read_only,
                  TreeItem* parent,
                  void* internal_pointer);

  QString key() const;
  QString readableValue() const;

  common::Value::Type type() const;
  core::NValue nvalue() const;
  core::NDbKValue dbv() const;

  bool isReadOnly() const;
  void setValue(core::NValue val);

 private:
  core::NDbKValue key_;
  const std::string delimiter_;
  const bool read_only_;
};

core::readable_string_t toRaw(FastoCommonItem* item);

}  // namespace gui
}  // namespace fastonosql
