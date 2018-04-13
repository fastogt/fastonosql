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
    along with FastoNoSQL.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "proxy/server/iserver.h"

#include <QApplication>

#include <common/qt/logger.h>  // for LOG_ERROR
#include <common/sprintf.h>

#include "proxy/driver/idriver.h"  // for IDriver

namespace fastonosql {
namespace proxy {

IServer::IServer(IDriver* drv) : drv_(drv), server_info_(), current_database_info_(), timer_check_key_exists_id_(0) {
  VERIFY(QObject::connect(drv_, &IDriver::ChildAdded, this, &IServer::ChildAdded));
  VERIFY(QObject::connect(drv_, &IDriver::ItemUpdated, this, &IServer::ItemUpdated));
  VERIFY(QObject::connect(drv_, &IDriver::ServerInfoSnapShooted, this, &IServer::ServerInfoSnapShooted));

  VERIFY(QObject::connect(drv_, &IDriver::DBCreated, this, &IServer::CreateDatabase));
  VERIFY(QObject::connect(drv_, &IDriver::DBRemoved, this, &IServer::RemoveDatabase));
  VERIFY(QObject::connect(drv_, &IDriver::DBFlushed, this, &IServer::FlushCurrentDatabase));
  VERIFY(QObject::connect(drv_, &IDriver::DBChanged, this, &IServer::ChangeCurrentDatabase));

  VERIFY(QObject::connect(drv_, &IDriver::KeyRemoved, this, &IServer::RemoveKey));
  VERIFY(QObject::connect(drv_, &IDriver::KeyAdded, this, &IServer::AddKey));
  VERIFY(QObject::connect(drv_, &IDriver::KeyLoaded, this, &IServer::LoadKey));
  VERIFY(QObject::connect(drv_, &IDriver::KeyRenamed, this, &IServer::RenameKey));
  VERIFY(QObject::connect(drv_, &IDriver::KeyTTLChanged, this, &IServer::ChangeKeyTTL));
  VERIFY(QObject::connect(drv_, &IDriver::KeyTTLLoaded, this, &IServer::LoadKeyTTL));
  VERIFY(QObject::connect(drv_, &IDriver::ModuleLoaded, this, &IServer::LoadModule));
  VERIFY(QObject::connect(drv_, &IDriver::ModuleUnLoaded, this, &IServer::UnLoadModule));
  VERIFY(QObject::connect(drv_, &IDriver::Disconnected, this, &IServer::Disconnected));

  drv_->Start();
}

IServer::~IServer() {
  StopCurrentEvent();
  drv_->Stop();
  delete drv_;
}

void IServer::StartCheckKeyExistTimer() {
  timer_check_key_exists_id_ = startTimer(1000);
  DCHECK(timer_check_key_exists_id_ != 0);
}

void IServer::StopCheckKeyExistTimer() {
  if (timer_check_key_exists_id_ != 0) {
    killTimer(timer_check_key_exists_id_);
    timer_check_key_exists_id_ = 0;
  }
}

void IServer::StopCurrentEvent() {
  drv_->Interrupt();
}

bool IServer::IsConnected() const {
  return drv_->IsConnected() && drv_->IsAuthenticated();
}

bool IServer::IsCanRemote() const {
  return IsRemoteType(GetType());
}

bool IServer::IsSupportTTLKeys() const {
  return fastonosql::core::IsSupportTTLKeys(GetType());
}

bool IServer::IsCanCreateDatabase() const {
  return fastonosql::core::IsCanCreateDatabase(GetType());
}

bool IServer::IsCanRemoveDatabase() const {
  return fastonosql::core::IsCanRemoveDatabase(GetType());
}

core::translator_t IServer::GetTranslator() const {
  return drv_->GetTranslator();
}

core::connectionTypes IServer::GetType() const {
  return drv_->GetType();
}

std::string IServer::GetName() const {
  connection_path_t path = drv_->GetConnectionPath();
  return path.GetName();
}

core::IServerInfoSPtr IServer::GetCurrentServerInfo() const {
  if (IsConnected()) {
    return server_info_;
  }

  return core::IServerInfoSPtr();
}

IServer::database_t IServer::GetCurrentDatabaseInfo() const {
  if (IsConnected()) {
    return current_database_info_;
  }

  return database_t();
}

std::string IServer::GetDelimiter() const {
  return drv_->GetDelimiter();
}

std::string IServer::GetNsSeparator() const {
  return drv_->GetNsSeparator();
}

core::NsDisplayStrategy IServer::GetNsDisplayStrategy() const {
  return drv_->GetNsDisplayStrategy();
}

IDatabaseSPtr IServer::CreateDatabaseByInfo(core::IDataBaseInfoSPtr inf) {
  database_t db = FindDatabase(inf);
  return db ? CreateDatabase(inf) : IDatabaseSPtr();
}

proxy::IServer::database_t IServer::FindDatabase(core::IDataBaseInfoSPtr inf) const {
  if (!inf) {
    DNOTREACHED();
    return database_t();
  }

  for (database_t db : databases_) {
    if (db->GetName() == inf->GetName()) {
      return db;
    }
  }

  return database_t();
}

void IServer::Connect(const events_info::ConnectInfoRequest& req) {
  emit ConnectStarted(req);
  drv_->PrepareSettings();
  QEvent* ev = new events::ConnectRequestEvent(this, req);
  NotifyStartEvent(ev);
}

void IServer::Disconnect(const events_info::DisConnectInfoRequest& req) {
  StopCurrentEvent();
  emit DisconnectStarted(req);
  QEvent* ev = new events::DisconnectRequestEvent(this, req);
  NotifyStartEvent(ev);
}

void IServer::LoadDatabases(const events_info::LoadDatabasesInfoRequest& req) {
  emit LoadDatabasesStarted(req);
  QEvent* ev = new events::LoadDatabasesInfoRequestEvent(this, req);
  NotifyStartEvent(ev);
}

void IServer::LoadDatabaseContent(const events_info::LoadDatabaseContentRequest& req) {
  emit LoadDataBaseContentStarted(req);
  QEvent* ev = new events::LoadDatabaseContentRequestEvent(this, req);
  NotifyStartEvent(ev);
}

void IServer::Execute(const events_info::ExecuteInfoRequest& req) {
  emit ExecuteStarted(req);
  QEvent* ev = new events::ExecuteRequestEvent(this, req);
  NotifyStartEvent(ev);
}

void IServer::BackupToPath(const events_info::BackupInfoRequest& req) {
  emit BackupStarted(req);
  QEvent* ev = new events::BackupRequestEvent(this, req);
  NotifyStartEvent(ev);
}

void IServer::RestoreFromPath(const events_info::RestoreInfoRequest& req) {
  emit ExportStarted(req);
  QEvent* ev = new events::RestoreRequestEvent(this, req);
  NotifyStartEvent(ev);
}

void IServer::LoadServerInfo(const events_info::ServerInfoRequest& req) {
  emit LoadServerInfoStarted(req);
  QEvent* ev = new events::ServerInfoRequestEvent(this, req);
  NotifyStartEvent(ev);
}

void IServer::ServerProperty(const events_info::ServerPropertyInfoRequest& req) {
  emit LoadServerPropertyStarted(req);
  QEvent* ev = new events::ServerPropertyInfoRequestEvent(this, req);
  NotifyStartEvent(ev);
}

void IServer::RequestHistoryInfo(const events_info::ServerInfoHistoryRequest& req) {
  emit LoadServerHistoryInfoStarted(req);
  QEvent* ev = new events::ServerInfoHistoryRequestEvent(this, req);
  NotifyStartEvent(ev);
}

void IServer::ClearHistory(const events_info::ClearServerHistoryRequest& req) {
  emit ClearServerHistoryStarted(req);
  QEvent* ev = new events::ClearServerHistoryRequestEvent(this, req);
  NotifyStartEvent(ev);
}

void IServer::ChangeProperty(const events_info::ChangeServerPropertyInfoRequest& req) {
  emit ChangeServerPropertyStarted(req);
  QEvent* ev = new events::ChangeServerPropertyInfoRequestEvent(this, req);
  NotifyStartEvent(ev);
}

void IServer::LoadChannels(const events_info::LoadServerChannelsRequest& req) {
  emit LoadServerChannelsStarted(req);
  QEvent* ev = new events::LoadServerChannelsRequestEvent(this, req);
  NotifyStartEvent(ev);
}

void IServer::customEvent(QEvent* event) {
  QEvent::Type type = event->type();
  if (type == static_cast<QEvent::Type>(events::ConnectResponceEvent::EventType)) {
    events::ConnectResponceEvent* ev = static_cast<events::ConnectResponceEvent*>(event);
    HandleConnectEvent(ev);

    events::ConnectResponceEvent::value_type v = ev->value();
    common::Error err(v.errorInfo());
    if (!err) {
      events_info::ServerInfoRequest sreq(this);
      LoadServerInfo(sreq);

      events_info::DiscoveryInfoRequest dreq(this);
      ProcessDiscoveryInfo(dreq);
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
    events::CommandRootCompleatedEvent* ev = static_cast<events::CommandRootCompleatedEvent*>(event);
    events::CommandRootCompleatedEvent::value_type v = ev->value();
    emit RootCompleated(v);
  } else if (type == static_cast<QEvent::Type>(events::DisconnectResponceEvent::EventType)) {
    events::DisconnectResponceEvent* ev = static_cast<events::DisconnectResponceEvent*>(event);
    HandleDisconnectEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::LoadDatabasesInfoResponceEvent::EventType)) {
    events::LoadDatabasesInfoResponceEvent* ev = static_cast<events::LoadDatabasesInfoResponceEvent*>(event);
    HandleLoadDatabaseInfosEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::ServerInfoResponceEvent::EventType)) {
    events::ServerInfoResponceEvent* ev = static_cast<events::ServerInfoResponceEvent*>(event);
    HandleLoadServerInfoEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::ServerInfoHistoryResponceEvent::EventType)) {
    events::ServerInfoHistoryResponceEvent* ev = static_cast<events::ServerInfoHistoryResponceEvent*>(event);
    HandleLoadServerInfoHistoryEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::ClearServerHistoryResponceEvent::EventType)) {
    events::ClearServerHistoryResponceEvent* ev = static_cast<events::ClearServerHistoryResponceEvent*>(event);
    HandleClearServerHistoryResponceEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::ServerPropertyInfoResponceEvent::EventType)) {
    events::ServerPropertyInfoResponceEvent* ev = static_cast<events::ServerPropertyInfoResponceEvent*>(event);
    HandleLoadServerPropertyEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::ChangeServerPropertyInfoResponceEvent::EventType)) {
    events::ChangeServerPropertyInfoResponceEvent* ev =
        static_cast<events::ChangeServerPropertyInfoResponceEvent*>(event);
    HandleServerPropertyChangeEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::LoadServerChannelsResponceEvent::EventType)) {
    events::LoadServerChannelsResponceEvent* ev = static_cast<events::LoadServerChannelsResponceEvent*>(event);
    HandleLoadServerChannelsEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::BackupResponceEvent::EventType)) {
    events::BackupResponceEvent* ev = static_cast<events::BackupResponceEvent*>(event);
    HandleBackupEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::RestoreResponceEvent::EventType)) {
    events::RestoreResponceEvent* ev = static_cast<events::RestoreResponceEvent*>(event);
    HandleRestoreEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::LoadDatabaseContentResponceEvent::EventType)) {
    events::LoadDatabaseContentResponceEvent* ev = static_cast<events::LoadDatabaseContentResponceEvent*>(event);
    HandleLoadDatabaseContentEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::ExecuteResponceEvent::EventType)) {
    events::ExecuteResponceEvent* ev = static_cast<events::ExecuteResponceEvent*>(event);
    HandleExecuteEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::DiscoveryInfoResponceEvent::EventType)) {
    events::DiscoveryInfoResponceEvent* ev = static_cast<events::DiscoveryInfoResponceEvent*>(event);
    HandleDiscoveryInfoResponceEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::ProgressResponceEvent::EventType)) {
    events::ProgressResponceEvent* ev = static_cast<events::ProgressResponceEvent*>(event);
    events::ProgressResponceEvent::value_type v = ev->value();
    emit ProgressChanged(v);
  }

  return QObject::customEvent(event);
}

