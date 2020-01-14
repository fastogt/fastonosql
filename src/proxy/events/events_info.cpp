/*  Copyright (C) 2014-2020 FastoGT. All right reserved.

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

#include "proxy/events/events_info.h"

#include <common/time.h>

namespace fastonosql {
namespace proxy {
namespace events_info {

EventInfoBase::EventInfoBase(initiator_type sender, error_type er)
    : base_class(sender, er), time_start_(common::time::current_utc_mstime()) {}

EventInfoBase::EventInfoBase(initiator_type sender, common::time64_t time_start, error_type er)
    : base_class(sender, er), time_start_(time_start) {}

common::time64_t EventInfoBase::GetElapsedTime() const {
  return common::time::current_utc_mstime() - time_start_;
}

ConnectInfoRequest::ConnectInfoRequest(initiator_type sender, error_type er) : base_class(sender, er) {}

ConnectInfoResponse::ConnectInfoResponse(const base_class& request) : base_class(request) {}

BackupInfoRequest::BackupInfoRequest(initiator_type sender, const std::string& path, error_type er)
    : base_class(sender, er), path(path) {}

BackupInfoResponse::BackupInfoResponse(const base_class& request) : base_class(request) {}

RestoreInfoRequest::RestoreInfoRequest(initiator_type sender, const std::string& path, error_type er)
    : base_class(sender, er), path(path) {}

RestoreInfoResponse::RestoreInfoResponse(const base_class& request) : base_class(request) {}

DiscoveryInfoRequest::DiscoveryInfoRequest(initiator_type sender, error_type er) : base_class(sender, er) {}

DiscoveryInfoResponse::DiscoveryInfoResponse(const base_class& request) : base_class(request) {}

EnterModeInfo::EnterModeInfo(initiator_type sender, core::ConnectionMode mode, error_type er)
    : base_class(sender, er), mode(mode) {}

LeaveModeInfo::LeaveModeInfo(initiator_type sender, core::ConnectionMode mode, error_type er)
    : base_class(sender, er), mode(mode) {}

CommandRootCreatedInfo::CommandRootCreatedInfo(initiator_type sender, core::FastoObjectIPtr root, error_type er)
    : base_class(sender, er), root(root) {}

CommandRootCompleatedInfo::CommandRootCompleatedInfo(initiator_type sender, core::FastoObjectIPtr root, error_type er)
    : base_class(sender, er), root(root) {}

CommandRootCompleatedInfo::CommandRootCompleatedInfo(initiator_type sender,
                                                     common::time64_t timest,
                                                     core::FastoObjectIPtr root,
                                                     error_type er)
    : base_class(sender, timest, er), root(root) {}

DisConnectInfoRequest::DisConnectInfoRequest(initiator_type sender, error_type er) : base_class(sender, er) {}

DisConnectInfoResponse::DisConnectInfoResponse(const base_class& request) : base_class(request) {}

ExecuteInfoRequest::ExecuteInfoRequest(initiator_type sender,
                                       const core::command_buffer_t& text,
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

ExecuteInfoResponse::ExecuteInfoResponse(const base_class& request) : base_class(request) {}

LoadDatabasesInfoRequest::LoadDatabasesInfoRequest(initiator_type sender, error_type er) : base_class(sender, er) {}

LoadDatabasesInfoResponse::LoadDatabasesInfoResponse(const base_class& request) : base_class(request) {}

LoadDatabaseContentRequest::LoadDatabaseContentRequest(initiator_type sender,
                                                       core::IDataBaseInfoSPtr inf,
                                                       const core::pattern_t& pattern,
                                                       core::keys_limit_t keys_count,
                                                       core::cursor_t cursor,
                                                       error_type er)
    : base_class(sender, er), inf(inf), pattern(pattern), keys_count(keys_count), cursor_in(cursor) {}

LoadDatabaseContentResponse::LoadDatabaseContentResponse(const base_class& request)
    : base_class(request), keys(), cursor_out(0), db_keys_count(0) {}

LoadServerChannelsRequest::LoadServerChannelsRequest(initiator_type sender, const std::string& pattern, error_type er)
    : base_class(sender, er), pattern(pattern) {}

LoadServerChannelsResponse::LoadServerChannelsResponse(const base_class& request) : base_class(request), channels() {}

LoadServerClientsRequest::LoadServerClientsRequest(initiator_type sender, error_type er) : base_class(sender, er) {}

LoadServerClientsResponse::LoadServerClientsResponse(const base_class& request) : base_class(request), clients() {}

ServerInfoRequest::ServerInfoRequest(initiator_type sender, error_type er) : base_class(sender, er) {}

ServerInfoResponse::ServerInfoResponse(const base_class& request) : base_class(request), info_() {}

core::IServerInfoSPtr ServerInfoResponse::GetInfo() const {
  return info_;
}

void ServerInfoResponse::SetInfo(core::IServerInfoSPtr inf) {
  info_ = inf;
}

ServerInfoHistoryRequest::ServerInfoHistoryRequest(initiator_type sender, error_type er) : base_class(sender, er) {}

ServerInfoHistoryResponse::ServerInfoHistoryResponse(const base_class& request) : base_class(request) {}

ServerInfoHistoryResponse::infos_container_type ServerInfoHistoryResponse::GetInfos() const {
  return infos_;
}

void ServerInfoHistoryResponse::SetInfos(const infos_container_type& inf) {
  infos_ = inf;
}

ClearServerHistoryRequest::ClearServerHistoryRequest(initiator_type sender, error_type er) : base_class(sender, er) {}

ClearServerHistoryResponse::ClearServerHistoryResponse(const base_class& request) : base_class(request) {}

ServerPropertyInfoRequest::ServerPropertyInfoRequest(initiator_type sender, error_type er) : base_class(sender, er) {}

ServerPropertyInfoResponse::ServerPropertyInfoResponse(const base_class& request) : base_class(request) {}

ChangeServerPropertyInfoRequest::ChangeServerPropertyInfoRequest(initiator_type sender,
                                                                 const core::property_t& pt,
                                                                 error_type er)
    : base_class(sender, er), new_item(pt) {}

ChangeServerPropertyInfoResponse::ChangeServerPropertyInfoResponse(const base_class& request)
    : base_class(request), is_change(false) {}

ProgressInfoResponse::ProgressInfoResponse(int pr) : progress(pr) {}

}  // namespace events_info
}  // namespace proxy
}  // namespace fastonosql
