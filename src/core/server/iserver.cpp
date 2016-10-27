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

#include "core/server/iserver.h"

#include <stddef.h>  // for size_t
#include <string>    // for string, operator==, etc

#include <QApplication>

#include <common/error.h>        // for Error
#include <common/macros.h>       // for VERIFY, CHECK, DNOTREACHED
#include <common/value.h>        // for ErrorValue
#include <common/qt/utils_qt.h>  // for Event<>::value_type
#include <common/qt/logger.h>    // for LOG_ERROR

#include "core/connection_settings/connection_settings.h"
#include "core/events/events_info.h"  // for LoadDatabaseContentResponce, etc
#include "core/driver/idriver.h"      // for IDriver

namespace fastonosql {
namespace core {

IServer::IServer(IDriver* drv) : drv_(drv) {
  VERIFY(QObject::connect(drv_, &IDriver::ChildAdded, this, &IServer::ChildAdded));
  VERIFY(QObject::connect(drv_, &IDriver::ItemUpdated, this, &IServer::ItemUpdated));
  VERIFY(
      QObject::connect(drv_, &IDriver::ServerInfoSnapShoot, this, &IServer::ServerInfoSnapShoot));

  VERIFY(QObject::connect(drv_, &IDriver::KeyRemoved, this, &IServer::KeyRemoved));
  VERIFY(QObject::connect(drv_, &IDriver::KeyAdded, this, &IServer::KeyAdded));
  VERIFY(QObject::connect(drv_, &IDriver::KeyLoaded, this, &IServer::KeyLoaded));
  VERIFY(QObject::connect(drv_, &IDriver::KeyRenamed, this, &IServer::KeyRenamed));
  VERIFY(QObject::connect(drv_, &IDriver::KeyTTLChanged, this, &IServer::KeyTTLChanged));

  drv_->Start();
}

IServer::~IServer() {
  StopCurrentEvent();
  drv_->Stop();
  delete drv_;
}

void IServer::StopCurrentEvent() {
  drv_->Interrupt();
}

bool IServer::IsConnected() const {
  return drv_->IsConnected() && drv_->IsAuthenticated();
}

bool IServer::IsCanRemote() const {
  return IsRemoteType(Type());
}

translator_t IServer::Translator() const {
  return drv_->Translator();
}

connectionTypes IServer::Type() const {
  return drv_->Type();
}

std::string IServer::Name() const {
  connection_path_t path = drv_->ConnectionPath();
  return path.Name();
}

IServerInfoSPtr IServer::CurrentServerInfo() const {
  return drv_->CurrentServerInfo();
}

IDataBaseInfoSPtr IServer::CurrentDatabaseInfo() const {
  return drv_->CurrentDatabaseInfo();
}

std::string IServer::Delimiter() const {
  return drv_->Delimiter();
}

std::string IServer::NsSeparator() const {
  return drv_->NsSeparator();
}

IDatabaseSPtr IServer::CreateDatabaseByInfo(IDataBaseInfoSPtr inf) {
  bool isContains = ContainsDatabase(inf);
  return isContains ? CreateDatabase(inf) : IDatabaseSPtr();
}

bool IServer::ContainsDatabase(IDataBaseInfoSPtr inf) const {
  if (!inf) {
    DNOTREACHED();
    return false;
  }

  CHECK(Type() == inf->Type());

  for (size_t i = 0; i < databases_.size(); ++i) {
    database_t db = databases_[i];
    if (db->Name() == inf->Name()) {
      return true;
    }
  }

  return false;
}

void IServer::Connect(const events_info::ConnectInfoRequest& req) {
  emit ConnectStarted(req);
  QEvent* ev = new events::ConnectRequestEvent(this, req);
  notify(ev);
}

void IServer::Disconnect(const events_info::DisConnectInfoRequest& req) {
  StopCurrentEvent();
  emit DisconnectStarted(req);
  QEvent* ev = new events::DisconnectRequestEvent(this, req);
  notify(ev);
}

void IServer::LoadDatabases(const events_info::LoadDatabasesInfoRequest& req) {
  emit LoadDatabasesStarted(req);
  QEvent* ev = new events::LoadDatabasesInfoRequestEvent(this, req);
  notify(ev);
}

void IServer::LoadDatabaseContent(const events_info::LoadDatabaseContentRequest& req) {
  emit LoadDataBaseContentStarted(req);
  QEvent* ev = new events::LoadDatabaseContentRequestEvent(this, req);
  notify(ev);
}

void IServer::SetDefaultDB(const events_info::SetDefaultDatabaseRequest& req) {
  emit SetDefaultDatabaseStarted(req);
  QEvent* ev = new events::SetDefaultDatabaseRequestEvent(this, req);
  notify(ev);
}

void IServer::ClearDB(const events_info::ClearDatabaseRequest& req) {
  emit ClearDatabaseStarted(req);
  QEvent* ev = new events::ClearDatabaseRequestEvent(this, req);
  notify(ev);
}

void IServer::Execute(const events_info::ExecuteInfoRequest& req) {
  emit ExecuteStarted(req);
  QEvent* ev = new events::ExecuteRequestEvent(this, req);
  notify(ev);
}

void IServer::ShutDown(const events_info::ShutDownInfoRequest& req) {
  emit ShutdownStarted(req);
  QEvent* ev = new events::ShutDownRequestEvent(this, req);
  notify(ev);
}

void IServer::BackupToPath(const events_info::BackupInfoRequest& req) {
  emit BackupStarted(req);
  QEvent* ev = new events::BackupRequestEvent(this, req);
  notify(ev);
}

void IServer::ExportFromPath(const events_info::ExportInfoRequest& req) {
  emit ExportStarted(req);
  QEvent* ev = new events::ExportRequestEvent(this, req);
  notify(ev);
}

void IServer::ChangePassword(const events_info::ChangePasswordRequest& req) {
  emit ChangePasswordStarted(req);
  QEvent* ev = new events::ChangePasswordRequestEvent(this, req);
  notify(ev);
}

void IServer::SetMaxConnection(const events_info::ChangeMaxConnectionRequest& req) {
  emit ChangeMaxConnectionStarted(req);
  QEvent* ev = new events::ChangeMaxConnectionRequestEvent(this, req);
  notify(ev);
}

void IServer::LoadServerInfo(const events_info::ServerInfoRequest& req) {
  emit LoadServerInfoStarted(req);
  QEvent* ev = new events::ServerInfoRequestEvent(this, req);
  notify(ev);
}

void IServer::ServerProperty(const events_info::ServerPropertyInfoRequest& req) {
  emit LoadServerPropertyStarted(req);
  QEvent* ev = new events::ServerPropertyInfoRequestEvent(this, req);
  notify(ev);
}

void IServer::RequestHistoryInfo(const events_info::ServerInfoHistoryRequest& req) {
  emit LoadServerHistoryInfoStarted(req);
  QEvent* ev = new events::ServerInfoHistoryRequestEvent(this, req);
  notify(ev);
}

void IServer::ClearHistory(const events_info::ClearServerHistoryRequest& req) {
  emit ClearServerHistoryStarted(req);
  QEvent* ev = new events::ClearServerHistoryRequestEvent(this, req);
  notify(ev);
}

void IServer::ChangeProperty(const events_info::ChangeServerPropertyInfoRequest& req) {
  emit ChangeServerPropertyStarted(req);
  QEvent* ev = new events::ChangeServerPropertyInfoRequestEvent(this, req);
  notify(ev);
}

void IServer::customEvent(QEvent* event) {
  QEvent::Type type = event->type();
  if (type == static_cast<QEvent::Type>(events::ConnectResponceEvent::EventType)) {
    events::ConnectResponceEvent* ev = static_cast<events::ConnectResponceEvent*>(event);
    HandleConnectEvent(ev);

    events::ConnectResponceEvent::value_type v = ev->value();
    common::Error er(v.errorInfo());
    if (!er) {
      events_info::DiscoveryInfoRequest dreq(this);
      ProcessDiscoveryInfo(dreq);

      events_info::ProcessConfigArgsInfoRequest preq(this);
      ProcessConfigArgs(preq);
    }
  } else if (type == static_cast<QEvent::Type>(events::EnterModeEvent::EventType)) {
    events::EnterModeEvent* ev = static_cast<events::EnterModeEvent*>(event);
    HandleEnterModeEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::LeaveModeEvent::EventType)) {
    events::LeaveModeEvent* ev = static_cast<events::LeaveModeEvent*>(event);
    HandleLeaveModeEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::CommandRootCreatedEvent::EventType)) {
    events::CommandRootCreatedEvent* ev = static_cast<events::CommandRootCreatedEvent*>(event);
    events::CommandRootCreatedEvent::value_type v = ev->value();
    emit RootCreated(v);
  } else if (type == static_cast<QEvent::Type>(events::CommandRootCompleatedEvent::EventType)) {
    events::CommandRootCompleatedEvent* ev =
        static_cast<events::CommandRootCompleatedEvent*>(event);
    events::CommandRootCompleatedEvent::value_type v = ev->value();
    emit RootCompleated(v);
  } else if (type == static_cast<QEvent::Type>(events::DisconnectResponceEvent::EventType)) {
    events::DisconnectResponceEvent* ev = static_cast<events::DisconnectResponceEvent*>(event);
    HandleDisconnectEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::LoadDatabasesInfoResponceEvent::EventType)) {
    events::LoadDatabasesInfoResponceEvent* ev =
        static_cast<events::LoadDatabasesInfoResponceEvent*>(event);
    HandleLoadDatabaseInfosEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::ServerInfoResponceEvent::EventType)) {
    events::ServerInfoResponceEvent* ev = static_cast<events::ServerInfoResponceEvent*>(event);
    HandleLoadServerInfoEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::ServerInfoHistoryResponceEvent::EventType)) {
    events::ServerInfoHistoryResponceEvent* ev =
        static_cast<events::ServerInfoHistoryResponceEvent*>(event);
    HandleLoadServerInfoHistoryEvent(ev);
  } else if (type ==
             static_cast<QEvent::Type>(events::ClearServerHistoryResponceEvent::EventType)) {
    events::ClearServerHistoryResponceEvent* ev =
        static_cast<events::ClearServerHistoryResponceEvent*>(event);
    HandleClearServerHistoryResponceEvent(ev);
  } else if (type ==
             static_cast<QEvent::Type>(events::ServerPropertyInfoResponceEvent::EventType)) {
    events::ServerPropertyInfoResponceEvent* ev =
        static_cast<events::ServerPropertyInfoResponceEvent*>(event);
    HandleLoadServerPropertyEvent(ev);
  } else if (type ==
             static_cast<QEvent::Type>(events::ChangeServerPropertyInfoResponceEvent::EventType)) {
    events::ChangeServerPropertyInfoResponceEvent* ev =
        static_cast<events::ChangeServerPropertyInfoResponceEvent*>(event);
    HandleServerPropertyChangeEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::BackupResponceEvent::EventType)) {
    events::BackupResponceEvent* ev = static_cast<events::BackupResponceEvent*>(event);
    HandleBackupEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::ExportResponceEvent::EventType)) {
    events::ExportResponceEvent* ev = static_cast<events::ExportResponceEvent*>(event);
    HandleExportEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::ChangePasswordResponceEvent::EventType)) {
    events::ChangePasswordResponceEvent* ev =
        static_cast<events::ChangePasswordResponceEvent*>(event);
    HandleChangePasswordEvent(ev);
  } else if (type ==
             static_cast<QEvent::Type>(events::ChangeMaxConnectionResponceEvent::EventType)) {
    events::ChangeMaxConnectionResponceEvent* ev =
        static_cast<events::ChangeMaxConnectionResponceEvent*>(event);
    HandleChangeMaxConnectionEvent(ev);
  } else if (type ==
             static_cast<QEvent::Type>(events::LoadDatabaseContentResponceEvent::EventType)) {
    events::LoadDatabaseContentResponceEvent* ev =
        static_cast<events::LoadDatabaseContentResponceEvent*>(event);
    HandleLoadDatabaseContentEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::ClearDatabaseResponceEvent::EventType)) {
    events::ClearDatabaseResponceEvent* ev =
        static_cast<events::ClearDatabaseResponceEvent*>(event);
    HandleClearDatabaseEvent(ev);
  } else if (type ==
             static_cast<QEvent::Type>(events::SetDefaultDatabaseResponceEvent::EventType)) {
    events::SetDefaultDatabaseResponceEvent* ev =
        static_cast<events::SetDefaultDatabaseResponceEvent*>(event);
    HandleSetDefaultDatabaseEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::ExecuteResponceEvent::EventType)) {
    events::ExecuteResponceEvent* ev = static_cast<events::ExecuteResponceEvent*>(event);
    HandleExecuteEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::DiscoveryInfoResponceEvent::EventType)) {
    events::DiscoveryInfoResponceEvent* ev =
        static_cast<events::DiscoveryInfoResponceEvent*>(event);
    HandleDiscoveryInfoResponceEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::ProgressResponceEvent::EventType)) {
    events::ProgressResponceEvent* ev = static_cast<events::ProgressResponceEvent*>(event);
    events::ProgressResponceEvent::value_type v = ev->value();
    emit ProgressChanged(v);
  }

  return QObject::customEvent(event);
}

