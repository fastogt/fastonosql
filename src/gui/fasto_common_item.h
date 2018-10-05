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

#include <common/qt/gui/base/tree_item.h>  // for TreeItem

#include <fastonosql/core/db_key.h>  // for NDbKValue, NValue

namespace fastonosql {
namespace gui {

class FastoCommonItem : public common::qt::gui::TreeItem {
 public:
  enum eColumn { eKey = 0, eValue = 1, eType = 2, eCountColumns = 3 };
  FastoCommonItem(const core::NDbKValue& key,
                  const std::string& delimiter,
                  bool read_only,
                  TreeItem* parent,
                  void* internalPointer);

  QString key() const;
  QString value() const;
  std::string basicStringValue() const;

  common::Value::Type type() const;
  core::NValue nvalue() const;
  core::NDbKValue dbv() const;
  const char* delimiter() const;

  bool isReadOnly() const;
  void setValue(core::NValue val);

 private:
  core::NDbKValue key_;
  const std::string delimiter_;
  const bool read_only_;
};

QString toJson(FastoCommonItem* item);
QString toRaw(FastoCommonItem* item);
QString toCsv(FastoCommonItem* item);

QString fromGzip(FastoCommonItem* item);
QString fromLZ4(FastoCommonItem* item);
QString fromBZip2(FastoCommonItem* item);
QString fromSnappy(FastoCommonItem* item);
QString fromHexMsgPack(FastoCommonItem* item);

}  // namespace gui
}  // namespace fastonosql
