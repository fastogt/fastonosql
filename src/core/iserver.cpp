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

#include "core/iserver.h"

#include <stddef.h>  // for size_t
#include <string>    // for string, operator==, etc

#include <QApplication>

#include "common/error.h"        // for Error
#include "common/macros.h"       // for VERIFY, CHECK, DNOTREACHED
#include "common/qt/utils_qt.h"  // for Event<>::value_type
#include "common/value.h"        // for ErrorValue

#include "core/connection_settings.h"  // for isRemoteType, etc
#include "core/events/events_info.h"   // for LoadDatabaseContentResponce, etc
#include "core/idriver.h"              // for IDriver

#include "common/qt/logger.h"  // for LOG_ERROR

namespace fastonosql {
namespace core {

IServerBase::~IServerBase() {}

IServer::IServer(IDriver* drv) : drv_(drv) {
  VERIFY(QObject::connect(drv_, &IDriver::addedChild, this, &IServer::addedChild));
  VERIFY(QObject::connect(drv_, &IDriver::itemUpdated, this, &IServer::itemUpdated));
  VERIFY(QObject::connect(drv_, &IDriver::serverInfoSnapShoot, this, &IServer::serverInfoSnapShoot));

  VERIFY(QObject::connect(drv_, &IDriver::keyRemoved, this, &IServer::keyRemoved));
  VERIFY(QObject::connect(drv_, &IDriver::keyAdded, this, &IServer::keyAdded));
  VERIFY(QObject::connect(drv_, &IDriver::keyTTLChanged, this, &IServer::keyTTLChanged));

  drv_->start();
}

IServer::~IServer() {
  drv_->interrupt();
  drv_->stop();
  delete drv_;
}

void IServer::stopCurrentEvent() {
  drv_->interrupt();
}

bool IServer::isConnected() const {
  return drv_->isConnected();
}

bool IServer::isAuthenticated() const {
  return drv_->isAuthenticated();
}

bool IServer::isCanRemote() const {
  return isRemoteType(type());
}

translator_t IServer::translator() const {
  return drv_->translator();
}

connectionTypes IServer::type() const {
  return drv_->type();
}

std::string IServer::name() const {
  IConnectionSettings::connection_path_t path = drv_->connectionPath();
  return path.name();
}

IServerInfoSPtr IServer::serverInfo() const {
  return drv_->serverInfo();
}

IDataBaseInfoSPtr IServer::currentDatabaseInfo() const {
  return drv_->currentDatabaseInfo();
}

std::string IServer::delimiter() const {
  return drv_->delimiter();
}

std::string IServer::nsSeparator() const {
  return drv_->nsSeparator();
}

IDatabaseSPtr IServer::createDatabaseByInfo(IDataBaseInfoSPtr inf) {
  bool isContains = containsDatabase(inf);
  return isContains ? createDatabase(inf) : IDatabaseSPtr();
}

bool IServer::containsDatabase(IDataBaseInfoSPtr inf) const {
  if (!inf) {
    DNOTREACHED();
    return false;
  }

  if (type() != inf->type()) {
    DNOTREACHED();
    return false;
  }

  for (size_t i = 0; i < databases_.size(); ++i) {
    database_t db = databases_[i];
    if (db->name() == inf->name()) {
      return true;
    }
  }

  return false;
}

void IServer::connect(const events_info::ConnectInfoRequest& req) {
  emit startedConnect(req);
  QEvent* ev = new events::ConnectRequestEvent(this, req);
  notify(ev);
}

void IServer::disconnect(const events_info::DisConnectInfoRequest& req) {
  emit startedDisconnect(req);
  QEvent* ev = new events::DisconnectRequestEvent(this, req);
  notify(ev);
}

void IServer::loadDatabases(const events_info::LoadDatabasesInfoRequest& req) {
  emit startedLoadDatabases(req);
  QEvent* ev = new events::LoadDatabasesInfoRequestEvent(this, req);
  notify(ev);
}

void IServer::loadDatabaseContent(const events_info::LoadDatabaseContentRequest& req) {
  emit startedLoadDataBaseContent(req);
  QEvent* ev = new events::LoadDatabaseContentRequestEvent(this, req);
  notify(ev);
}

void IServer::setDefaultDB(const events_info::SetDefaultDatabaseRequest& req) {
  emit startedSetDefaultDatabase(req);
  QEvent* ev = new events::SetDefaultDatabaseRequestEvent(this, req);
  notify(ev);
}

void IServer::clearDB(const events_info::ClearDatabaseRequest& req) {
  emit startedClearDatabase(req);
  QEvent* ev = new events::ClearDatabaseRequestEvent(this, req);
  notify(ev);
}

void IServer::execute(const events_info::ExecuteInfoRequest& req) {
  emit startedExecute(req);
  QEvent* ev = new events::ExecuteRequestEvent(this, req);
  notify(ev);
}

void IServer::shutDown(const events_info::ShutDownInfoRequest& req) {
  emit startedShutdown(req);
  QEvent* ev = new events::ShutDownRequestEvent(this, req);
  notify(ev);
}

void IServer::backupToPath(const events_info::BackupInfoRequest& req) {
  emit startedBackup(req);
  QEvent* ev = new events::BackupRequestEvent(this, req);
  notify(ev);
}

void IServer::exportFromPath(const events_info::ExportInfoRequest& req) {
  emit startedExport(req);
  QEvent* ev = new events::ExportRequestEvent(this, req);
  notify(ev);
}

void IServer::changePassword(const events_info::ChangePasswordRequest& req) {
  emit startedChangePassword(req);
  QEvent* ev = new events::ChangePasswordRequestEvent(this, req);
  notify(ev);
}

void IServer::setMaxConnection(const events_info::ChangeMaxConnectionRequest& req) {
  emit startedChangeMaxConnection(req);
  QEvent* ev = new events::ChangeMaxConnectionRequestEvent(this, req);
  notify(ev);
}

void IServer::loadServerInfo(const events_info::ServerInfoRequest& req) {
  emit startedLoadServerInfo(req);
  QEvent* ev = new events::ServerInfoRequestEvent(this, req);
  notify(ev);
}

void IServer::serverProperty(const events_info::ServerPropertyInfoRequest& req) {
  emit startedLoadServerProperty(req);
  QEvent* ev = new events::ServerPropertyInfoRequestEvent(this, req);
  notify(ev);
}

void IServer::requestHistoryInfo(const events_info::ServerInfoHistoryRequest& req) {
  emit startedLoadServerHistoryInfo(req);
  QEvent* ev = new events::ServerInfoHistoryRequestEvent(this, req);
  notify(ev);
}

void IServer::clearHistory(const events_info::ClearServerHistoryRequest& req) {
  emit startedClearServerHistory(req);
  QEvent* ev = new events::ClearServerHistoryRequestEvent(this, req);
  notify(ev);
}

void IServer::changeProperty(const events_info::ChangeServerPropertyInfoRequest& req) {
  emit startedChangeServerProperty(req);
  QEvent* ev = new events::ChangeServerPropertyInfoRequestEvent(this, req);
  notify(ev);
}

void IServer::customEvent(QEvent* event) {
  QEvent::Type type = event->type();
  if (type == static_cast<QEvent::Type>(events::ConnectResponceEvent::EventType)) {
    events::ConnectResponceEvent* ev = static_cast<events::ConnectResponceEvent*>(event);
    handleConnectEvent(ev);

    events::ConnectResponceEvent::value_type v = ev->value();
    common::Error er(v.errorInfo());
    if (!er) {
      events_info::DiscoveryInfoRequest dreq(this);
      processDiscoveryInfo(dreq);

      events_info::ProcessConfigArgsInfoRequest preq(this);
      processConfigArgs(preq);
    }
  } else if (type == static_cast<QEvent::Type>(events::EnterModeEvent::EventType)) {
    events::EnterModeEvent* ev = static_cast<events::EnterModeEvent*>(event);
    handleEnterModeEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::LeaveModeEvent::EventType)) {
    events::LeaveModeEvent* ev = static_cast<events::LeaveModeEvent*>(event);
    handleLeaveModeEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::CommandRootCreatedEvent::EventType)) {
    events::CommandRootCreatedEvent* ev = static_cast<events::CommandRootCreatedEvent*>(event);
    events::CommandRootCreatedEvent::value_type v = ev->value();
    emit rootCreated(v);
  } else if (type == static_cast<QEvent::Type>(events::CommandRootCompleatedEvent::EventType)) {
    events::CommandRootCompleatedEvent* ev =
        static_cast<events::CommandRootCompleatedEvent*>(event);
    events::CommandRootCompleatedEvent::value_type v = ev->value();
    emit rootCompleated(v);
  } else if (type == static_cast<QEvent::Type>(events::DisconnectResponceEvent::EventType)) {
    events::DisconnectResponceEvent* ev = static_cast<events::DisconnectResponceEvent*>(event);
    handleDisconnectEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::LoadDatabasesInfoResponceEvent::EventType)) {
    events::LoadDatabasesInfoResponceEvent* ev =
        static_cast<events::LoadDatabasesInfoResponceEvent*>(event);
    handleLoadDatabaseInfosEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::ServerInfoResponceEvent::EventType)) {
    events::ServerInfoResponceEvent* ev = static_cast<events::ServerInfoResponceEvent*>(event);
    handleLoadServerInfoEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::ServerInfoHistoryResponceEvent::EventType)) {
    events::ServerInfoHistoryResponceEvent* ev =
        static_cast<events::ServerInfoHistoryResponceEvent*>(event);
    handleLoadServerInfoHistoryEvent(ev);
  } else if (type ==
             static_cast<QEvent::Type>(events::ClearServerHistoryResponceEvent::EventType)) {
    events::ClearServerHistoryResponceEvent* ev =
        static_cast<events::ClearServerHistoryResponceEvent*>(event);
    handleClearServerHistoryResponceEvent(ev);
  } else if (type ==
             static_cast<QEvent::Type>(events::ServerPropertyInfoResponceEvent::EventType)) {
    events::ServerPropertyInfoResponceEvent* ev =
        static_cast<events::ServerPropertyInfoResponceEvent*>(event);
    handleLoadServerPropertyEvent(ev);
  } else if (type ==
             static_cast<QEvent::Type>(events::ChangeServerPropertyInfoResponceEvent::EventType)) {
    events::ChangeServerPropertyInfoResponceEvent* ev =
        static_cast<events::ChangeServerPropertyInfoResponceEvent*>(event);
    handleServerPropertyChangeEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::BackupResponceEvent::EventType)) {
    events::BackupResponceEvent* ev = static_cast<events::BackupResponceEvent*>(event);
    handleBackupEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::ExportResponceEvent::EventType)) {
    events::ExportResponceEvent* ev = static_cast<events::ExportResponceEvent*>(event);
    handleExportEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::ChangePasswordResponceEvent::EventType)) {
    events::ChangePasswordResponceEvent* ev =
        static_cast<events::ChangePasswordResponceEvent*>(event);
    handleChangePasswordEvent(ev);
  } else if (type ==
             static_cast<QEvent::Type>(events::ChangeMaxConnectionResponceEvent::EventType)) {
    events::ChangeMaxConnectionResponceEvent* ev =
        static_cast<events::ChangeMaxConnectionResponceEvent*>(event);
    handleChangeMaxConnection(ev);
  } else if (type ==
             static_cast<QEvent::Type>(events::LoadDatabaseContentResponceEvent::EventType)) {
    events::LoadDatabaseContentResponceEvent* ev =
        static_cast<events::LoadDatabaseContentResponceEvent*>(event);
    handleLoadDatabaseContentEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::ClearDatabaseResponceEvent::EventType)) {
    events::ClearDatabaseResponceEvent* ev =
        static_cast<events::ClearDatabaseResponceEvent*>(event);
    handleClearDatabaseResponceEvent(ev);
  } else if (type ==
             static_cast<QEvent::Type>(events::SetDefaultDatabaseResponceEvent::EventType)) {
    events::SetDefaultDatabaseResponceEvent* ev =
        static_cast<events::SetDefaultDatabaseResponceEvent*>(event);
    handleSetDefaultDatabaseEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::ExecuteResponceEvent::EventType)) {
    events::ExecuteResponceEvent* ev = static_cast<events::ExecuteResponceEvent*>(event);
    handleExecuteResponceEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::DiscoveryInfoResponceEvent::EventType)) {
    events::DiscoveryInfoResponceEvent* ev =
        static_cast<events::DiscoveryInfoResponceEvent*>(event);
    handleDiscoveryInfoResponceEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::ProgressResponceEvent::EventType)) {
    events::ProgressResponceEvent* ev = static_cast<events::ProgressResponceEvent*>(event);
    events::ProgressResponceEvent::value_type v = ev->value();
    emit progressChanged(v);
  }

  return QObject::customEvent(event);
}