void IServer::timerEvent(QTimerEvent* event) {
  if (timer_check_key_exists_id_ == event->timerId() && IsConnected()) {
    database_t cdb = GetCurrentDatabaseInfo();
    HandleCheckDBKeys(cdb, 1);
  }
  QObject::timerEvent(event);
}

void IServer::NotifyStartEvent(QEvent* ev) {
  events_info::ProgressInfoResponce resp(0);
  emit ProgressChanged(resp);
  qApp->postEvent(drv_, ev);
}

void IServer::HandleConnectEvent(events::ConnectResponceEvent* ev) {
  auto v = ev->value();
  common::Error err(v.errorInfo());
  if (err) {
    LOG_ERROR(err, common::logging::LOG_LEVEL_ERR, true);
  }
  emit ConnectFinished(v);
}

void IServer::HandleDisconnectEvent(events::DisconnectResponceEvent* ev) {
  auto v = ev->value();
  common::Error err(v.errorInfo());
  if (err) {
    LOG_ERROR(err, common::logging::LOG_LEVEL_ERR, true);
  }
  emit DisconnectFinished(v);
}

void IServer::HandleLoadServerInfoEvent(events::ServerInfoResponceEvent* ev) {
  auto v = ev->value();
  common::Error err(v.errorInfo());
  if (err) {
    LOG_ERROR(err, common::logging::LOG_LEVEL_ERR, true);
  } else {
    server_info_ = v.info();
  }
  emit LoadServerInfoFinished(v);
}

