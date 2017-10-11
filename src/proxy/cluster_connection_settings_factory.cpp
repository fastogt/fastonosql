/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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

#include "proxy/connection_settings_factory.h"

namespace fastonosql {
namespace proxy {

IClusterSettingsBase* ClusterConnectionSettingsFactory::CreateFromType(core::connectionTypes type,
                                                                       const connection_path_t& conName) {
#ifdef BUILD_WITH_REDIS
  if (type == core::REDIS) {
    return new redis::ClusterSettings(conName);
  }
#endif
  return nullptr;
}

IClusterSettingsBase* ClusterConnectionSettingsFactory::CreateFromString(const std::string& val) {
  if (val.empty()) {
    return nullptr;
  }

  IClusterSettingsBase* result = nullptr;
  size_t len = val.size();

  uint8_t commaCount = 0;
  std::string elText;

  for (size_t i = 0; i < len; ++i) {
    char ch = val[i];
    if (ch == setting_value_delemitr) {
      if (commaCount == 0) {
        int crT = elText[0] - 48;
        result = CreateFromType(static_cast<core::connectionTypes>(crT), connection_path_t());
        if (!result) {
          return nullptr;
        }
      } else if (commaCount == 1) {
        connection_path_t path(elText);
        result->SetPath(path);
      } else if (commaCount == 2) {
        int msTime;
        if (common::ConvertFromString(elText, &msTime)) {
          result->SetLoggingMsTimeInterval(msTime);
        }
        std::string serText;
        for (size_t j = i + 2; j < len; ++j) {
          ch = val[j];
          if (ch == magic_number || j == len - 1) {
            IConnectionSettingsBaseSPtr ser(ConnectionSettingsFactory::GetInstance().CreateFromString(serText));
            result->AddNode(ser);
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

}  // namespace proxy
}  // namespace fastonosql
