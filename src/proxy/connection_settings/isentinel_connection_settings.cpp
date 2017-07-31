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

#include "proxy/connection_settings/isentinel_connection_settings.h"

#include <stddef.h>  // for size_t
#include <stdint.h>  // for uint32_t, uint8_t

#include <sstream>

#include <common/utils.h>

#include "proxy/connection_settings_factory.h"

namespace fastonosql {
namespace proxy {

SentinelSettings::SentinelSettings() : sentinel(), sentinel_nodes() {}

std::string SentinelSettingsToString(const SentinelSettings& sent) {
  std::stringstream str;
  std::string sent_raw = sent.sentinel->ToString();
  str << common::utils::base64::encode64(sent_raw) << ',';

  std::string sents_raw;
  for (size_t i = 0; i < sent.sentinel_nodes.size(); ++i) {
    IConnectionSettingsBaseSPtr serv = sent.sentinel_nodes[i];
    if (serv) {
      sents_raw += magicNumber;
      sents_raw += serv->ToString();
    }
  }

  str << common::utils::base64::encode64(sents_raw);
  std::string res = str.str();
  return res;
}

bool SentinelSettingsfromString(const std::string& text, SentinelSettings* sent) {
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
        IConnectionSettingsBaseSPtr sent(ConnectionSettingsFactory::Instance().CreateFromString(sent_raw));
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
            IConnectionSettingsBaseSPtr ser(ConnectionSettingsFactory::Instance().CreateFromString(serText));
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

ISentinelSettingsBase::ISentinelSettingsBase(const connection_path_t& connectionName, core::connectionTypes type)
    : IConnectionSettings(connectionName, type), sentinel_nodes_() {}

ISentinelSettingsBase::sentinel_connections_t ISentinelSettingsBase::Sentinels() const {
  return sentinel_nodes_;
}

void ISentinelSettingsBase::AddSentinel(sentinel_connection_t sent) {
  sentinel_nodes_.push_back(sent);
}

std::string ISentinelSettingsBase::ToString() const {
  std::stringstream str;
  str << IConnectionSettings::ToString() << ',';
  for (size_t i = 0; i < sentinel_nodes_.size(); ++i) {
    sentinel_connection_t sent = sentinel_nodes_[i];
    str << magicNumber << SentinelSettingsToString(sent);
  }

  std::string res = str.str();
  return res;
}

}  // namespace proxy
}  // namespace fastonosql