void IServer::HandleLoadServerPropertyEvent(events::ServerPropertyInfoResponceEvent* ev) {
  auto v = ev->value();
  common::Error err(v.errorInfo());
  if (err) {
    LOG_ERROR(err, common::logging::LOG_LEVEL_ERR, true);
  }
  emit LoadServerPropertyFinished(v);
}

void IServer::HandleServerPropertyChangeEvent(events::ChangeServerPropertyInfoResponceEvent* ev) {
  auto v = ev->value();
  common::Error err(v.errorInfo());
  if (err) {
    LOG_ERROR(err, common::logging::LOG_LEVEL_ERR, true);
  }
  emit ChangeServerPropertyFinished(v);
}

void IServer::HandleLoadServerChannelsEvent(events::LoadServerChannelsResponceEvent* ev) {
  auto v = ev->value();
  common::Error err(v.errorInfo());
  if (err) {
    LOG_ERROR(err, common::logging::LOG_LEVEL_ERR, true);
  }
  emit LoadServerChannelsFinished(v);
}

void IServer::HandleBackupEvent(events::BackupResponceEvent* ev) {
  auto v = ev->value();
  common::Error err(v.errorInfo());
  if (err) {
    LOG_ERROR(err, common::logging::LOG_LEVEL_ERR, true);
  }
  emit BackupFinished(v);
}

