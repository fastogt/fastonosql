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

#include <string>

#include <common/convert2string.h>
#include <common/utils.h>

#include "proxy/connection_settings_factory.h"

#ifdef BUILD_WITH_REDIS
#include "proxy/db/redis/sentinel_settings.h"  // for SentinelSettings
#endif

#ifdef BUILD_WITH_PIKA
#include "proxy/db/pika/sentinel_settings.h"  // for SentinelSettings
#endif

namespace fastonosql {
namespace proxy {

namespace {
serialize_t SentinelSettingsToString(const SentinelSettings& sent) {
  std::ostringstream str;
  serialize_t sent_raw = ConnectionSettingsFactory::GetInstance().ConvertSettingsToString(sent.sentinel.get());
  common::char_buffer_t sent_raw_buff;
  common::utils::base64::encode64(sent_raw, &sent_raw_buff);
  str << sent_raw_buff << setting_value_delemitr;

  std::ostringstream sents_raw;
  for (size_t i = 0; i < sent.sentinel_nodes.size(); ++i) {
    IConnectionSettingsBaseSPtr serv = sent.sentinel_nodes[i];
    if (serv) {
      sents_raw << magic_number;
      sents_raw << ConnectionSettingsFactory::GetInstance().ConvertSettingsToString(serv.get());
    }
  }

  common::char_buffer_t buff;
  common::utils::base64::encode64(sents_raw.str(), &buff);
  str << buff;
  return str.str();
}

bool SentinelSettingsfromString(const serialize_t& text, SentinelSettings* sent) {
  if (text.empty() || !sent) {
    return false;
  }

  SentinelSettings result;
  size_t value_len = text.size();

  uint8_t comma_count = 0;
  serialize_t element_text;
  for (size_t i = 0; i < value_len; ++i) {
    serialize_t::value_type ch = text[i];
    if (ch == setting_value_delemitr || i == value_len - 1) {
      if (comma_count == 0) {
        common::char_buffer_t sent_raw;
        if (!common::utils::base64::decode64(element_text, &sent_raw)) {
          return false;
        }

        const serialize_t sent_raw_str = common::ConvertToString(sent_raw);
        IConnectionSettingsBaseSPtr sent(
            ConnectionSettingsFactory::GetInstance().CreateSettingsFromString(sent_raw_str));
        if (!sent) {
          return false;
        }

        result.sentinel = sent;

        serialize_t server_text;
        serialize_t end_text(text.begin() + i + 1, text.end());

        common::char_buffer_t raw_sent;
        common::utils::base64::decode64(end_text, &raw_sent);
        size_t len = raw_sent.size();
        for (size_t j = 0; j < len; ++j) {
          ch = raw_sent[j];
          if (ch == magic_number || j == len - 1) {
            IConnectionSettingsBaseSPtr ser(
                ConnectionSettingsFactory::GetInstance().CreateSettingsFromString(server_text));
            if (ser) {
              result.sentinel_nodes.push_back(ser);
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

  *sent = result;
  return true;
}

}  // namespace

serialize_t SentinelConnectionSettingsFactory::ConvertSettingsToString(ISentinelSettingsBase* settings) {
  std::ostringstream str;
  str << ConnectionSettingsFactory::GetInstance().ConvertSettingsToString(settings) << setting_value_delemitr;
  auto nodes = settings->GetSentinels();
  for (size_t i = 0; i < nodes.size(); ++i) {
    auto sent = nodes[i];
    str << magic_number << SentinelSettingsToString(sent);
  }

  return str.str();
}

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

ISentinelSettingsBase* SentinelConnectionSettingsFactory::CreateFromStringSentinel(const serialize_t& value) {
  if (value.empty()) {
    DNOTREACHED();
    return nullptr;
  }

  ISentinelSettingsBase* result = nullptr;
  const size_t value_len = value.size();

  uint8_t comma_count = 0;
  serialize_t element_text;

  for (size_t i = 0; i < value_len; ++i) {
    serialize_t::value_type ch = value[i];
    if (ch == setting_value_delemitr) {
      if (comma_count == 0) {
        uint8_t connection_type;
        if (common::ConvertFromString(element_text, &connection_type)) {
          result = CreateFromTypeSentinel(static_cast<core::ConnectionType>(connection_type), connection_path_t());
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
        if (common::ConvertFromString(element_text, &ms_time)) {
          result->SetLoggingMsTimeInterval(ms_time);
        }

        serialize_t sentinel_text;
        for (size_t j = i + 2; j < value_len; ++j) {
          ch = value[j];
          if (ch == magic_number || j == value_len - 1) {
            if (j == value_len - 1) {
              sentinel_text.push_back(ch);
            }

            SentinelSettings sent;
            bool res = SentinelSettingsfromString(sentinel_text, &sent);
            if (res) {
              result->AddSentinel(sent);
            }
            sentinel_text.clear();
          } else {
            sentinel_text.push_back(ch);
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
