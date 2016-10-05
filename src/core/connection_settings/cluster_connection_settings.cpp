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

#include "core/connection_settings/cluster_connection_settings.h"

#include <sstream>

#include <common/convert2string.h>

#ifdef BUILD_WITH_REDIS
#include "core/redis/cluster_settings.h"  // for ClusterSettings
#endif

namespace fastonosql {
namespace core {

IClusterSettingsBase::IClusterSettingsBase(const connection_path_t& connectionPath,
                                           connectionTypes type)
    : IConnectionSettings(connectionPath, type) {}

IClusterSettingsBase::cluster_nodes_t IClusterSettingsBase::nodes() const {
  return clusters_nodes_;
}

void IClusterSettingsBase::addNode(IConnectionSettingsBaseSPtr node) {
  if (!node) {
    NOTREACHED();
    return;
  }

  clusters_nodes_.push_back(node);
}

IClusterSettingsBase* IClusterSettingsBase::createFromType(connectionTypes type,
                                                           const connection_path_t& conName) {
#ifdef BUILD_WITH_REDIS
  if (type == REDIS) {
    return new redis::ClusterSettings(conName);
  }
#endif
  return nullptr;
}

IClusterSettingsBase* IClusterSettingsBase::fromString(const std::string& val) {
  if (val.empty()) {
    return nullptr;
  }

  IClusterSettingsBase* result = nullptr;
  size_t len = val.size();

  uint8_t commaCount = 0;
  std::string elText;

  for (size_t i = 0; i < len; ++i) {
    char ch = val[i];
    if (ch == ',') {
      if (commaCount == 0) {
        int crT = elText[0] - 48;
        result = createFromType(static_cast<connectionTypes>(crT), connection_path_t());
        if (!result) {
          return nullptr;
        }
      } else if (commaCount == 1) {
        connection_path_t path(elText);
        result->setPath(path);
      } else if (commaCount == 2) {
        uint32_t msTime = common::ConvertFromString<uint32_t>(elText);
        result->setLoggingMsTimeInterval(msTime);
        std::string serText;
        for (size_t j = i + 2; j < len; ++j) {
          ch = val[j];
          if (ch == magicNumber || j == len - 1) {
            IConnectionSettingsBaseSPtr ser(IConnectionSettingsBase::fromString(serText));
            result->addNode(ser);
            serText.clear();
          } else {
            serText += ch;
          }
        }
        break;
      }
      commaCount++;
      elText.clear();
    } else {
      elText += ch;
    }
  }

  return result;
}

std::string IClusterSettingsBase::toString() const {
  std::stringstream str;
  str << IConnectionSettings::toString() << ',';
  for (size_t i = 0; i < clusters_nodes_.size(); ++i) {
    IConnectionSettingsBaseSPtr serv = clusters_nodes_[i];
    if (serv) {
      str << magicNumber << serv->toString();
    }
  }

  std::string res = str.str();
  return res;
}

IConnectionSettingsBaseSPtr IClusterSettingsBase::findSettingsByHost(
    const common::net::HostAndPort& host) const {
  for (size_t i = 0; i < clusters_nodes_.size(); ++i) {
    IConnectionSettingsBaseSPtr cur = clusters_nodes_[i];
    IConnectionSettingsRemote* remote = dynamic_cast<IConnectionSettingsRemote*>(cur.get());  // +
    CHECK(remote);
    if (remote->host() == host) {
      return cur;
    }
  }

  return IConnectionSettingsBaseSPtr();
}

}  // namespace core
}  // namespace fastonosql