void IServer::HandleRestoreEvent(events::RestoreResponceEvent* ev) {
  auto v = ev->value();
  common::Error err(v.errorInfo());
  if (err) {
    LOG_ERROR(err, common::logging::LOG_LEVEL_ERR, true);
  }
  emit ExportFinished(v);
}

void IServer::HandleExecuteEvent(events::ExecuteResponceEvent* ev) {
  auto v = ev->value();
  common::Error err(v.errorInfo());
  if (!err) {
    emit ExecuteFinished(v);
    return;
  }

  bool is_eintr = err->GetErrorCode() == common::COMMON_EINTR;
  auto payload = err->GetPayload();
  if (is_eintr && payload) {
    common::net::HostAndPortAndSlot* hs = static_cast<common::net::HostAndPortAndSlot*>(payload);
    common::net::HostAndPortAndSlot copy = *hs;
    std::string redirect_str = common::MemSPrintf("-> Redirected to slot [%d] located at %s:%d", copy.GetSlot(),
                                                  copy.GetHost(), copy.GetPort());
    common::Error red_error = common::make_error(redirect_str);
    emit RedirectRequested(copy, v);
    LOG_ERROR(red_error, common::logging::LOG_LEVEL_WARNING, true);
    emit ExecuteFinished(v);
    // delete hs;
    return;
  }

  LOG_ERROR(err, is_eintr ? common::logging::LOG_LEVEL_WARNING : common::logging::LOG_LEVEL_ERR, true);
  emit ExecuteFinished(v);
}

