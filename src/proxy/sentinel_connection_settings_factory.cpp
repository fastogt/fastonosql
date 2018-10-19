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

#include "proxy/sentinel_connection_settings_factory.h"

#include <common/convert2string.h>

#ifdef BUILD_WITH_REDIS
#include "proxy/db/redis/sentinel_settings.h"  // for SentinelSettings
#endif

#ifdef BUILD_WITH_PIKA
#include "proxy/db/pika/sentinel_settings.h"  // for SentinelSettings
#endif

namespace fastonosql {
namespace proxy {

ISentinelSettingsBase* SentinelConnectionSettingsFactory::CreateFromTypeSentinel(
    core::ConnectionType type,
    const connection_path_t& connection_path) {
#ifdef BUILD_WITH_REDIS
  if (type == core::REDIS) {
    return new redis::SentinelSettings(connection_path);
  }
#endif
#ifdef BUILD_WITH_PIKA
  if (type == core::PIKA) {
    return new pika::SentinelSettings(connection_path);
  }
#endif

  NOTREACHED() << "Not handled type: " << type;
  return nullptr;
}

ISentinelSettingsBase* SentinelConnectionSettingsFactory::CreateFromStringSentinel(const std::string& value) {
  if (value.empty()) {
    DNOTREACHED();
    return nullptr;
  }

  ISentinelSettingsBase* result = nullptr;
  const size_t value_len = value.size();

  uint8_t comma_count = 0;
  std::string element_text;

  for (size_t i = 0; i < value_len; ++i) {
    char ch = value[i];
    if (ch == setting_value_delemitr) {
      if (comma_count == 0) {
        int ascii_connection_type = element_text[0] - 48;  // saved in char but number
        result = CreateFromTypeSentinel(static_cast<core::ConnectionType>(ascii_connection_type), connection_path_t());
        if (!result) {
          DNOTREACHED() << "Unknown ascii_connection_type: " << ascii_connection_type;
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

        std::string sentinel_text;
        for (size_t j = i + 2; j < value_len; ++j) {
          ch = value[j];
          if (ch == magic_number || j == value_len - 1) {
            SentinelSettings sent;
            bool res = SentinelSettingsfromString(sentinel_text, &sent);
            if (res) {
              result->AddSentinel(sent);
            }
            sentinel_text.clear();
          } else {
            sentinel_text += ch;
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
