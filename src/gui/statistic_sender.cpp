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

#include "gui/statistic_sender.h"

#include <common/error.h>                    // for ErrnoError, ErrnoErrorValue
#include <common/net/socket_tcp.h>           // for ClientSocketTcp
#include <common/system_info/system_info.h>  // for SystemInfo, etc

#include <json-c/json_object.h>

#include "server_config_daemon/server_config.h"  // for FASTONOSQL_URL, etc

namespace fastonosql {
namespace gui {

StatisticSender::StatisticSender(QObject* parent) : QObject(parent) {}

void StatisticSender::routine() {
#if defined(FASTONOSQL)
  common::net::ClientSocketTcp client(common::net::HostAndPort(FASTONOSQL_URL, SERV_STATISTIC_PORT));
#elif defined(FASTOREDIS)
  common::net::ClientSocketTcp client(common::net::HostAndPort(FASTOREDIS_URL, SERV_STATISTIC_PORT));
#else
#error please specify url and port to send statistic information
#endif
  common::ErrnoError err = client.Connect();
  if (err && err->IsError()) {
    emit statisticSended(false);
    return;
  }

  json_object* stats_json = json_object_new_object();
  common::system_info::SystemInfo inf = common::system_info::currentSystemInfo();

  json_object* os_json = json_object_new_object();
  json_object_object_add(os_json, FIELD_OS_NAME, json_object_new_string(inf.name().c_str()));
  json_object_object_add(os_json, FIELD_OS_VERSION, json_object_new_string(inf.version().c_str()));
  json_object_object_add(os_json, FILED_OS_ARCH, json_object_new_string(inf.arch().c_str()));
  json_object_object_add(stats_json, FIELD_OS, os_json);

  json_object* project_json = json_object_new_object();
  json_object_object_add(project_json, FIELD_PROJECT_NAME, json_object_new_string(PROJECT_NAME));
  json_object_object_add(project_json, FIELD_PROJECT_VERSION, json_object_new_string(PROJECT_VERSION));
  json_object_object_add(project_json, FILED_PROJECT_ARCH, json_object_new_string(PROJECT_ARCH));
  json_object_object_add(stats_json, FIELD_PROJECT, project_json);

  const char* stats_json_string = json_object_get_string(stats_json);

  size_t nwrite = 0;
  err = client.Write(stats_json_string, strlen(stats_json_string), &nwrite);
  json_object_put(stats_json);
  if (err && err->IsError()) {
    emit statisticSended(false);
    DCHECK(!client.Close());
    return;
  }

  emit statisticSended(true);
  DCHECK(!client.Close());
  return;
}

}  // namespace gui
}  // namespace fastonosql