void IServer::notify(QEvent* ev) {
  events_info::ProgressInfoResponce resp(0);
  emit ProgressChanged(resp);
  qApp->postEvent(drv_, ev);
}

void IServer::HandleConnectEvent(events::ConnectResponceEvent* ev) {
  auto v = ev->value();
  common::Error er(v.errorInfo());
  if (er && er->isError()) {
    LOG_ERROR(er, true);
  }
  emit ConnectFinished(v);
}

void IServer::HandleDisconnectEvent(events::DisconnectResponceEvent* ev) {
  auto v = ev->value();
  common::Error er(v.errorInfo());
  if (er && er->isError()) {
    LOG_ERROR(er, true);
  }
  emit DisconnectFinished(v);
}

void IServer::HandleLoadServerInfoEvent(events::ServerInfoResponceEvent* ev) {
  auto v = ev->value();
  common::Error er(v.errorInfo());
  if (er && er->isError()) {
    LOG_ERROR(er, true);
  }
  emit LoadServerInfoFinished(v);
}

void IServer::HandleLoadServerPropertyEvent(events::ServerPropertyInfoResponceEvent* ev) {
  auto v = ev->value();
  common::Error er(v.errorInfo());
  if (er && er->isError()) {
    LOG_ERROR(er, true);
  }
  emit LoadServerPropertyFinished(v);
}

