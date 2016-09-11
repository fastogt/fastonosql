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

#include "core/events/events_info.h"

namespace fastonosql {
namespace core {
namespace events {

typedef common::qt::Event<events_info::ConnectInfoRequest, QEvent::User + 1> ConnectRequestEvent;
typedef common::qt::Event<events_info::ConnectInfoResponce, QEvent::User + 2> ConnectResponceEvent;

typedef common::qt::Event<events_info::ShutDownInfoRequest, QEvent::User + 3> ShutDownRequestEvent;
typedef common::qt::Event<events_info::ShutDownInfoResponce, QEvent::User + 4>
    ShutDownResponceEvent;

typedef common::qt::Event<events_info::ProcessConfigArgsInfoRequest, QEvent::User + 5>
    ProcessConfigArgsRequestEvent;
typedef common::qt::Event<events_info::ProcessConfigArgsInfoResponce, QEvent::User + 6>
    ProcessConfigArgsResponceEvent;

typedef common::qt::Event<events_info::CommandRootCreatedInfo, QEvent::User + 7>
    CommandRootCreatedEvent;
typedef common::qt::Event<events_info::CommandRootCompleatedInfo, QEvent::User + 8>
    CommandRootCompleatedEvent;

typedef common::qt::Event<events_info::EnterModeInfo, QEvent::User + 9> EnterModeEvent;
typedef common::qt::Event<events_info::LeaveModeInfo, QEvent::User + 10> LeaveModeEvent;

typedef common::qt::Event<events_info::DisConnectInfoRequest, QEvent::User + 11>
    DisconnectRequestEvent;
typedef common::qt::Event<events_info::DisConnectInfoResponce, QEvent::User + 12>
    DisconnectResponceEvent;

typedef common::qt::Event<events_info::ExecuteInfoRequest, QEvent::User + 13> ExecuteRequestEvent;
typedef common::qt::Event<events_info::ExecuteInfoResponce, QEvent::User + 14> ExecuteResponceEvent;

typedef common::qt::Event<events_info::LoadDatabasesInfoRequest, QEvent::User + 15>
    LoadDatabasesInfoRequestEvent;
typedef common::qt::Event<events_info::LoadDatabasesInfoResponce, QEvent::User + 16>
    LoadDatabasesInfoResponceEvent;

typedef common::qt::Event<events_info::ServerInfoRequest, QEvent::User + 17> ServerInfoRequestEvent;
typedef common::qt::Event<events_info::ServerInfoResponce, QEvent::User + 18>
    ServerInfoResponceEvent;

typedef common::qt::Event<events_info::ServerInfoHistoryRequest, QEvent::User + 19>
    ServerInfoHistoryRequestEvent;
typedef common::qt::Event<events_info::ServerInfoHistoryResponce, QEvent::User + 20>
    ServerInfoHistoryResponceEvent;

typedef common::qt::Event<events_info::ClearServerHistoryRequest, QEvent::User + 21>
    ClearServerHistoryRequestEvent;
typedef common::qt::Event<events_info::ClearServerHistoryResponce, QEvent::User + 22>
    ClearServerHistoryResponceEvent;

typedef common::qt::Event<events_info::ServerPropertyInfoRequest, QEvent::User + 23>
    ServerPropertyInfoRequestEvent;
typedef common::qt::Event<events_info::ServerPropertyInfoResponce, QEvent::User + 24>
    ServerPropertyInfoResponceEvent;

typedef common::qt::Event<events_info::ChangeServerPropertyInfoRequest, QEvent::User + 25>
    ChangeServerPropertyInfoRequestEvent;
typedef common::qt::Event<events_info::ChangeServerPropertyInfoResponce, QEvent::User + 26>
    ChangeServerPropertyInfoResponceEvent;

typedef common::qt::Event<events_info::BackupInfoRequest, QEvent::User + 27> BackupRequestEvent;
typedef common::qt::Event<events_info::BackupInfoResponce, QEvent::User + 28> BackupResponceEvent;

typedef common::qt::Event<events_info::ExportInfoRequest, QEvent::User + 29> ExportRequestEvent;
typedef common::qt::Event<events_info::ExportInfoResponce, QEvent::User + 30> ExportResponceEvent;

typedef common::qt::Event<events_info::LoadDatabaseContentRequest, QEvent::User + 31>
    LoadDatabaseContentRequestEvent;
typedef common::qt::Event<events_info::LoadDatabaseContentResponce, QEvent::User + 32>
    LoadDatabaseContentResponceEvent;

typedef common::qt::Event<events_info::ClearDatabaseRequest, QEvent::User + 33>
    ClearDatabaseRequestEvent;
typedef common::qt::Event<events_info::ClearDatabaseResponce, QEvent::User + 34>
    ClearDatabaseResponceEvent;

typedef common::qt::Event<events_info::SetDefaultDatabaseRequest, QEvent::User + 35>
    SetDefaultDatabaseRequestEvent;
typedef common::qt::Event<events_info::SetDefaultDatabaseResponce, QEvent::User + 36>
    SetDefaultDatabaseResponceEvent;

typedef common::qt::Event<events_info::CommandRequest, QEvent::User + 37> CommandRequestEvent;
typedef common::qt::Event<events_info::CommandResponce, QEvent::User + 38> CommandResponceEvent;

typedef common::qt::Event<events_info::DiscoveryInfoRequest, QEvent::User + 39>
    DiscoveryInfoRequestEvent;
typedef common::qt::Event<events_info::DiscoveryInfoResponce, QEvent::User + 40>
    DiscoveryInfoResponceEvent;

typedef common::qt::Event<events_info::ChangePasswordRequest, QEvent::User + 41>
    ChangePasswordRequestEvent;
typedef common::qt::Event<events_info::ChangePasswordResponce, QEvent::User + 42>
    ChangePasswordResponceEvent;

typedef common::qt::Event<events_info::ChangeMaxConnectionRequest, QEvent::User + 43>
    ChangeMaxConnectionRequestEvent;
typedef common::qt::Event<events_info::ChangeMaxConnectionResponce, QEvent::User + 44>
    ChangeMaxConnectionResponceEvent;

typedef common::qt::Event<events_info::ProgressInfoResponce, QEvent::User + 100>
    ProgressResponceEvent;

}  // namespace events
}  // namespace core
}  // namespace fastonosql
