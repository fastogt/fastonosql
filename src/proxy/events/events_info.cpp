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

#include "proxy/events/events_info.h"

#include <common/time.h>  // for current_mstime

namespace fastonosql {
namespace proxy {
namespace events_info {

EventInfoBase::EventInfoBase(initiator_type sender, error_type er)
    : base_class(sender, er), time_start_(common::time::current_mstime()) {}

EventInfoBase::EventInfoBase(initiator_type sender, common::time64_t time_start, error_type er)
    : base_class(sender, er), time_start_(time_start) {}

common::time64_t EventInfoBase::ElapsedTime() const {
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

DiscoveryInfoRequest::DiscoveryInfoRequest(initiator_type sender, error_type er)
    : base_class(sender, er) {}

DiscoveryInfoResponce::DiscoveryInfoResponce(const base_class& request) : base_class(request) {}

EnterModeInfo::EnterModeInfo(initiator_type sender, core::ConnectionMode mode, error_type er)
    : base_class(sender, er), mode(mode) {}

LeaveModeInfo::LeaveModeInfo(initiator_type sender, core::ConnectionMode mode, error_type er)
    : base_class(sender, er), mode(mode) {}

CommandRootCreatedInfo::CommandRootCreatedInfo(initiator_type sender,
                                               core::FastoObjectIPtr root,
                                               error_type er)
    : base_class(sender, er), root(root) {}

CommandRootCompleatedInfo::CommandRootCompleatedInfo(initiator_type sender,
                                                     core::FastoObjectIPtr root,
                                                     error_type er)
    : base_class(sender, er), root(root) {}

CommandRootCompleatedInfo::CommandRootCompleatedInfo(initiator_type sender,
                                                     common::time64_t timest,
                                                     core::FastoObjectIPtr root,
                                                     error_type er)
    : base_class(sender, timest, er), root(root) {}

DisConnectInfoRequest::DisConnectInfoRequest(initiator_type sender, error_type er)
    : base_class(sender, er) {}

DisConnectInfoResponce::DisConnectInfoResponce(const base_class& request) : base_class(request) {}

ExecuteInfoRequest::ExecuteInfoRequest(initiator_type sender,
                                       const std::string& text,
                                       size_t repeat,
                                       common::time64_t msec_repeat_interval,
                                       bool history,
                                       bool silence,
                                       core::CmdLoggingType logtype,
                                       error_type er)
    : base_class(sender, er),
      text(text),
      repeat(repeat),
      msec_repeat_interval(msec_repeat_interval),
      history(history),
      silence(silence),
      logtype(logtype) {}

ExecuteInfoResponce::ExecuteInfoResponce(const base_class& request) : base_class(request) {}

LoadDatabasesInfoRequest::LoadDatabasesInfoRequest(initiator_type sender, error_type er)
    : base_class(sender, er) {}

LoadDatabasesInfoResponce::LoadDatabasesInfoResponce(const base_class& request)
    : base_class(request) {}

LoadDatabaseContentRequest::LoadDatabaseContentRequest(initiator_type sender,
                                                       core::IDataBaseInfoSPtr inf,
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

LoadServerChannelsRequest::LoadServerChannelsRequest(initiator_type sender,
                                                     const std::string& pattern,
                                                     error_type er)
    : base_class(sender, er), pattern(pattern) {}

LoadServerChannelsResponce::LoadServerChannelsResponce(const base_class& request)
    : base_class(request), channels() {}

ServerInfoRequest::ServerInfoRequest(initiator_type sender, error_type er)
    : base_class(sender, er) {}

ServerInfoResponce::ServerInfoResponce(const base_class& request) : base_class(request), info_() {}

core::IServerInfoSPtr ServerInfoResponce::info() const {
  return info_;
}

void ServerInfoResponce::setInfo(core::IServerInfoSPtr inf) {
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
                                                                 const core::property_t& pt,
                                                                 error_type er)
    : base_class(sender, er), new_item(pt) {}

ChangeServerPropertyInfoResponce::ChangeServerPropertyInfoResponce(const base_class& request)
    : base_class(request) {}

ProgressInfoResponce::ProgressInfoResponce(int pr) : progress(pr) {}

}  // namespace events_info
}  // namespace proxy
}  // namespace fastonosql
