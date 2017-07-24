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

#include "core/db/redis/cluster_infos.h"

#include <stddef.h>  // for size_t
#include <memory>    // for __shared_ptr

#include <common/convert2string.h>  // for ConvertFromString
#include <common/macros.h>          // for NOTREACHED
#include <common/net/types.h>       // for HostAndPortAndSlot, etc
#include <common/string_util.h>     // for Tokenize
#include <common/value.h>           // for ErrorValue, etc

#include "core/connection_types.h"  // for connectionTypes::REDIS, etc

#define MARKER "\n"

namespace fastonosql {
namespace core {
namespace redis {

DiscoveryClusterInfo::DiscoveryClusterInfo(const ServerCommonInfo& info, bool self)
    : ServerDiscoveryClusterInfo(REDIS, info, self) {}

namespace {

common::Error MakeServerCommonInfoFromLine(const std::string& line, ServerCommonInfo* info, bool* self) {
  if (line.empty() || !info || !self) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  ServerCommonInfo linfo;
  bool lself = false;
  int fieldpos = 0;
  std::string word;
  const size_t len = line.size();
  for (size_t i = 0; i < len; ++i) {
    char ch = line[i];
    if (ch != ' ') {
      word += ch;
    }

    if (ch == ' ' || i == len - 1) {
      switch (fieldpos) {
        case 0: {
          linfo.name = word;
          break;
        }
        case 1: {
          common::net::HostAndPortAndSlot hs;
          if (common::ConvertFromString(word, &hs)) {
            linfo.host = hs;
          }
          break;
        }
        case 2: {
          std::vector<std::string> flags;
          const size_t len = common::Tokenize(word, ",", &flags);
          for (size_t i = 0; i < len; ++i) {
            const std::string flag = flags[i];
            if (flag == "master") {
              linfo.type = MASTER;
            } else if (flag == "slave") {
              linfo.type = SLAVE;
            } else if (flag == "myself") {
              lself = true;
            } else if (flag == "fail") {
            } else {
              NOTREACHED();
            }
          }
          break;
        }
        case 3: {  // dep
          break;
        }
        case 4: {  //
          break;
        }
        case 5: {  //
          break;
        }
        case 6: {  //
          break;
        }
        case 7: {  // connected/disconnected
          if (word == "connected") {
            linfo.state = SUP;
          } else if (word == "disconnected") {
            linfo.state = SDOWN;
          } else {
            NOTREACHED();
          }
          break;
        }
        case 8: {  // slots
          break;
        }
        default:
          break;
      }
      word.clear();
      ++fieldpos;
    }
  }

  *info = linfo;
  *self = lself;
  return common::Error();
}

}  // namespace

common::Error makeDiscoveryClusterInfo(const common::net::HostAndPort& parentHost,
                                       const std::string& text,
                                       std::vector<ServerDiscoveryClusterInfoSPtr>* infos) {
  if (text.empty() || !infos) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
    ;
  }

  size_t pos = 0;
  size_t start = 0;

  while ((pos = text.find(MARKER, start)) != std::string::npos) {
    std::string line = text.substr(start, pos - start);

    ServerCommonInfo inf;
    bool self = false;
    common::Error lerr = MakeServerCommonInfoFromLine(line, &inf, &self);
    if (lerr && lerr->IsError()) {
      continue;
    }
    if (common::net::IsLocalHost(inf.host.host)) {  // for direct connection
      inf.host.host = parentHost.host;
    }

    ServerDiscoveryClusterInfoSPtr ser(new DiscoveryClusterInfo(inf, self));
    infos->push_back(ser);
    start = pos + 1;
  }

  return common::Error();
}

}  // namespace redis
}  // namespace core
}  // namespace fastonosql
