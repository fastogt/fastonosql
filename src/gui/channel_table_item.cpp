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

#include "gui/channel_table_item.h"

#include <common/qt/convert2string.h>  // for ConvertFromString

namespace fastonosql {
namespace gui {

ChannelTableItem::ChannelTableItem(const core::NDbPSChannel& chan) : channel_(chan) {}

core::NDbPSChannel ChannelTableItem::channel() const {
  return channel_;
}

QString ChannelTableItem::name() const {
  QString qname;
  common::ConvertFromString(channel_.GetName(), &qname);
  return qname;
}

size_t ChannelTableItem::numberOfSubscribers() const {
  return channel_.GetNumberOfSubscribers();
}

}  // namespace gui
}  // namespace fastonosql
