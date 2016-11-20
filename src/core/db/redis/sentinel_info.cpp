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

#include "core/db/redis/sentinel_info.h"

#include <stddef.h>  // for size_t
#include <stdint.h>  // for uint16_t
#include <string.h>  // for strcmp

#include <string>  // for string, operator==
#include <vector>  // for vector

#include <hiredis/hiredis.h>  // for redisReply

#include <common/convert2string.h>  // for ConvertFromString
#include <common/macros.h>          // for NOTREACHED
#include <common/net/types.h>       // for HostAndPortAndSlot
#include <common/string_util.h>     // for Tokenize
#include <common/value.h>           // for ErrorValue, etc

#include "core/connection_types.h"  // for connectionTypes::REDIS, etc

#define NAME_FIELD "name"
#define TYPE_FIELD "flags"
#define HOSTNAME_FIELD "ip"
#define PORT_FIELD "port"

namespace fastonosql {
namespace core {
namespace redis {

DiscoverySentinelInfo::DiscoverySentinelInfo(const ServerCommonInfo& args)
    : ServerDiscoverySentinelInfo(REDIS, args) {}

common::Error MakeServerCommonInfo(struct redisReply* repl_info, ServerCommonInfo* info) {
  if (!repl_info || !info) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  ServerCommonInfo linf;
  for (size_t j = 0; j < repl_info->elements; j += 2) {
    if (strcmp(repl_info->element[j]->str, NAME_FIELD) == 0) {
      linf.name = repl_info->element[j + 1]->str;
    } else if (strcmp(repl_info->element[j]->str, TYPE_FIELD) == 0) {
      std::string str_type = repl_info->element[j + 1]->str;
      std::vector<std::string> flags;
      const size_t len = common::Tokenize(str_type, ",", &flags);
      for (size_t i = 0; i < len; ++i) {
        const std::string flag = flags[i];
        if (flag == "master") {
          linf.type = MASTER;
        } else if (flag == "slave") {
          linf.type = SLAVE;
        } else if (flag == "s_down") {
          linf.state = SDOWN;
        } else if (flag == "disconnected") {
          linf.cstate = SDISCONNECTED;
        } else {
          NOTREACHED();
        }
      }
    } else if (strcmp(repl_info->element[j]->str, HOSTNAME_FIELD) == 0) {
      linf.host.host = repl_info->element[j + 1]->str;
    } else if (strcmp(repl_info->element[j]->str, PORT_FIELD) == 0) {
      linf.host.port = common::ConvertFromString<uint16_t>(repl_info->element[j + 1]->str);
    }
  }

  *info = linf;
  return common::Error();
}

}  // namespace redis
}  // namespace core
}  // namespace fastonosql
