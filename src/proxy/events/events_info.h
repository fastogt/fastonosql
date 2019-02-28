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
    along with FastoNoSQL. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <string>
#include <vector>

#include <common/qt/utils_qt.h>

#include <fastonosql/core/command_info.h>
#include <fastonosql/core/database/idatabase_info.h>
#include <fastonosql/core/db_key.h>
#include <fastonosql/core/server/iserver_info.h>
#include <fastonosql/core/server_property_info.h>

#if defined(PRO_VERSION) || defined(ENTERPRISE_VERSION)
#include <fastonosql/core/module_info.h>
#endif

#include <fastonosql/core/global.h>

#include "proxy/db_client.h"
#include "proxy/db_ps_channel.h"

namespace fastonosql {
namespace proxy {
namespace events_info {

class EventInfoBase : public common::qt::EventInfo<common::Error> {
 public:
  typedef common::qt::EventInfo<common::Error> base_class;
  explicit EventInfoBase(initiator_type sender, error_type er = error_type());
  EventInfoBase(initiator_type sender, common::time64_t time_start, error_type er = error_type());
  common::time64_t GetElapsedTime() const;

 private:
  const common::time64_t time_start_;
};

struct ConnectInfoRequest : public EventInfoBase {
  typedef EventInfoBase base_class;
  explicit ConnectInfoRequest(initiator_type sender, error_type er = error_type());
};

struct ConnectInfoResponse : ConnectInfoRequest {
  typedef ConnectInfoRequest base_class;
  explicit ConnectInfoResponse(const base_class& request);
};

struct BackupInfoRequest : public EventInfoBase {
  typedef EventInfoBase base_class;
  BackupInfoRequest(initiator_type sender, const std::string& path, error_type er = error_type());
  std::string path;
};

struct BackupInfoResponse : BackupInfoRequest {
  typedef BackupInfoRequest base_class;
  explicit BackupInfoResponse(const base_class& request);
};

struct RestoreInfoRequest : public EventInfoBase {
  typedef EventInfoBase base_class;
  RestoreInfoRequest(initiator_type sender, const std::string& path, error_type er = error_type());
  std::string path;
};

struct RestoreInfoResponse : RestoreInfoRequest {
  typedef RestoreInfoRequest base_class;
  explicit RestoreInfoResponse(const base_class& request);
};

struct DiscoveryInfoRequest : public EventInfoBase {
  typedef EventInfoBase base_class;
  explicit DiscoveryInfoRequest(initiator_type sender, error_type er = error_type());
};

struct DiscoveryInfoResponse : DiscoveryInfoRequest {
  typedef DiscoveryInfoRequest base_class;
  explicit DiscoveryInfoResponse(const base_class& request);

  core::IDataBaseInfoSPtr dbinfo;
  std::vector<const core::CommandInfo*> commands;
#if defined(PRO_VERSION) || defined(ENTERPRISE_VERSION)
  // only for redis
  std::vector<core::ModuleInfo> loaded_modules;
#endif
};

struct EnterModeInfo : public EventInfoBase {
  typedef EventInfoBase base_class;
  EnterModeInfo(initiator_type sender, core::ConnectionMode mode, error_type er = error_type());
  core::ConnectionMode mode;
};

struct LeaveModeInfo : public EventInfoBase {
  typedef EventInfoBase base_class;
  LeaveModeInfo(initiator_type sender, core::ConnectionMode mode, error_type er = error_type());
  core::ConnectionMode mode;
};

struct CommandRootCreatedInfo : public EventInfoBase {
  typedef EventInfoBase base_class;
  CommandRootCreatedInfo(initiator_type sender, core::FastoObjectIPtr root, error_type er = error_type());

  core::FastoObjectIPtr root;
};

struct CommandRootCompleatedInfo : public EventInfoBase {
  typedef EventInfoBase base_class;
  CommandRootCompleatedInfo(initiator_type sender, core::FastoObjectIPtr root, error_type er = error_type());
  CommandRootCompleatedInfo(initiator_type sender,
                            common::time64_t timest,
                            core::FastoObjectIPtr root,
                            error_type er = error_type());

  core::FastoObjectIPtr root;
};

struct DisConnectInfoRequest : public EventInfoBase {
  typedef EventInfoBase base_class;
  explicit DisConnectInfoRequest(initiator_type sender, error_type er = error_type());
};

struct DisConnectInfoResponse : DisConnectInfoRequest {
  typedef DisConnectInfoRequest base_class;
  explicit DisConnectInfoResponse(const base_class& request);
};

struct ExecuteInfoRequest : public EventInfoBase {
  typedef EventInfoBase base_class;
  ExecuteInfoRequest(initiator_type sender,
                     const core::command_buffer_t& text,
                     size_t repeat = 0,
                     common::time64_t msec_repeat_interval = 0,
                     bool history = true,
                     bool silence = false,
                     core::CmdLoggingType logtype = core::C_USER,
                     error_type er = error_type());

  const core::command_buffer_t text;
  const size_t repeat;
  const common::time64_t msec_repeat_interval;
  const bool history;
  const bool silence;
  const core::CmdLoggingType logtype;
};

struct ExecuteInfoResponse : ExecuteInfoRequest {
  typedef ExecuteInfoRequest base_class;
  explicit ExecuteInfoResponse(const base_class& request);

