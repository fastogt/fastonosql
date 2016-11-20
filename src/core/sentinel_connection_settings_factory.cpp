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

#include "core/sentinel_connection_settings_factory.h"

#include <common/convert2string.h>

#ifdef BUILD_WITH_REDIS
#include "core/db/redis/sentinel_settings.h"  // for SentinelSettings
#endif

namespace fastonosql {
namespace core {

ISentinelSettingsBase* SentinelConnectionSettingsFactory::CreateFromType(
    connectionTypes type,
    const connection_path_t& conName) {
#ifdef BUILD_WITH_REDIS
  if (type == REDIS) {
    return new redis::SentinelSettings(conName);
  }
#endif
  return nullptr;
}

ISentinelSettingsBase* SentinelConnectionSettingsFactory::CreateFromString(const std::string& val) {
  if (val.empty()) {
    return nullptr;
  }

  ISentinelSettingsBase* result = nullptr;
  size_t len = val.size();

  uint8_t commaCount = 0;
  std::string elText;

  for (size_t i = 0; i < len; ++i) {
    char ch = val[i];
    if (ch == ',') {
      if (commaCount == 0) {
        int crT = elText[0] - 48;
        result = CreateFromType(static_cast<connectionTypes>(crT), connection_path_t());
        if (!result) {
          return nullptr;
        }
      } else if (commaCount == 1) {
        connection_path_t path(elText);
        result->SetPath(path);
      } else if (commaCount == 2) {
        uint32_t msTime = common::ConvertFromString<uint32_t>(elText);
        result->SetLoggingMsTimeInterval(msTime);

        std::string serText;
        for (size_t j = i + 2; j < len; ++j) {
          ch = val[j];
          if (ch == magicNumber || j == len - 1) {
            SentinelSettings sent;
            bool res = SentinelSettingsfromString(serText, &sent);
            if (res) {
              result->AddSentinel(sent);
            }
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

}  // namespace core
}  // namespace fastonosql
