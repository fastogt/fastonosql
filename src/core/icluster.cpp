/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    SiteOnYourDevice is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SiteOnYourDevice is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SiteOnYourDevice.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "core/icluster.h"

#include <string>

#include "common/qt/convert_string.h"

#include "core/iserver.h"

namespace fastonosql {

ICluster::ICluster(const std::string &name)
  : name_(name) {
}

QString ICluster::name() const {
  return common::convertFromString<QString>(name_);
}

ICluster::nodes_type ICluster::nodes() const {
  return nodes_;
}

void ICluster::addServer(IServerSPtr serv) {
  if (serv) {
    nodes_.push_back(serv);
  }
}

IServerSPtr ICluster::root() const {
  if (nodes_.empty()) {
    return IServerSPtr();
  }

  return nodes_[0];
}

}  // namespace fastonosql
