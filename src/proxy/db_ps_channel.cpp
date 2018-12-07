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

#include "proxy/db_ps_channel.h"

namespace fastonosql {
namespace proxy {

NDbPSChannel::NDbPSChannel() {}

NDbPSChannel::NDbPSChannel(const name_t& name, size_t nos) : name_(name), number_of_subscribers_(nos) {}

NDbPSChannel::name_t NDbPSChannel::GetName() const {
  return name_;
}

void NDbPSChannel::SetName(const name_t& name) {
  name_ = name;
}

size_t NDbPSChannel::GetNumberOfSubscribers() const {
  return number_of_subscribers_;
}

void NDbPSChannel::SetNumberOfSubscribers(size_t nos) {
  number_of_subscribers_ = nos;
}

}  // namespace proxy
}  // namespace fastonosql
