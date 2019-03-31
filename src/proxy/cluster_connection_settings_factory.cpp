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
    along with FastoNoSQL. If not, see <http://www.gnu.org/licenses/>.
*/

#include "proxy/cluster_connection_settings_factory.h"

#include <string>

#include <common/convert2string.h>

#if defined(BUILD_WITH_REDIS)
#include "proxy/db/redis/cluster_settings.h"
#endif
#if defined(BUILD_WITH_KEYDB)
#include "proxy/db/keydb/cluster_settings.h"
#endif

#include "proxy/connection_settings_factory.h"

namespace fastonosql {
namespace proxy {

serialize_t ClusterConnectionSettingsFactory::ConvertSettingsToString(IClusterSettingsBase* settings) {
  common::char_writer<1024> str;
  str << ConnectionSettingsFactory::GetInstance().ConvertSettingsToString(settings) << kSettingValueDelemiter;
  auto nodes = settings->GetNodes();
  for (size_t i = 0; i < nodes.size(); ++i) {
    IConnectionSettingsBaseSPtr serv = nodes[i];
    if (serv) {
      str << kMagicNumber << ConnectionSettingsFactory::GetInstance().ConvertSettingsToString(serv.get());
    }
  }

  return str.str();
}

IClusterSettingsBase* ClusterConnectionSettingsFactory::CreateFromTypeCluster(
    core::ConnectionType type,
    const connection_path_t& connection_path) {
#if defined(BUILD_WITH_REDIS)
  if (type == core::REDIS) {
    return new redis::ClusterSettings(connection_path);
  }
#endif
#if defined(BUILD_WITH_KEYDB)
  if (type == core::KEYDB) {
    return new keydb::ClusterSettings(connection_path);
  }
#endif

  NOTREACHED() << "Not handled type: " << type;
  return nullptr;
}

IClusterSettingsBase* ClusterConnectionSettingsFactory::CreateFromStringCluster(const serialize_t& value) {
  if (value.empty()) {
    DNOTREACHED();
    return nullptr;
  }

  IClusterSettingsBase* result = nullptr;
  const size_t value_len = value.size();

  uint8_t comma_count = 0;
  serialize_t element_text;

  for (size_t i = 0; i < value_len; ++i) {
    serialize_t::value_type ch = value[i];
    if (ch == kSettingValueDelemiter) {
      if (comma_count == 0) {
        uint8_t connection_type;
        if (common::ConvertFromBytes(element_text, &connection_type)) {
          result = CreateFromTypeCluster(static_cast<core::ConnectionType>(connection_type), connection_path_t());
        }
        if (!result) {
          DNOTREACHED() << "Unknown connection_type: " << element_text;
          return nullptr;
        }
      } else if (comma_count == 1) {
        const std::string path_str = common::ConvertToString(element_text);
        connection_path_t path(path_str);
        result->SetPath(path);
      } else if (comma_count == 2) {
        int ms_time;
        if (common::ConvertFromBytes(element_text, &ms_time)) {
          result->SetLoggingMsTimeInterval(ms_time);
        }
        serialize_t server_text;
        for (size_t j = i + 2; j < value_len; ++j) {
          ch = value[j];
          if (ch == kMagicNumber || j == value_len - 1) {
            if (j == value_len - 1) {
              server_text.push_back(ch);
            }

            IConnectionSettingsBase* server =
                ConnectionSettingsFactory::GetInstance().CreateSettingsFromString(server_text);
            if (server) {
              result->AddNode(IConnectionSettingsBaseSPtr(server));
            }
            server_text.clear();
          } else {
            server_text.push_back(ch);
          }
        }
        break;
      }
      comma_count++;
      element_text.clear();
    } else {
      element_text.push_back(ch);
    }
  }

  DCHECK(result);
  return result;
}

}  // namespace proxy
}  // namespace fastonosql
