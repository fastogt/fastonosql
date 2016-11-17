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

#include "gui/property_table_item.h"

#include <common/convert2string.h>

namespace fastonosql {
namespace gui {

PropertyTableItem::PropertyTableItem(const core::property_t& prop) : prop_(prop) {}

QString PropertyTableItem::key() const {
  return common::ConvertFromString<QString>(prop_.first);
}

QString PropertyTableItem::value() const {
  return common::ConvertFromString<QString>(prop_.second);
}

core::property_t PropertyTableItem::property() const {
  return prop_;
}

void PropertyTableItem::setProperty(const core::property_t& prop) {
  prop_ = prop;
}

}  // namespace gui
}  // namespace fastonosql
