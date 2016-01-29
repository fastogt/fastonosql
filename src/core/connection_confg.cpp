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

#include "core/connection_confg.h"

#include "common/convert2string.h"

namespace fastonosql {

LocalConfig::LocalConfig(const std::string& dbname)
  : BaseConfig<LOCAL>(), dbname_(dbname) {
}

std::vector<std::string> LocalConfig::args() const {
  std::vector<std::string> argv;

  if(!dbname_.empty()){
      argv.push_back("-f");
      argv.push_back(dbname_);
  }

  if (!mb_delim_.empty()) {
      argv.push_back("-d");
      argv.push_back(mb_delim_);
  }

  return argv;
}

RemoteConfig::RemoteConfig(const std::string &hostip, uint16_t port)
  : BaseConfig<REMOTE>(), hostip_(hostip), hostport_(port) {
}

std::vector<std::string> RemoteConfig::args() const {
  std::vector<std::string> argv;

  if(!hostip_.empty()){
      argv.push_back("-h");
      argv.push_back(hostip_);
  }
  if(hostport_ != 0){
      argv.push_back("-p");
      argv.push_back(common::convertToString(hostport_));
  }

  if (!mb_delim_.empty()) {
      argv.push_back("-d");
      argv.push_back(mb_delim_);
  }

  return argv;
}

}