void IServer::HandleServerPropertyChangeEvent(events::ChangeServerPropertyInfoResponceEvent* ev) {
  auto v = ev->value();
  common::Error er(v.errorInfo());
  if (er && er->isError()) {
    LOG_ERROR(er, true);
  }
  emit ChangeServerPropertyFinished(v);
}

void IServer::HandleShutdownEvent(events::ShutDownResponceEvent* ev) {
  auto v = ev->value();
  common::Error er(v.errorInfo());
  if (er && er->isError()) {
    LOG_ERROR(er, true);
  }
  emit ShutdownFinished(v);
}

void IServer::HandleBackupEvent(events::BackupResponceEvent* ev) {
  auto v = ev->value();
  common::Error er(v.errorInfo());
  if (er && er->isError()) {
    LOG_ERROR(er, true);
  }
  emit BackupFinished(v);
}

void IServer::HandleExportEvent(events::ExportResponceEvent* ev) {
  auto v = ev->value();
  common::Error er(v.errorInfo());
  if (er && er->isError()) {
    LOG_ERROR(er, true);
  }
  emit ExportFinished(v);
}

void IServer::HandleChangePasswordEvent(events::ChangePasswordResponceEvent* ev) {
  auto v = ev->value();
  common::Error er(v.errorInfo());
  if (er && er->isError()) {
    LOG_ERROR(er, true);
  }

  emit ChangePasswordFinished(v);
}