void IServer::notify(QEvent* ev) {
  events_info::ProgressInfoResponce resp(0);
  emit progressChanged(resp);
  qApp->postEvent(drv_, ev);
}

void IServer::handleConnectEvent(events::ConnectResponceEvent* ev) {
  auto v = ev->value();
  common::Error er(v.errorInfo());
  if (er && er->isError()) {
    LOG_ERROR(er, true);
  }
  emit finishedConnect(v);
}

void IServer::handleDisconnectEvent(events::DisconnectResponceEvent* ev) {
  auto v = ev->value();
  common::Error er(v.errorInfo());
  if (er && er->isError()) {
    LOG_ERROR(er, true);
  }
  emit finishedDisconnect(v);
}

void IServer::handleLoadServerInfoEvent(events::ServerInfoResponceEvent* ev) {
  auto v = ev->value();
  common::Error er(v.errorInfo());
  if (er && er->isError()) {
    LOG_ERROR(er, true);
  }
  emit finishedLoadServerInfo(v);
}

void IServer::handleLoadServerPropertyEvent(events::ServerPropertyInfoResponceEvent* ev) {
  auto v = ev->value();
  common::Error er(v.errorInfo());
  if (er && er->isError()) {
    LOG_ERROR(er, true);
  }
  emit finishedLoadServerProperty(v);
}

