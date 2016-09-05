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

#include "core/events/events_info.h"

#include <string>
#include <vector>

#include "common/time.h"

namespace fastonosql {
namespace core {
namespace events_info {

EventInfoBase::EventInfoBase(initiator_type sender, error_type er)
    : base_class(sender, er), time_start_(common::time::current_mstime()) {}

EventInfoBase::EventInfoBase(initiator_type sender, common::time64_t time_start, error_type er)
    : base_class(sender, er), time_start_(time_start) {}

common::time64_t EventInfoBase::elapsedTime() const {
  return common::time::current_mstime() - time_start_;
}

ConnectInfoRequest::ConnectInfoRequest(initiator_type sender, error_type er)
    : base_class(sender, er) {}

ConnectInfoResponce::ConnectInfoResponce(const base_class& request) : base_class(request) {}

ShutDownInfoRequest::ShutDownInfoRequest(initiator_type sender, error_type er)
    : base_class(sender, er) {}

ShutDownInfoResponce::ShutDownInfoResponce(const base_class& request) : base_class(request) {}

BackupInfoRequest::BackupInfoRequest(initiator_type sender, const std::string& path, error_type er)
    : base_class(sender, er), path(path) {}

BackupInfoResponce::BackupInfoResponce(const base_class& request) : base_class(request) {}

ExportInfoRequest::ExportInfoRequest(initiator_type sender, const std::string& path, error_type er)
    : base_class(sender, er), path(path) {}

ExportInfoResponce::ExportInfoResponce(const base_class& request) : base_class(request) {}

ChangePasswordRequest::ChangePasswordRequest(initiator_type sender,
                                             const std::string& oldPassword,
                                             const std::string& newPassword,
                                             error_type er)
    : base_class(sender, er), old_password(oldPassword), new_password(newPassword) {}

ChangePasswordResponce::ChangePasswordResponce(const base_class& request) : base_class(request) {}

ChangeMaxConnectionRequest::ChangeMaxConnectionRequest(initiator_type sender,
                                                       int maxConnection,
                                                       error_type er)
    : base_class(sender, er), max_connection(maxConnection) {}

ChangeMaxConnectionResponce::ChangeMaxConnectionResponce(const base_class& request)
    : base_class(request) {}

ProcessConfigArgsInfoRequest::ProcessConfigArgsInfoRequest(initiator_type sender, error_type er)
    : base_class(sender, er) {}

ProcessConfigArgsInfoResponce::ProcessConfigArgsInfoResponce(const base_class& request)
    : base_class(request) {}

DiscoveryInfoRequest::DiscoveryInfoRequest(initiator_type sender, error_type er)
    : base_class(sender, er) {}

DiscoveryInfoResponce::DiscoveryInfoResponce(const base_class& request) : base_class(request) {}

EnterModeInfo::EnterModeInfo(initiator_type sender, ConnectionMode mode, error_type er)
    : base_class(sender, er), mode(mode) {}

LeaveModeInfo::LeaveModeInfo(initiator_type sender, ConnectionMode mode, error_type er)
    : base_class(sender, er), mode(mode) {}

CommandRootCreatedInfo::CommandRootCreatedInfo(initiator_type sender,
                                               FastoObjectIPtr root,
                                               error_type er)
    : base_class(sender, er), root(root) {}

CommandRootCompleatedInfo::CommandRootCompleatedInfo(initiator_type sender,
                                                     FastoObjectIPtr root,
                                                     error_type er)
    : base_class(sender, er), root(root) {}

CommandRootCompleatedInfo::CommandRootCompleatedInfo(initiator_type sender,
                                                     common::time64_t timest,
                                                     FastoObjectIPtr root,
                                                     error_type er)
    : base_class(sender, timest, er), root(root) {}

DisConnectInfoRequest::DisConnectInfoRequest(initiator_type sender, error_type er)
    : base_class(sender, er) {}

DisConnectInfoResponce::DisConnectInfoResponce(const base_class& request) : base_class(request) {}

ExecuteInfoRequest::ExecuteInfoRequest(initiator_type sender,
                                       const std::string& text,
                                       const std::vector<std::string>& args,
                                       error_type er)
    : base_class(sender, er), text(text), args(args) {}

ExecuteInfoResponce::ExecuteInfoResponce(const base_class& request) : base_class(request) {}

LoadDatabasesInfoRequest::LoadDatabasesInfoRequest(initiator_type sender, error_type er)
    : base_class(sender, er) {}

LoadDatabasesInfoResponce::LoadDatabasesInfoResponce(const base_class& request)
    : base_class(request) {}

LoadDatabaseContentRequest::LoadDatabaseContentRequest(initiator_type sender,
                                                       IDataBaseInfoSPtr inf,
                                                       const std::string& pattern,
                                                       uint32_t countKeys,
                                                       uint32_t cursor,
                                                       error_type er)
    : base_class(sender, er),
      inf(inf),
      pattern(pattern),
      count_keys(countKeys),
      cursor_in(cursor) {}

LoadDatabaseContentResponce::LoadDatabaseContentResponce(const base_class& request)
    : base_class(request), keys(), cursor_out(0), db_keys_count(0) {}

ClearDatabaseRequest::ClearDatabaseRequest(initiator_type sender,
                                           IDataBaseInfoSPtr inf,
                                           error_type er)
    : base_class(sender, er), inf(inf) {}

ClearDatabaseResponce::ClearDatabaseResponce(const base_class& request) : base_class(request) {}

SetDefaultDatabaseRequest::SetDefaultDatabaseRequest(initiator_type sender,
                                                     IDataBaseInfoSPtr inf,
                                                     error_type er)
    : base_class(sender, er), inf(inf) {}

SetDefaultDatabaseResponce::SetDefaultDatabaseResponce(const base_class& request)
    : base_class(request) {}

ServerInfoRequest::ServerInfoRequest(initiator_type sender, error_type er)
    : base_class(sender, er) {}

ServerInfoResponce::ServerInfoResponce(const base_class& request) : base_class(request), info_() {}

IServerInfoSPtr ServerInfoResponce::info() const {
  return info_;
}

void ServerInfoResponce::setInfo(IServerInfoSPtr inf) {
  info_ = inf;
}

ServerInfoResponce::~ServerInfoResponce() {}

ServerInfoHistoryRequest::ServerInfoHistoryRequest(initiator_type sender, error_type er)
    : base_class(sender, er) {}

ServerInfoHistoryResponce::ServerInfoHistoryResponce(const base_class& request)
    : base_class(request) {}

ServerInfoHistoryResponce::infos_container_type ServerInfoHistoryResponce::infos() const {
  return infos_;
}

void ServerInfoHistoryResponce::setInfos(const infos_container_type& inf) {
  infos_ = inf;
}

ClearServerHistoryRequest::ClearServerHistoryRequest(initiator_type sender, error_type er)
    : base_class(sender, er) {}

ClearServerHistoryResponce::ClearServerHistoryResponce(const base_class& request)
    : base_class(request) {}

ServerPropertyInfoRequest::ServerPropertyInfoRequest(initiator_type sender, error_type er)
    : base_class(sender, er) {}

ServerPropertyInfoResponce::ServerPropertyInfoResponce(const base_class& request)
    : base_class(request) {}

ChangeServerPropertyInfoRequest::ChangeServerPropertyInfoRequest(initiator_type sender,
                                                                 const property_t& pt,
                                                                 error_type er)
    : base_class(sender, er), new_item(pt) {}

ChangeServerPropertyInfoResponce::ChangeServerPropertyInfoResponce(const base_class& request)
    : base_class(request) {}

CommandRequest::CommandRequest(initiator_type sender,
                               IDataBaseInfoSPtr inf,
                               CommandKeySPtr cmd,
                               error_type er)
    : base_class(sender, er), inf(inf), cmd(cmd) {}

CommandResponce::CommandResponce(const base_class& request) : base_class(request) {}

ProgressInfoResponce::ProgressInfoResponce(uint8_t pr) : progress(pr) {}

}  // namespace events_info
}  // namespace core
}  // namespace fastonosql
