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

#pragma once

#include <common/qt/utils_qt.h>  // for EventInfo

#include "core/database/idatabase_info.h"
#include "core/db_key.h"  // for NDbKValue
#include "core/db_ps_channel.h"
#include "core/server/iserver_info.h"   // for IDataBaseInfoSPtr, IServerInf...
#include "core/server_property_info.h"  // for property_t, ServerPropertiesInfo

#include "core/global.h"  // for FastoObjectIPtr

namespace fastonosql {
namespace proxy {
namespace events_info {

class EventInfoBase : public common::qt::EventInfo<common::Error> {
 public:
  typedef common::qt::EventInfo<common::Error> base_class;
  explicit EventInfoBase(initiator_type sender, error_type er = error_type());
  EventInfoBase(initiator_type sender, common::time64_t time_start, error_type er = error_type());
  common::time64_t ElapsedTime() const;

 private:
  const common::time64_t time_start_;
};

struct ConnectInfoRequest : public EventInfoBase {
  typedef EventInfoBase base_class;
  explicit ConnectInfoRequest(initiator_type sender, error_type er = error_type());
};

struct ConnectInfoResponce : ConnectInfoRequest {
  typedef ConnectInfoRequest base_class;
  explicit ConnectInfoResponce(const base_class& request);
};

struct BackupInfoRequest : public EventInfoBase {
  typedef EventInfoBase base_class;
  BackupInfoRequest(initiator_type sender, const std::string& path, error_type er = error_type());
  std::string path;
};

struct BackupInfoResponce : BackupInfoRequest {
  typedef BackupInfoRequest base_class;
  explicit BackupInfoResponce(const base_class& request);
};

struct ExportInfoRequest : public EventInfoBase {
  typedef EventInfoBase base_class;
  ExportInfoRequest(initiator_type sender, const std::string& path, error_type er = error_type());
  std::string path;
};

struct ExportInfoResponce : ExportInfoRequest {
  typedef ExportInfoRequest base_class;
  explicit ExportInfoResponce(const base_class& request);
};

struct DiscoveryInfoRequest : public EventInfoBase {
  typedef EventInfoBase base_class;
  explicit DiscoveryInfoRequest(initiator_type sender, error_type er = error_type());
};

struct DiscoveryInfoResponce : DiscoveryInfoRequest {
  typedef DiscoveryInfoRequest base_class;
  explicit DiscoveryInfoResponce(const base_class& request);

  core::IServerInfoSPtr sinfo;
  core::IDataBaseInfoSPtr dbinfo;
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

struct DisConnectInfoResponce : DisConnectInfoRequest {
  typedef DisConnectInfoRequest base_class;
  explicit DisConnectInfoResponce(const base_class& request);
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

struct ExecuteInfoResponce : ExecuteInfoRequest {
  typedef ExecuteInfoRequest base_class;
  explicit ExecuteInfoResponce(const base_class& request);
};

struct LoadDatabasesInfoRequest : public EventInfoBase {
  typedef EventInfoBase base_class;
  explicit LoadDatabasesInfoRequest(initiator_type sender, error_type er = error_type());
};

struct LoadDatabasesInfoResponce : LoadDatabasesInfoRequest {
  typedef LoadDatabasesInfoRequest base_class;
  typedef std::vector<core::IDataBaseInfoSPtr> database_info_cont_type;
  explicit LoadDatabasesInfoResponce(const base_class& request);

  database_info_cont_type databases;
};

struct LoadDatabaseContentRequest : public EventInfoBase {
  typedef EventInfoBase base_class;
  LoadDatabaseContentRequest(initiator_type sender,
                             core::IDataBaseInfoSPtr inf,
                             const std::string& pattern,
                             size_t countKeys,
                             uint64_t cursor = 0,
                             error_type er = error_type());

  core::IDataBaseInfoSPtr inf;
  const std::string pattern;
  size_t count_keys;
  const uint64_t cursor_in;
};

struct LoadDatabaseContentResponce : LoadDatabaseContentRequest {
  typedef LoadDatabaseContentRequest base_class;
  typedef std::vector<core::NDbKValue> keys_container_t;
  explicit LoadDatabaseContentResponce(const base_class& request);

  keys_container_t keys;
  uint64_t cursor_out;
  size_t db_keys_count;
};

struct LoadServerChannelsRequest : public EventInfoBase {
  typedef EventInfoBase base_class;
  LoadServerChannelsRequest(initiator_type sender, const std::string& pattern, error_type er = error_type());

  const std::string pattern;
};

struct LoadServerChannelsResponce : LoadServerChannelsRequest {
  typedef LoadServerChannelsRequest base_class;
  typedef std::vector<core::NDbPSChannel> channels_container_t;
  explicit LoadServerChannelsResponce(const base_class& request);

  channels_container_t channels;
};

struct ServerInfoRequest : public EventInfoBase {
  typedef EventInfoBase base_class;
  explicit ServerInfoRequest(initiator_type sender, error_type er = error_type());
};

class ServerInfoResponce : public ServerInfoRequest {
 public:
  typedef ServerInfoRequest base_class;
  explicit ServerInfoResponce(const base_class& request);
  ~ServerInfoResponce();

  core::IServerInfoSPtr info() const;
  void setInfo(core::IServerInfoSPtr inf);

 private:
  core::IServerInfoSPtr info_;
};

struct ServerInfoHistoryRequest : public EventInfoBase {
  typedef EventInfoBase base_class;
  explicit ServerInfoHistoryRequest(initiator_type sender, error_type er = error_type());
};

class ServerInfoHistoryResponce : public ServerInfoHistoryRequest {
 public:
  typedef ServerInfoHistoryRequest base_class;
  typedef std::vector<core::ServerInfoSnapShoot> infos_container_type;
  explicit ServerInfoHistoryResponce(const base_class& request);

  infos_container_type infos() const;
  void setInfos(const infos_container_type& inf);

 private:
  infos_container_type infos_;
};

struct ClearServerHistoryRequest : public EventInfoBase {
  typedef EventInfoBase base_class;
  explicit ClearServerHistoryRequest(initiator_type sender, error_type er = error_type());
};

struct ClearServerHistoryResponce : public ClearServerHistoryRequest {
  typedef ClearServerHistoryRequest base_class;
  explicit ClearServerHistoryResponce(const base_class& request);
};

struct ServerPropertyInfoRequest : public EventInfoBase {
  typedef EventInfoBase base_class;
  explicit ServerPropertyInfoRequest(initiator_type sender, error_type er = error_type());
};

struct ServerPropertyInfoResponce : ServerPropertyInfoRequest {
  typedef ServerPropertyInfoRequest base_class;
  explicit ServerPropertyInfoResponce(const base_class& request);

  core::ServerPropertiesInfo info;
};

struct ChangeServerPropertyInfoRequest : public EventInfoBase {
  typedef EventInfoBase base_class;
  ChangeServerPropertyInfoRequest(initiator_type sender, const core::property_t& pt, error_type er = error_type());

  core::property_t new_item;
};

struct ChangeServerPropertyInfoResponce : ChangeServerPropertyInfoRequest {
  typedef ChangeServerPropertyInfoRequest base_class;
  explicit ChangeServerPropertyInfoResponce(const base_class& request);

  bool is_change;
};

struct ProgressInfoResponce {
  explicit ProgressInfoResponce(int pr);

  const int progress;
};

}  // namespace events_info
}  // namespace proxy
}  // namespace fastonosql