void IServer::handleServerPropertyChangeEvent(events::ChangeServerPropertyInfoResponceEvent* ev) {
  auto v = ev->value();
  common::Error er(v.errorInfo());
  if (er && er->isError()) {
    LOG_ERROR(er, true);
  }
  emit finishedChangeServerProperty(v);
}

void IServer::handleShutdownEvent(events::ShutDownResponceEvent* ev) {
  auto v = ev->value();
  common::Error er(v.errorInfo());
  if (er && er->isError()) {
    LOG_ERROR(er, true);
  }
  emit finishedShutdown(v);
}

void IServer::handleBackupEvent(events::BackupResponceEvent* ev) {
  auto v = ev->value();
  common::Error er(v.errorInfo());
  if (er && er->isError()) {
    LOG_ERROR(er, true);
  }
  emit finishedBackup(v);
}

void IServer::handleExportEvent(events::ExportResponceEvent* ev) {
  auto v = ev->value();
  common::Error er(v.errorInfo());
  if (er && er->isError()) {
    LOG_ERROR(er, true);
  }
  emit finishedExport(v);
}

void IServer::handleChangePasswordEvent(events::ChangePasswordResponceEvent* ev) {
  auto v = ev->value();
  common::Error er(v.errorInfo());
  if (er && er->isError()) {
    LOG_ERROR(er, true);
  }

  emit finishedChangePassword(v);
}