  std::vector<core::FastoObjectCommandIPtr> executed_commands;
};

struct LoadDatabasesInfoRequest : public EventInfoBase {
  typedef EventInfoBase base_class;
  explicit LoadDatabasesInfoRequest(initiator_type sender, error_type er = error_type());
};

struct LoadDatabasesInfoResponse : LoadDatabasesInfoRequest {
  typedef LoadDatabasesInfoRequest base_class;
  typedef std::vector<core::IDataBaseInfoSPtr> database_info_cont_type;
  explicit LoadDatabasesInfoResponse(const base_class& request);

  database_info_cont_type databases;
};

struct LoadDatabaseContentRequest : public EventInfoBase {
  typedef EventInfoBase base_class;
  LoadDatabaseContentRequest(initiator_type sender,
                             core::IDataBaseInfoSPtr inf,
                             const core::pattern_t& pattern,
                             core::keys_limit_t keys_count,
                             core::cursor_t cursor = 0,
                             error_type er = error_type());

  core::IDataBaseInfoSPtr inf;
  const core::pattern_t pattern;
  const core::keys_limit_t keys_count;  // requested
  const core::cursor_t cursor_in;
};

struct LoadDatabaseContentResponse : LoadDatabaseContentRequest {
  typedef LoadDatabaseContentRequest base_class;
  typedef std::vector<core::NDbKValue> keys_container_t;
  explicit LoadDatabaseContentResponse(const base_class& request);

  keys_container_t keys;
  core::cursor_t cursor_out;
  core::keys_limit_t db_keys_count;  // total keys count
};

struct LoadServerChannelsRequest : public EventInfoBase {
  typedef EventInfoBase base_class;
  LoadServerChannelsRequest(initiator_type sender, const std::string& pattern, error_type er = error_type());

  const std::string pattern;
};

struct LoadServerChannelsResponse : LoadServerChannelsRequest {
  typedef LoadServerChannelsRequest base_class;
  typedef std::vector<proxy::NDbPSChannel> channels_container_t;
  explicit LoadServerChannelsResponse(const base_class& request);

  channels_container_t channels;
};

struct LoadServerClientsRequest : public EventInfoBase {
  typedef EventInfoBase base_class;
  explicit LoadServerClientsRequest(initiator_type sender, error_type er = error_type());
};

struct LoadServerClientsResponse : LoadServerClientsRequest {
  typedef LoadServerClientsRequest base_class;
  typedef std::vector<proxy::NDbClient> clients_container_t;
  explicit LoadServerClientsResponse(const base_class& request);

  clients_container_t clients;
};

struct ServerInfoRequest : public EventInfoBase {
  typedef EventInfoBase base_class;
  explicit ServerInfoRequest(initiator_type sender, error_type er = error_type());
};

class ServerInfoResponse : public ServerInfoRequest {
 public:
  typedef ServerInfoRequest base_class;
  explicit ServerInfoResponse(const base_class& request);

  core::IServerInfoSPtr GetInfo() const;
  void SetInfo(core::IServerInfoSPtr inf);

 private:
  core::IServerInfoSPtr info_;
};

struct ServerInfoHistoryRequest : public EventInfoBase {
  typedef EventInfoBase base_class;
  explicit ServerInfoHistoryRequest(initiator_type sender, error_type er = error_type());
};

class ServerInfoHistoryResponse : public ServerInfoHistoryRequest {
 public:
  typedef ServerInfoHistoryRequest base_class;
  typedef std::vector<core::ServerInfoSnapShoot> infos_container_type;
  explicit ServerInfoHistoryResponse(const base_class& request);

  infos_container_type GetInfos() const;
  void SetInfos(const infos_container_type& inf);

 private:
  infos_container_type infos_;
};

struct ClearServerHistoryRequest : public EventInfoBase {
  typedef EventInfoBase base_class;
  explicit ClearServerHistoryRequest(initiator_type sender, error_type er = error_type());
};

struct ClearServerHistoryResponse : public ClearServerHistoryRequest {
  typedef ClearServerHistoryRequest base_class;
  explicit ClearServerHistoryResponse(const base_class& request);
};

struct ServerPropertyInfoRequest : public EventInfoBase {
  typedef EventInfoBase base_class;
  explicit ServerPropertyInfoRequest(initiator_type sender, error_type er = error_type());
};

struct ServerPropertyInfoResponse : ServerPropertyInfoRequest {
  typedef ServerPropertyInfoRequest base_class;
  explicit ServerPropertyInfoResponse(const base_class& request);

  core::ServerPropertiesInfo info;
};

struct ChangeServerPropertyInfoRequest : public EventInfoBase {
  typedef EventInfoBase base_class;
  ChangeServerPropertyInfoRequest(initiator_type sender, const core::property_t& pt, error_type er = error_type());

  core::property_t new_item;
};

struct ChangeServerPropertyInfoResponse : ChangeServerPropertyInfoRequest {
  typedef ChangeServerPropertyInfoRequest base_class;
  explicit ChangeServerPropertyInfoResponse(const base_class& request);

  bool is_change;
};

struct ProgressInfoResponse {
  explicit ProgressInfoResponse(int pr);

  const int progress;
};

}  // namespace events_info
}  // namespace proxy
}  // namespace fastonosql