void IServer::HandleLoadDatabaseInfosEvent(events::LoadDatabasesInfoResponceEvent* ev) {
  auto v = ev->value();
  common::Error err(v.errorInfo());
  if (err) {
    LOG_ERROR(err, common::logging::LOG_LEVEL_ERR, true);
  } else {
    events_info::LoadDatabasesInfoResponce::database_info_cont_type dbs = v.databases;
    events_info::LoadDatabasesInfoResponce::database_info_cont_type tmp;
    for (size_t j = 0; j < dbs.size(); ++j) {
      core::IDataBaseInfoSPtr db = dbs[j];
      database_t dbs = FindDatabase(db);
      if (!dbs) {
        DCHECK(!db->IsDefault());
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
  common::Error err(v.errorInfo());
  if (err) {
    LOG_ERROR(err, common::logging::LOG_LEVEL_ERR, true);
  } else {
    database_t dbs = FindDatabase(v.inf);
    if (dbs) {
      dbs->SetKeys(v.keys);
      dbs->SetDBKeysCount(v.db_keys_count);
      v.inf = dbs;
    }
  }

  emit LoadDatabaseContentFinished(v);
}

void IServer::CreateDB(core::IDataBaseInfoSPtr db) {
  database_t dbs = FindDatabase(db);
  if (!dbs) {
    databases_.push_back(db);
  }
  emit DatabaseCreated(db);
}

void IServer::RemoveDatabase(core::IDataBaseInfoSPtr db) {
  databases_.erase(std::remove_if(databases_.begin(), databases_.end(),
                                  [db](database_t edb) { return db->GetName() == edb->GetName(); }));
  emit DatabaseRemoved(db);
}

void IServer::FlushCurrentDatabase() {
  database_t cdb = GetCurrentDatabaseInfo();
  if (!cdb) {
    return;
  }

  cdb->ClearKeys();
  cdb->SetDBKeysCount(0);
  emit DatabaseFlushed(cdb);
}

void IServer::ChangeCurrentDatabase(core::IDataBaseInfoSPtr db) {
  database_t cdb = GetCurrentDatabaseInfo();
  if (cdb) {
    if (db->GetName() == cdb->GetName()) {
      return;
    }
  }

  database_t founded;
  for (database_t cached_db : databases_) {
    if (db->GetName() == cached_db->GetName()) {
      founded = cached_db;
      founded->SetIsDefault(true);
    } else {
      cached_db->SetIsDefault(false);
    }
  }

  if (!founded) {
    founded = db;
    databases_.push_back(founded);
    current_database_info_ = founded;
  } else {
    current_database_info_ = founded;
  }

  DCHECK(founded->IsDefault());
  emit DatabaseChanged(founded);
}

void IServer::RemoveKey(core::NKey key) {
  database_t cdb = GetCurrentDatabaseInfo();
  if (!cdb) {
    return;
  }

  if (cdb->RemoveKey(key)) {
    emit KeyRemoved(cdb, key);
  }
}

void IServer::AddKey(core::NDbKValue key) {
  database_t cdb = GetCurrentDatabaseInfo();
  if (!cdb) {
    return;
  }

  if (cdb->InsertKey(key)) {
    emit KeyAdded(cdb, key);
  } else {
    emit KeyLoaded(cdb, key);
  }
}

void IServer::LoadKey(core::NDbKValue key) {
  database_t cdb = GetCurrentDatabaseInfo();
  if (!cdb) {
    return;
  }

  if (cdb->InsertKey(key)) {
    emit KeyAdded(cdb, key);
  } else {
    emit KeyLoaded(cdb, key);
  }
}

void IServer::RenameKey(core::NKey key, core::key_t new_name) {
  database_t cdb = GetCurrentDatabaseInfo();
  if (!cdb) {
    return;
  }

  if (cdb->RenameKey(key, new_name)) {
    emit KeyRenamed(cdb, key, new_name);
  }
}

void IServer::ChangeKeyTTL(core::NKey key, core::ttl_t ttl) {
  database_t cdb = GetCurrentDatabaseInfo();
  if (!cdb) {
    return;
  }

  if (cdb->UpdateKeyTTL(key, ttl)) {
    emit KeyTTLChanged(cdb, key, ttl);
  }
}

void IServer::LoadKeyTTL(core::NKey key, core::ttl_t ttl) {
  database_t cdb = GetCurrentDatabaseInfo();
  if (!cdb) {
    return;
  }

  if (ttl == EXPIRED_TTL) {
    if (cdb->RemoveKey(key)) {
      emit KeyRemoved(cdb, key);
    }
    return;
  }

  if (cdb->UpdateKeyTTL(key, ttl)) {
    emit KeyTTLChanged(cdb, key, ttl);
  }
}

void IServer::LoadModule(core::ModuleInfo module) {
  emit ModuleLoaded(module);
}

void IServer::UnLoadModule(core::ModuleInfo module) {
  emit ModuleUnLoaded(module);
}

void IServer::HandleCheckDBKeys(core::IDataBaseInfoSPtr db, core::ttl_t expired_time) {
  if (!db) {
    return;
  }

  auto keys = db->GetKeys();
  for (core::NDbKValue key : keys) {
    core::NKey nkey = key.GetKey();
    core::ttl_t key_ttl = nkey.GetTTL();
    if (key_ttl == NO_TTL) {
    } else if (key_ttl == EXPIRED_TTL) {
      if (db->RemoveKey(nkey)) {
        emit KeyRemoved(db, nkey);
      }
    } else {  // live
      const core::ttl_t new_ttl = key_ttl - expired_time;
      if (new_ttl == NO_TTL) {
        core::translator_t trans = GetTranslator();
        core::command_buffer_t load_ttl_cmd;
        common::Error err = trans->LoadKeyTTLCommand(nkey, &load_ttl_cmd);
        if (err) {
          return;
        }
        proxy::events_info::ExecuteInfoRequest req(this, load_ttl_cmd, 0, 0, true, true, core::C_INNER);
        Execute(req);
      } else {
        if (db->UpdateKeyTTL(nkey, new_ttl)) {
          emit KeyTTLChanged(db, nkey, new_ttl);
        }
      }
    }
  }
}

void IServer::HandleEnterModeEvent(events::EnterModeEvent* ev) {
  auto v = ev->value();
  common::Error err(v.errorInfo());
  if (err) {
    LOG_ERROR(err, common::logging::LOG_LEVEL_ERR, true);
  }

  emit ModeEntered(v);
}

void IServer::HandleLeaveModeEvent(events::LeaveModeEvent* ev) {
  auto v = ev->value();
  common::Error err(v.errorInfo());
  if (err) {
    LOG_ERROR(err, common::logging::LOG_LEVEL_ERR, true);
  }

  emit ModeLeaved(v);
}

void IServer::HandleLoadServerInfoHistoryEvent(events::ServerInfoHistoryResponceEvent* ev) {
  auto v = ev->value();
  common::Error err = v.errorInfo();
  if (err) {
    LOG_ERROR(err, common::logging::LOG_LEVEL_ERR, true);
  }

  emit LoadServerHistoryInfoFinished(v);
}

void IServer::HandleDiscoveryInfoResponceEvent(events::DiscoveryInfoResponceEvent* ev) {
  auto v = ev->value();
  common::Error err = v.errorInfo();
  if (err) {
    LOG_ERROR(err, common::logging::LOG_LEVEL_ERR, true);
  } else {
    database_t dbs = FindDatabase(v.dbinfo);
    if (!dbs) {
      current_database_info_ = v.dbinfo;
      databases_.push_back(current_database_info_);
    }
  }
  emit LoadDiscoveryInfoFinished(v);
}

void IServer::HandleClearServerHistoryResponceEvent(events::ClearServerHistoryResponceEvent* ev) {
  auto v = ev->value();
  common::Error err = v.errorInfo();
  if (err) {
    LOG_ERROR(err, common::logging::LOG_LEVEL_ERR, true);
  }

  emit ClearServerHistoryFinished(v);
}

void IServer::ProcessDiscoveryInfo(const events_info::DiscoveryInfoRequest& req) {
  emit LoadDiscoveryInfoStarted(req);
  QEvent* ev = new events::DiscoveryInfoRequestEvent(this, req);
  NotifyStartEvent(ev);
}

}  // namespace proxy
}  // namespace fastonosql