void IServer::handleChangeMaxConnection(events::ChangeMaxConnectionResponceEvent* ev) {
  auto v = ev->value();
  common::Error er(v.errorInfo());
  if (er && er->isError()) {
    LOG_ERROR(er, true);
  }

  emit finishedChangeMaxConnection(v);
}

void IServer::handleExecuteResponceEvent(events::ExecuteResponceEvent* ev) {
  auto v = ev->value();
  common::Error er(v.errorInfo());
  if (er && er->isError()) {
    LOG_ERROR(er, true);
  }

  emit finishedExecute(v);
}

void IServer::handleLoadDatabaseInfosEvent(events::LoadDatabasesInfoResponceEvent* ev) {
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
      if (!containsDatabase(db)) {
        databases_.push_back(db);
      }
      tmp.push_back(db);
    }
    v.databases = tmp;
  }

  emit finishedLoadDatabases(v);
}

void IServer::handleLoadDatabaseContentEvent(events::LoadDatabaseContentResponceEvent* ev) {
  auto v = ev->value();
  common::Error er(v.errorInfo());
  if (er && er->isError()) {
    LOG_ERROR(er, true);
  } else {
    if (containsDatabase(v.inf)) {
      v.inf->setKeys(v.keys);
      v.inf->setDBKeysCount(v.db_keys_count);
    }
  }

  emit finishedLoadDatabaseContent(v);
}

