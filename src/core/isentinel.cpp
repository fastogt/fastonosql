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

#include "core/isentinel.h"

#include <string>

namespace fastonosql {
namespace core {

ISentinel::ISentinel(const std::string& name)
  : name_(name) {
}

std::string ISentinel::name() const {
  return name_;
}

ISentinel::nodes_type ISentinel::nodes() const {
  return nodes_;
}

void ISentinel::addServer(IServerSPtr serv) {
  if (!serv) {
    DNOTREACHED();
    return;
  }

  nodes_.push_back(serv);
}

IServerSPtr ISentinel::root() const {
  for (size_t i = 0; i < nodes_.size(); ++i) {
    IServerRemote* rserver = dynamic_cast<IServerRemote*>(nodes_[i].get());  // +
    CHECK(rserver);
    if (rserver->role() == MASTER) {
      return nodes_[i];
    }
  }

  return IServerSPtr();
}

}  // namespace core
}  // namespace fastonosql
