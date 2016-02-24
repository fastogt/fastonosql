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

#pragma once

#include <vector>
#include <string>

#include "core/core_fwd.h"
#include "common/qt/utils_qt.h"

namespace fastonosql {
namespace events_info {

class EventInfoBase
  : public common::utils_qt::EventInfo<common::Error> {
 public:
  typedef common::utils_qt::EventInfo<common::Error > base_class;
  explicit EventInfoBase(initiator_type sender, error_type er = error_type());
  EventInfoBase(initiator_type sender, const common::time64_t time_start,
                error_type er = error_type());
  common::time64_t elapsedTime() const;

 private:
  const common::time64_t time_start_;
};

struct ConnectInfoRequest
  : public EventInfoBase {
  typedef EventInfoBase base_class;
  explicit ConnectInfoRequest(initiator_type sender, error_type er = error_type());
};

struct ConnectInfoResponce
  : ConnectInfoRequest {
  typedef ConnectInfoRequest base_class;
  explicit ConnectInfoResponce(const base_class &request);
};

struct ShutDownInfoRequest
  : public EventInfoBase {
  typedef EventInfoBase base_class;
  explicit ShutDownInfoRequest(initiator_type sender, error_type er = error_type());
};

struct ShutDownInfoResponce
  : ShutDownInfoRequest {
  typedef ShutDownInfoRequest base_class;
  explicit ShutDownInfoResponce(const base_class &request);
};

struct BackupInfoRequest
  : public EventInfoBase {
  typedef EventInfoBase base_class;
  BackupInfoRequest(initiator_type sender, const std::string& path, error_type er = error_type());
  std::string path;
};

struct BackupInfoResponce
  : BackupInfoRequest {
  typedef BackupInfoRequest base_class;
  explicit BackupInfoResponce(const base_class &request);
};

struct ExportInfoRequest
  : public EventInfoBase {
  typedef EventInfoBase base_class;
  ExportInfoRequest(initiator_type sender, const std::string& path, error_type er = error_type());
  std::string path;
};

struct ExportInfoResponce
  : ExportInfoRequest {
  typedef ExportInfoRequest base_class;
  explicit ExportInfoResponce(const base_class &request);
};

struct ChangePasswordRequest
  : public EventInfoBase {
  typedef EventInfoBase base_class;
  ChangePasswordRequest(initiator_type sender, const std::string& oldPassword,
                        const std::string& newPassword, error_type er = error_type());
  std::string old_password;
  std::string new_password;
};

struct ChangePasswordResponce
  : ChangePasswordRequest {
  typedef ChangePasswordRequest base_class;
  explicit ChangePasswordResponce(const base_class& request);
};

struct ChangeMaxConnectionRequest
  : public EventInfoBase {
  typedef EventInfoBase base_class;
  ChangeMaxConnectionRequest(initiator_type sender, int maxConnection,
                             error_type er = error_type());
  int max_connection;
};

struct ChangeMaxConnectionResponce
  : ChangeMaxConnectionRequest {
  typedef ChangeMaxConnectionRequest base_class;
  explicit ChangeMaxConnectionResponce(const base_class& request);
};

struct ProcessConfigArgsInfoRequest
  : public EventInfoBase {
  typedef EventInfoBase base_class;
  explicit ProcessConfigArgsInfoRequest(initiator_type sender, error_type er = error_type());
};

struct ProcessConfigArgsInfoResponce
  : ProcessConfigArgsInfoRequest {
  typedef ProcessConfigArgsInfoRequest base_class;
  explicit ProcessConfigArgsInfoResponce(const base_class &request);
};

struct DiscoveryInfoRequest
  : public EventInfoBase {
  typedef EventInfoBase base_class;
  explicit DiscoveryInfoRequest(initiator_type sender, error_type er = error_type());
};

struct DiscoveryInfoResponce
  : DiscoveryInfoRequest {
  typedef DiscoveryInfoRequest base_class;
  explicit DiscoveryInfoResponce(const base_class &request);

  ServerInfoSPtr sinfo;
  ServerDiscoveryInfoSPtr dinfo;
  IDataBaseInfoSPtr dbinfo;
};

struct EnterModeInfo
  : public EventInfoBase {
  typedef EventInfoBase base_class;
  EnterModeInfo(initiator_type sender, ConnectionMode mode, error_type er = error_type());
  ConnectionMode mode;
};

struct LeaveModeInfo
  : public EventInfoBase {
  typedef EventInfoBase base_class;
  LeaveModeInfo(initiator_type sender, ConnectionMode mode, error_type er = error_type());
  ConnectionMode mode;
};

struct CommandRootCreatedInfo
  : public EventInfoBase {
  typedef EventInfoBase base_class;
  CommandRootCreatedInfo(initiator_type sender, FastoObjectIPtr root, error_type er = error_type());

  FastoObjectIPtr root;
};

struct CommandRootCompleatedInfo
  : public EventInfoBase {
  typedef EventInfoBase base_class;
  CommandRootCompleatedInfo(initiator_type sender,
                            FastoObjectIPtr root, error_type er = error_type());
  CommandRootCompleatedInfo(initiator_type sender, common::time64_t timest,
                            FastoObjectIPtr root, error_type er = error_type());

  FastoObjectIPtr root;
};

struct DisConnectInfoRequest
  : public EventInfoBase {
  typedef EventInfoBase base_class;
  explicit DisConnectInfoRequest(initiator_type sender, error_type er = error_type());
};

struct DisConnectInfoResponce
  : DisConnectInfoRequest {
  typedef DisConnectInfoRequest base_class;
  explicit DisConnectInfoResponce(const base_class &request);
};

struct ExecuteInfoRequest
  : public EventInfoBase {
  typedef EventInfoBase base_class;
  ExecuteInfoRequest(initiator_type sender, const std::string &text,
                     const std::vector<std::string>& args = std::vector<std::string>(),
                     error_type er = error_type());

  const std::string text;
  const std::vector<std::string> args;
};

struct LoadDatabasesInfoRequest
  : public EventInfoBase {
  typedef EventInfoBase base_class;
  explicit LoadDatabasesInfoRequest(initiator_type sender, error_type er = error_type());
};

struct LoadDatabasesInfoResponce
  : LoadDatabasesInfoRequest {
  typedef LoadDatabasesInfoRequest base_class;
  typedef std::vector<IDataBaseInfoSPtr> database_info_cont_type;
  explicit LoadDatabasesInfoResponce(const base_class &request);

  database_info_cont_type databases;
};

struct LoadDatabaseContentRequest
  : public EventInfoBase {
  typedef EventInfoBase base_class;
  LoadDatabaseContentRequest(initiator_type sender, IDataBaseInfoSPtr inf,
                             const std::string& pattern, uint32_t countKeys,
             uint32_t cursor = 0, error_type er = error_type());

  IDataBaseInfoSPtr inf;
  std::string pattern;
  uint32_t count_keys;
  const uint32_t cursor_in;
};

struct LoadDatabaseContentResponce
  : LoadDatabaseContentRequest {
  typedef LoadDatabaseContentRequest base_class;
  typedef std::vector<NDbKValue> keys_cont_type;
  explicit LoadDatabaseContentResponce(const base_class &request);

  keys_cont_type keys;
  uint32_t cursor_out;
};

struct SetDefaultDatabaseRequest
  : public EventInfoBase {
  typedef EventInfoBase base_class;
  SetDefaultDatabaseRequest(initiator_type sender, IDataBaseInfoSPtr inf,
                            error_type er = error_type());

  IDataBaseInfoSPtr inf;
};

struct SetDefaultDatabaseResponce
  : SetDefaultDatabaseRequest {
  typedef SetDefaultDatabaseRequest base_class;
  explicit SetDefaultDatabaseResponce(const base_class &request);
};

struct ServerInfoRequest
  : public EventInfoBase {
  typedef EventInfoBase base_class;
  explicit ServerInfoRequest(initiator_type sender, error_type er = error_type());
};

class ServerInfoResponce
  : public ServerInfoRequest {
 public:
  typedef ServerInfoRequest base_class;
  explicit ServerInfoResponce(const base_class &request);
  ~ServerInfoResponce();

  ServerInfoSPtr info() const;
  void setInfo(ServerInfoSPtr inf);

 private:
  ServerInfoSPtr info_;
};

struct ServerInfoHistoryRequest
  : public EventInfoBase {
  typedef EventInfoBase base_class;
  explicit ServerInfoHistoryRequest(initiator_type sender, error_type er = error_type());
};

class ServerInfoHistoryResponce
  : public ServerInfoHistoryRequest {
 public:
  typedef ServerInfoHistoryRequest base_class;
  typedef std::vector<ServerInfoSnapShoot> infos_container_type;
  explicit ServerInfoHistoryResponce(const base_class &request);

  infos_container_type infos() const;
  void setInfos(const infos_container_type& inf);

 private:
  infos_container_type infos_;
};

struct ClearServerHistoryRequest
  : public EventInfoBase {
  typedef EventInfoBase base_class;
  explicit ClearServerHistoryRequest(initiator_type sender, error_type er = error_type());
};

struct ClearServerHistoryResponce
  : public ClearServerHistoryRequest {
  typedef ClearServerHistoryRequest base_class;
  explicit ClearServerHistoryResponce(const base_class &request);
};

struct ServerPropertyInfoRequest
  : public EventInfoBase {
  typedef EventInfoBase base_class;
  explicit ServerPropertyInfoRequest(initiator_type sender, error_type er = error_type());
};

struct ServerPropertyInfoResponce
  : ServerPropertyInfoRequest {
  typedef ServerPropertyInfoRequest base_class;
  explicit ServerPropertyInfoResponce(const base_class &request);

  ServerPropertyInfo info;
};

struct ChangeServerPropertyInfoRequest
  : public EventInfoBase {
  typedef EventInfoBase base_class;
  ChangeServerPropertyInfoRequest(initiator_type sender, const PropertyType& pt,
                                  error_type er = error_type());

  PropertyType new_item;
};

struct ChangeServerPropertyInfoResponce
  : ChangeServerPropertyInfoRequest {
  typedef ChangeServerPropertyInfoRequest base_class;
  explicit ChangeServerPropertyInfoResponce(const base_class &request);

  bool is_change;
};

struct CommandRequest
  : public EventInfoBase {
  typedef EventInfoBase base_class;
  CommandRequest(initiator_type sender, IDataBaseInfoSPtr inf, CommandKeySPtr cmd,
                 error_type er = error_type());

  IDataBaseInfoSPtr inf;
  CommandKeySPtr cmd;
};

struct CommandResponce
  : CommandRequest {
  typedef CommandRequest base_class;
  explicit CommandResponce(const base_class &request);
};

struct ProgressInfoResponce {
  explicit ProgressInfoResponce(uint8_t pr);

  const uint8_t progress;
};

}  // namespace events_info
}  // namespace fastonosql