void IServer::handleClearDatabaseResponceEvent(events::ClearDatabaseResponceEvent* ev) {
  auto v = ev->value();
  common::Error er = v.errorInfo();
  if (er && er->isError()) {
    LOG_ERROR(er, true);
  } else {
    if (containsDatabase(v.inf)) {
      v.inf->clearKeys();
    }
  }

  emit finishedClearDatabase(v);
}

void IServer::handleEnterModeEvent(events::EnterModeEvent* ev) {
  auto v = ev->value();
  common::Error er(v.errorInfo());
  if (er && er->isError()) {
    LOG_ERROR(er, true);
  }

  emit enteredMode(v);
}

void IServer::handleLeaveModeEvent(events::LeaveModeEvent* ev) {
  auto v = ev->value();
  common::Error er(v.errorInfo());
  if (er && er->isError()) {
    LOG_ERROR(er, true);
  }

  emit leavedMode(v);
}

void IServer::handleLoadServerInfoHistoryEvent(events::ServerInfoHistoryResponceEvent* ev) {
  auto v = ev->value();
  common::Error er = v.errorInfo();
  if (er && er->isError()) {
    LOG_ERROR(er, true);
  }

  emit finishedLoadServerHistoryInfo(v);
}

void IServer::handleDiscoveryInfoResponceEvent(events::DiscoveryInfoResponceEvent* ev) {
  auto v = ev->value();
  common::Error er = v.errorInfo();
  if (er && er->isError()) {
    LOG_ERROR(er, true);
  }
  emit finishedLoadDiscoveryInfo(v);
}

void IServer::handleClearServerHistoryResponceEvent(events::ClearServerHistoryResponceEvent* ev) {
  auto v = ev->value();
  common::Error er = v.errorInfo();
  if (er && er->isError()) {
    LOG_ERROR(er, true);
  }

  emit finishedClearServerHistory(v);
}

void IServer::handleSetDefaultDatabaseEvent(events::SetDefaultDatabaseResponceEvent* ev) {
  auto v = ev->value();
  common::Error er(v.errorInfo());
  if (er && er->isError()) {
    LOG_ERROR(er, true);
  } else {
    IDataBaseInfoSPtr inf = v.inf;
    for (size_t i = 0; i < databases_.size(); ++i) {
      databases_[i]->setIsDefault(false);
    }
    inf->setIsDefault(true);
  }

  emit finishedSetDefaultDatabase(v);
}

void IServer::processConfigArgs(const events_info::ProcessConfigArgsInfoRequest& req) {
  QEvent* ev = new events::ProcessConfigArgsRequestEvent(this, req);
  notify(ev);
}

void IServer::processDiscoveryInfo(const events_info::DiscoveryInfoRequest& req) {
  emit startedLoadDiscoveryInfo(req);
  QEvent* ev = new events::DiscoveryInfoRequestEvent(this, req);
  notify(ev);
}

IServerLocal::IServerLocal(IDriver* drv) : IServer(drv) {
  CHECK(!isRemoteType(drv->type()));
}

IServerRemote::IServerRemote(IDriver* drv) : IServer(drv) {
  CHECK(isRemoteType(drv->type()));
}

}  // namespace core
}  // namespace fastonosql