void IServer::HandleChangeMaxConnectionEvent(events::ChangeMaxConnectionResponceEvent* ev) {
  auto v = ev->value();
  common::Error er(v.errorInfo());
  if (er && er->isError()) {
    LOG_ERROR(er, true);
  }

  emit ChangeMaxConnectionFinished(v);
}

void IServer::HandleExecuteEvent(events::ExecuteResponceEvent* ev) {
  auto v = ev->value();
  common::Error er(v.errorInfo());
  if (er && er->isError()) {
    LOG_ERROR(er, true);
  }

  emit ExecuteFinished(v);
}

void IServer::HandleLoadDatabaseInfosEvent(events::LoadDatabasesInfoResponceEvent* ev) {
  auto v = ev->value();
  common::Error er(v.errorInfo());
  if (er && er->isError()) {
    LOG_ERROR(er, true);
    databases_.clear();
  } else {
    events_info::LoadDatabasesInfoResponce::database_info_cont_type dbs = v.databases;
    events_info::LoadDatabasesInfoResponce::database_info_cont_type tmp;
    for (size_t j = 0; j < dbs.size(); ++j) {
      IDataBaseInfoSPtr db = dbs[j];
      if (!ContainsDatabase(db)) {
        databases_.push_back(db);
      }
      tmp.push_back(db);
    }
    v.databases = tmp;
  }

  emit LoadDatabasesFinished(v);
}

