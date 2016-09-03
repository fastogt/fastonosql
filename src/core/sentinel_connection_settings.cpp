/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    FastoNoSQL is free software: you can redistribute it
   and/or modify
    it under the terms of the GNU General Public License as
   published by
    the Free Software Foundation, either version 3 of the
   License, or
    (at your option) any later version.

    FastoNoSQL is distributed in the hope that it will be
   useful,
    but WITHOUT ANY WARRANTY; without even the implied
   warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General
   Public License
    along with FastoNoSQL.  If not, see
   <http://www.gnu.org/licenses/>.
*/

#include "core/sentinel_connection_settings.h"

#include <sstream>

#include "common/convert2string.h"
#include "common/utils.h"

#ifdef BUILD_WITH_REDIS
#include "core/redis/sentinel_settings.h"  // for SentinelSettings
#endif

namespace fastonosql {
namespace core {

SentinelSettings::SentinelSettings() : sentinel(), sentinel_nodes() {}

std::string sentinelSettingsToString(const SentinelSettings& sent) {
  std::stringstream str;
  std::string sent_raw = sent.sentinel->toString();
  str << common::utils::base64::encode64(sent_raw) << ',';

  std::string sents_raw;
  for (size_t i = 0; i < sent.sentinel_nodes.size(); ++i) {
    IConnectionSettingsBaseSPtr serv = sent.sentinel_nodes[i];
    if (serv) {
      sents_raw += magicNumber;
      sents_raw += serv->toString();
    }
  }

  str << common::utils::base64::encode64(sents_raw);
  std::string res = str.str();
  return res;
}

bool sentinelSettingsfromString(const std::string& text, SentinelSettings* sent) {
  if (text.empty() || !sent) {
    return false;
  }

  SentinelSettings result;
  size_t len = text.size();

  uint8_t commaCount = 0;
  std::string elText;
  for (size_t i = 0; i < len; ++i) {
    char ch = text[i];
    if (ch == ',' || i == len - 1) {
      if (commaCount == 0) {
        std::string sent_raw = common::utils::base64::decode64(elText);
        IConnectionSettingsBaseSPtr sent(IConnectionSettingsBase::fromString(sent_raw));
        if (!sent) {
          return false;
        }

        result.sentinel = sent;

        std::string serText;
        std::string raw_sent = common::utils::base64::decode64(text.substr(i + 1));
        len = raw_sent.length();
        for (size_t j = 0; j < len; ++j) {
          ch = raw_sent[j];
          if (ch == magicNumber || j == len - 1) {
            IConnectionSettingsBaseSPtr ser(IConnectionSettingsBase::fromString(serText));
            if (ser) {
              result.sentinel_nodes.push_back(ser);
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

  *sent = result;
  return true;
}

//

ISentinelSettingsBase::ISentinelSettingsBase(const connection_path_t& connectionName,
                                             connectionTypes type)
    : IConnectionSettings(connectionName, type), sentinel_nodes_() {}

ISentinelSettingsBase::sentinel_connections_t ISentinelSettingsBase::sentinels() const {
  return sentinel_nodes_;
}

void ISentinelSettingsBase::addSentinel(sentinel_connection_t sent) {
  sentinel_nodes_.push_back(sent);
}

ISentinelSettingsBase* ISentinelSettingsBase::createFromType(connectionTypes type,
                                                             const connection_path_t& conName) {
#ifdef BUILD_WITH_REDIS
  if (type == REDIS) {
    return new redis::SentinelSettings(conName);
  }
#endif

  NOTREACHED();
  return nullptr;
}

ISentinelSettingsBase* ISentinelSettingsBase::fromString(const std::string& val) {
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
            SentinelSettings sent;
            bool res = sentinelSettingsfromString(serText, &sent);
            if (res) {
              result->addSentinel(sent);
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

std::string ISentinelSettingsBase::toString() const {
  std::stringstream str;
  str << IConnectionSettings::toString() << ',';
  for (size_t i = 0; i < sentinel_nodes_.size(); ++i) {
    sentinel_connection_t sent = sentinel_nodes_[i];
    str << magicNumber << sentinelSettingsToString(sent);
  }

  std::string res = str.str();
  return res;
}

}  // namespace core
}  // namespace fastonosql
