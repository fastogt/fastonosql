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

#include "proxy/cluster_connection_settings_factory.h"

#include <common/convert2string.h>

#ifdef BUILD_WITH_REDIS
#include "proxy/db/redis/cluster_settings.h"  // for ClusterSettings
#endif

#ifdef BUILD_WITH_PIKA
#include "proxy/db/pika/cluster_settings.h"  // for ClusterSettings
#endif

#include "proxy/connection_settings_factory.h"

namespace fastonosql {
namespace proxy {

std::string ClusterConnectionSettingsFactory::ConvertSettingsToString(IClusterSettingsBase* settings) {
  std::stringstream str;
  str << ConnectionSettingsFactory::GetInstance().ConvertSettingsToString(settings) << setting_value_delemitr;
  auto nodes = settings->GetNodes();
  for (size_t i = 0; i < nodes.size(); ++i) {
    IConnectionSettingsBaseSPtr serv = nodes[i];
    if (serv) {
      str << magic_number << ConnectionSettingsFactory::GetInstance().ConvertSettingsToString(serv.get());
    }
  }

  return str.str();
}

IClusterSettingsBase* ClusterConnectionSettingsFactory::CreateFromTypeCluster(
    core::ConnectionType type,
    const connection_path_t& connection_path) {
#ifdef BUILD_WITH_REDIS
  if (type == core::REDIS) {
    return new redis::ClusterSettings(connection_path);
  }
#endif
#ifdef BUILD_WITH_PIKA
  if (type == core::PIKA) {
    return new pika::ClusterSettings(connection_path);
  }
#endif

  NOTREACHED() << "Not handled type: " << type;
  return nullptr;
}

IClusterSettingsBase* ClusterConnectionSettingsFactory::CreateFromStringCluster(const std::string& value) {
  if (value.empty()) {
    DNOTREACHED();
    return nullptr;
  }

  IClusterSettingsBase* result = nullptr;
  size_t value_len = value.size();

  uint8_t comma_count = 0;
  std::string element_text;

  for (size_t i = 0; i < value_len; ++i) {
    char ch = value[i];
    if (ch == setting_value_delemitr) {
      if (comma_count == 0) {
        int ascii_connection_type = element_text[0] - 48;  // saved in char but number
        result = CreateFromTypeCluster(static_cast<core::ConnectionType>(ascii_connection_type), connection_path_t());
        if (!result) {
          return nullptr;
        }
      } else if (comma_count == 1) {
        connection_path_t path(element_text);
        result->SetPath(path);
      } else if (comma_count == 2) {
        int ms_time;
        if (common::ConvertFromString(element_text, &ms_time)) {
          result->SetLoggingMsTimeInterval(ms_time);
        }
        std::string server_text;
        for (size_t j = i + 2; j < value_len; ++j) {
          ch = value[j];
          if (ch == magic_number || j == value_len - 1) {
            IConnectionSettingsBase* server =
                ConnectionSettingsFactory::GetInstance().CreateSettingsFromString(server_text);
            if (server) {
              result->AddNode(IConnectionSettingsBaseSPtr(server));
            }
            server_text.clear();
          } else {
            server_text += ch;
          }
        }
        break;
      }
      comma_count++;
      element_text.clear();
    } else {
      element_text += ch;
    }
  }

  DCHECK(result);
  return result;
}

}  // namespace proxy
}  // namespace fastonosql
