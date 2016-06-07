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

#include "core/redis/sentinel_info.h"

#include <stdlib.h>

#define NAME_FIELD "name"
#define TYPE_FIELD "flags"
#define HOSTNAME_FIELD "ip"
#define PORT_FIELD "port"

namespace fastonosql {
namespace core {
namespace redis {

DiscoverySentinelInfo::DiscoverySentinelInfo(const ServerCommonInfo& args)
  : ServerDiscoverySentinelInfo(REDIS, args){
}

common::Error makeServerCommonInfo(struct redisReply* repl_info, ServerCommonInfo* info) {
  if (!repl_info || !info) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  ServerCommonInfo linf;
  for (size_t j = 0; j < repl_info->elements; j += 2) {
    if (strcmp(repl_info->element[j]->str, NAME_FIELD) == 0) {
      linf.name = repl_info->element[j + 1]->str;
    } else if (strcmp(repl_info->element[j]->str, TYPE_FIELD) == 0) {
       std::string str_type = repl_info->element[j + 1]->str;
       if (str_type == "master") {
         linf.type = MASTER;
       } else if (str_type == "slave") {
         linf.type = SLAVE;
       } else {
         NOTREACHED();
       }
    } else if (strcmp(repl_info->element[j]->str, HOSTNAME_FIELD) == 0) {
       linf.host.host = repl_info->element[j + 1]->str;
    } else if (strcmp(repl_info->element[j]->str, PORT_FIELD) == 0) {
       linf.host.port = atoi(repl_info->element[j + 1]->str);
    }
  }

  *info = linf;
  return common::Error();
}

}  // namespace redis
}  // namespace core
}  // namespace fastonosql