void IServer::HandleLoadDatabaseContentEvent(events::LoadDatabaseContentResponceEvent* ev) {
  auto v = ev->value();
  common::Error er(v.errorInfo());
  if (er && er->isError()) {
    LOG_ERROR(er, true);
  } else {
    if (ContainsDatabase(v.inf)) {
      v.inf->SetKeys(v.keys);
      v.inf->SetDBKeysCount(v.db_keys_count);
    }
  }

  emit LoadDatabaseContentFinished(v);
}

void IServer::HandleClearDatabaseEvent(events::ClearDatabaseResponceEvent* ev) {
  auto v = ev->value();
  common::Error er = v.errorInfo();
  if (er && er->isError()) {
    LOG_ERROR(er, true);
  } else {
    if (ContainsDatabase(v.inf)) {
      v.inf->ClearKeys();
    }
  }

  emit ClearDatabaseFinished(v);
}

void IServer::HandleEnterModeEvent(events::EnterModeEvent* ev) {
  auto v = ev->value();
  common::Error er(v.errorInfo());
  if (er && er->isError()) {
    LOG_ERROR(er, true);
  }

  emit ModeEntered(v);
}

void IServer::HandleLeaveModeEvent(events::LeaveModeEvent* ev) {
  auto v = ev->value();
  common::Error er(v.errorInfo());
  if (er && er->isError()) {
    LOG_ERROR(er, true);
  }

  emit ModeLeaved(v);
}

void IServer::HandleLoadServerInfoHistoryEvent(events::ServerInfoHistoryResponceEvent* ev) {
  auto v = ev->value();
  common::Error er = v.errorInfo();
  if (er && er->isError()) {
    LOG_ERROR(er, true);
  }

  emit LoadServerHistoryInfoFinished(v);
}

void IServer::HandleDiscoveryInfoResponceEvent(events::DiscoveryInfoResponceEvent* ev) {
  auto v = ev->value();
  common::Error er = v.errorInfo();
  if (er && er->isError()) {
    LOG_ERROR(er, true);
  }
  emit LoadDiscoveryInfoFinished(v);
}

void IServer::HandleClearServerHistoryResponceEvent(events::ClearServerHistoryResponceEvent* ev) {
  auto v = ev->value();
  common::Error er = v.errorInfo();
  if (er && er->isError()) {
    LOG_ERROR(er, true);
  }

  emit ClearServerHistoryFinished(v);
}

void IServer::HandleSetDefaultDatabaseEvent(events::SetDefaultDatabaseResponceEvent* ev) {
  auto v = ev->value();
  common::Error er(v.errorInfo());
  if (er && er->isError()) {
    LOG_ERROR(er, true);
  } else {
    IDataBaseInfoSPtr inf = v.inf;
    for (size_t i = 0; i < databases_.size(); ++i) {
      databases_[i]->SetIsDefault(false);
    }
    inf->SetIsDefault(true);
  }

  emit SetDefaultDatabaseFinished(v);
}

void IServer::ProcessConfigArgs(const events_info::ProcessConfigArgsInfoRequest& req) {
  QEvent* ev = new events::ProcessConfigArgsRequestEvent(this, req);
  notify(ev);
}

void IServer::ProcessDiscoveryInfo(const events_info::DiscoveryInfoRequest& req) {
  emit LoadDiscoveryInfoStarted(req);
  QEvent* ev = new events::DiscoveryInfoRequestEvent(this, req);
  notify(ev);
}

}  // namespace core
}  // namespace fastonosql
