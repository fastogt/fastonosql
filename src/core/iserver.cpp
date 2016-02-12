/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    SiteOnYourDevice is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SiteOnYourDevice is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SiteOnYourDevice.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "core/iserver.h"

#include <string>

#include <QApplication>

#include "common/qt/convert_string.h"
#include "fasto/qt/logger.h"
#include "common/net/net.h"

#include "core/idatabase.h"
#include "core/idriver.h"

namespace fastonosql {
namespace {

template<bool isConnect>
struct connectFunct {
  template<typename t1, typename t2>
  bool operator()(const IServer* sender, t1 signal, const IServer* receiver,
                  t2 member, Qt::ConnectionType type) const {
      return QObject::disconnect(sender, signal, receiver, member);
  }
};

template<>
struct connectFunct<true> {
  template<typename t1, typename t2>
  bool operator()(const IServer* sender, t1 signal, const IServer* receiver,
                  t2 member, Qt::ConnectionType type) const {
      return QObject::connect(sender, signal, receiver, member, type);
  }
};

template<bool con>
void syncServersFunct(IServer *src, IServer *dsc) {
  if (!src || !dsc) {
    return;
  }

  connectFunct<con> func;
  func(src, &IServer::startedConnect, dsc, &IServer::startedConnect, Qt::UniqueConnection);
  func(src, &IServer::finishedConnect, dsc, &IServer::finishedConnect, Qt::UniqueConnection);

  func(src, &IServer::startedDisconnect, dsc, &IServer::startedDisconnect, Qt::UniqueConnection);
  func(src, &IServer::finishedDisconnect, dsc, &IServer::finishedDisconnect, Qt::UniqueConnection);

  func(src, &IServer::startedExecute, dsc, &IServer::startedExecute, Qt::UniqueConnection);

  func(src, &IServer::rootCreated, dsc, &IServer::rootCreated, Qt::UniqueConnection);
  func(src, &IServer::rootCompleated, dsc, &IServer::rootCompleated, Qt::UniqueConnection);

  func(src, &IServer::addedChild, dsc, &IServer::addedChild, Qt::UniqueConnection);
  func(src, &IServer::itemUpdated, dsc, &IServer::itemUpdated, Qt::UniqueConnection);
  func(src, &IServer::serverInfoSnapShoot, dsc,
       &IServer::serverInfoSnapShoot, Qt::UniqueConnection);
}

}

IServerBase::~IServerBase() {
}

IServer::IServer(IDriverSPtr drv, bool isSuperServer)
  : drv_(drv), is_super_server_(isSuperServer) {
  if (is_super_server_) {
    VERIFY(QObject::connect(drv_.get(), &IDriver::addedChild, this, &IServer::addedChild));
    VERIFY(QObject::connect(drv_.get(), &IDriver::itemUpdated, this, &IServer::itemUpdated));
    VERIFY(QObject::connect(drv_.get(), &IDriver::serverInfoSnapShoot,
                            this, &IServer::serverInfoSnapShoot));
  }
}

IServer::~IServer() {
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
  return IConnectionSettingsBase::isRemoteType(type());
}

bool IServer::isSuperServer() const {
  return is_super_server_;
}

void IServer::setSuperServer(bool isSuper){
  is_super_server_ = isSuper;
}

common::net::hostAndPort IServer::address() const {
  return drv_->address();
}

connectionTypes IServer::type() const {
  return drv_->connectionType();
}

QString IServer::name() const {
  return common::convertFromString<QString>(drv_->settings()->connectionName());
}

IDriverSPtr IServer::driver() const {
  return drv_;
}

ServerDiscoveryInfoSPtr IServer::discoveryInfo() const {
  return drv_->serverDiscoveryInfo();
}

ServerInfoSPtr IServer::serverInfo() const {
  return drv_->serverInfo();
}

IDataBaseInfoSPtr IServer::currentDatabaseInfo() const {
  return drv_->currentDatabaseInfo();
}

QString IServer::outputDelemitr() const {
  return common::convertFromString<QString>(drv_->outputDelemitr());
}

IDatabaseSPtr IServer::findDatabaseByInfo(IDataBaseInfoSPtr inf) const {
  DCHECK(inf);
  if (!inf) {
      return IDatabaseSPtr();
  }

  DCHECK(type() == inf->type());
  if (type() != inf->type()) {
      return IDatabaseSPtr();
  }

  for (int i = 0; i < databases_.size(); ++i) {
      IDataBaseInfoSPtr db = databases_[i]->info();
      if (db->name() == inf->name()) {
          return databases_[i];
      }
  }

  return IDatabaseSPtr();
}

IDatabaseSPtr IServer::findDatabaseByName(const std::string& name) const {
  for (int i = 0; i < databases_.size(); ++i) {
    IDataBaseInfoSPtr db = databases_[i]->info();
    if (db->name() == name) {
      return databases_[i];
    }
  }

  return IDatabaseSPtr();
}

void IServer::syncWithServer(IServer *src) {
  DCHECK(src != this);
  syncServersFunct<true>(src, this);
  // syncServersFunct<true>(this, src);
}

void IServer::unSyncFromServer(IServer *src) {
  DCHECK(src != this);
  syncServersFunct<false>(src, this);
  // syncServersFunct<false>(this, src);
}

void IServer::connect(const events_info::ConnectInfoRequest& req) {
  emit startedConnect(req);
  QEvent *ev = new events::ConnectRequestEvent(this, req);
  notify(ev);
}

void IServer::disconnect(const events_info::DisConnectInfoRequest& req) {
  emit startedDisconnect(req);
  QEvent *ev = new events::DisconnectRequestEvent(this, req);
  notify(ev);
}

void IServer::loadDatabases(const events_info::LoadDatabasesInfoRequest& req) {
  emit startedLoadDatabases(req);
  QEvent *ev = new events::LoadDatabasesInfoRequestEvent(this, req);
  notify(ev);
}

void IServer::loadDatabaseContent(const events_info::LoadDatabaseContentRequest& req) {
  emit startedLoadDataBaseContent(req);
  QEvent *ev = new events::LoadDatabaseContentRequestEvent(this, req);
  notify(ev);
}

void IServer::setDefaultDb(const events_info::SetDefaultDatabaseRequest& req) {
  emit startedSetDefaultDatabase(req);
  QEvent *ev = new events::SetDefaultDatabaseRequestEvent(this, req);
  notify(ev);
}

void IServer::execute(const events_info::ExecuteInfoRequest& req) {
  emit startedExecute(req);
  QEvent *ev = new events::ExecuteRequestEvent(this, req);
  notify(ev);
}

void IServer::executeCommand(const events_info::CommandRequest& req) {
  emit startedExecuteCommand(req);
  QEvent *ev = new events::CommandRequestEvent(this, req);
  notify(ev);
}

void IServer::shutDown(const events_info::ShutDownInfoRequest& req) {
  emit startedShutdown(req);
  QEvent *ev = new events::ShutDownRequestEvent(this, req);
  notify(ev);
}

void IServer::backupToPath(const events_info::BackupInfoRequest& req) {
  emit startedBackup(req);
  QEvent *ev = new events::BackupRequestEvent(this, req);
  notify(ev);
}

void IServer::exportFromPath(const events_info::ExportInfoRequest& req) {
  emit startedExport(req);
  QEvent *ev = new events::ExportRequestEvent(this, req);
  notify(ev);
}

void IServer::changePassword(const events_info::ChangePasswordRequest& req) {
  emit startedChangePassword(req);
  QEvent *ev = new events::ChangePasswordRequestEvent(this, req);
  notify(ev);
}

void IServer::setMaxConnection(const events_info::ChangeMaxConnectionRequest& req) {
  emit startedChangeMaxConnection(req);
  QEvent *ev = new events::ChangeMaxConnectionRequestEvent(this, req);
  notify(ev);
}

void IServer::loadServerInfo(const events_info::ServerInfoRequest& req) {
  emit startedLoadServerInfo(req);
  QEvent *ev = new events::ServerInfoRequestEvent(this, req);
  notify(ev);
}

void IServer::serverProperty(const events_info::ServerPropertyInfoRequest& req) {
  emit startedLoadServerProperty(req);
  QEvent *ev = new events::ServerPropertyInfoRequestEvent(this, req);
  notify(ev);
}

void IServer::requestHistoryInfo(const events_info::ServerInfoHistoryRequest& req) {
  emit startedLoadServerHistoryInfo(req);
  QEvent *ev = new events::ServerInfoHistoryRequestEvent(this, req);
  notify(ev);
}

void IServer::clearHistory(const events_info::ClearServerHistoryRequest &req) {
  emit startedClearServerHistory(req);
  QEvent *ev = new events::ClearServerHistoryRequestEvent(this, req);
  notify(ev);
}

void IServer::changeProperty(const events_info::ChangeServerPropertyInfoRequest& req) {
  emit startedChangeServerProperty(req);
  QEvent *ev = new events::ChangeServerPropertyInfoRequestEvent(this, req);
  notify(ev);
}

void IServer::customEvent(QEvent *event) {
  using namespace events;
  QEvent::Type type = event->type();
  if (type == static_cast<QEvent::Type>(ConnectResponceEvent::EventType)) {
    ConnectResponceEvent *ev = static_cast<ConnectResponceEvent*>(event);
    handleConnectEvent(ev);

    ConnectResponceEvent::value_type v = ev->value();
    common::Error er(v.errorInfo());
    if (!er) {
      events_info::DiscoveryInfoRequest dreq(this);
      processDiscoveryInfo(dreq);

      events_info::ProcessConfigArgsInfoRequest preq(this);
      processConfigArgs(preq);
    }
  } else if (type == static_cast<QEvent::Type>(EnterModeEvent::EventType)) {
    EnterModeEvent *ev = static_cast<EnterModeEvent*>(event);
    EnterModeEvent::value_type v = ev->value();
    emit enteredMode(v);
  } else if (type == static_cast<QEvent::Type>(LeaveModeEvent::EventType)) {
    LeaveModeEvent *ev = static_cast<LeaveModeEvent*>(event);
    LeaveModeEvent::value_type v = ev->value();
    emit leavedMode(v);
  } else if (type == static_cast<QEvent::Type>(CommandRootCreatedEvent::EventType)) {
    CommandRootCreatedEvent *ev = static_cast<CommandRootCreatedEvent*>(event);
    CommandRootCreatedEvent::value_type v = ev->value();
    emit rootCreated(v);
  } else if (type == static_cast<QEvent::Type>(CommandRootCompleatedEvent::EventType)) {
    CommandRootCompleatedEvent *ev = static_cast<CommandRootCompleatedEvent*>(event);
    CommandRootCompleatedEvent::value_type v = ev->value();
    emit rootCompleated(v);
  } else if (type == static_cast<QEvent::Type>(DisconnectResponceEvent::EventType)) {
    DisconnectResponceEvent *ev = static_cast<DisconnectResponceEvent*>(event);
    handleDisconnectEvent(ev);
  } else if (type == static_cast<QEvent::Type>(LoadDatabasesInfoResponceEvent::EventType)) {
    LoadDatabasesInfoResponceEvent *ev = static_cast<LoadDatabasesInfoResponceEvent*>(event);
    handleLoadDatabaseInfosEvent(ev);
  } else if (type == static_cast<QEvent::Type>(ServerInfoResponceEvent::EventType)) {
    ServerInfoResponceEvent *ev = static_cast<ServerInfoResponceEvent*>(event);
    handleLoadServerInfoEvent(ev);
  } else if (type == static_cast<QEvent::Type>(ServerInfoHistoryResponceEvent::EventType)) {
    ServerInfoHistoryResponceEvent *ev = static_cast<ServerInfoHistoryResponceEvent*>(event);
    handleLoadServerInfoHistoryEvent(ev);
  } else if (type == static_cast<QEvent::Type>(ClearServerHistoryResponceEvent::EventType)) {
    ClearServerHistoryResponceEvent *ev = static_cast<ClearServerHistoryResponceEvent*>(event);
    handleClearServerHistoryResponceEvent(ev);
  } else if (type == static_cast<QEvent::Type>(ServerPropertyInfoResponceEvent::EventType)) {
    ServerPropertyInfoResponceEvent *ev = static_cast<ServerPropertyInfoResponceEvent*>(event);
    handleLoadServerPropertyEvent(ev);
  } else if (type == static_cast<QEvent::Type>(ChangeServerPropertyInfoResponceEvent::EventType)) {
    ChangeServerPropertyInfoResponceEvent *ev = static_cast<ChangeServerPropertyInfoResponceEvent*>(event);
    handleServerPropertyChangeEvent(ev);
  } else if (type == static_cast<QEvent::Type>(BackupResponceEvent::EventType)) {
    BackupResponceEvent *ev = static_cast<BackupResponceEvent*>(event);
    handleBackupEvent(ev);
  } else if (type == static_cast<QEvent::Type>(ExportResponceEvent::EventType)) {
    ExportResponceEvent *ev = static_cast<ExportResponceEvent*>(event);
    handleExportEvent(ev);
  } else if (type == static_cast<QEvent::Type>(ChangePasswordResponceEvent::EventType)) {
    ChangePasswordResponceEvent *ev = static_cast<ChangePasswordResponceEvent*>(event);
    handleChangePasswordEvent(ev);
  } else if (type == static_cast<QEvent::Type>(ChangeMaxConnectionResponceEvent::EventType)) {
    ChangeMaxConnectionResponceEvent *ev = static_cast<ChangeMaxConnectionResponceEvent*>(event);
    handleChangeMaxConnection(ev);
  } else if (type == static_cast<QEvent::Type>(LoadDatabaseContentResponceEvent::EventType)) {
    LoadDatabaseContentResponceEvent *ev = static_cast<LoadDatabaseContentResponceEvent*>(event);
    handleLoadDatabaseContentEvent(ev);
  } else if (type == static_cast<QEvent::Type>(SetDefaultDatabaseResponceEvent::EventType)) {
    SetDefaultDatabaseResponceEvent *ev = static_cast<SetDefaultDatabaseResponceEvent*>(event);
    handleSetDefaultDatabaseEvent(ev);
  } else if (type == static_cast<QEvent::Type>(CommandResponceEvent::EventType)) {
    CommandResponceEvent *ev = static_cast<CommandResponceEvent*>(event);
    handleCommandResponceEvent(ev);
  } else if (type == static_cast<QEvent::Type>(DiscoveryInfoResponceEvent::EventType)) {
    DiscoveryInfoResponceEvent *ev = static_cast<DiscoveryInfoResponceEvent*>(event);
    handleDiscoveryInfoResponceEvent(ev);
  } else if (type == static_cast<QEvent::Type>(ProgressResponceEvent::EventType)) {
    ProgressResponceEvent *ev = static_cast<ProgressResponceEvent*>(event);
    ProgressResponceEvent::value_type v = ev->value();
    emit progressChanged(v);
  }

  return QObject::customEvent(event);
}

void IServer::notify(QEvent *ev) {
  events_info::ProgressInfoResponce resp(0);
  emit progressChanged(resp);
  qApp->postEvent(drv_.get(), ev);
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

void IServer::handleLoadDatabaseInfosEvent(events::LoadDatabasesInfoResponceEvent* ev) {
  auto v = ev->value();
  common::Error er(v.errorInfo());
  if (er && er->isError()) {
    LOG_ERROR(er, true);
    databases_.clear();
  } else {
    events_info::LoadDatabasesInfoResponce::database_info_cont_type dbs = v.databases;
    events_info::LoadDatabasesInfoResponce::database_info_cont_type tmp;
    for (int j = 0; j < dbs.size(); ++j) {
      IDataBaseInfoSPtr db = dbs[j];
      IDatabaseSPtr datab = findDatabaseByInfo(db);
      if (!datab) {
        datab = createDatabase(db);
        databases_.push_back(datab);
       }
       tmp.push_back(datab->info());
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
    IDatabaseSPtr db = findDatabaseByInfo(v.inf);
    if (db) {
      IDataBaseInfoSPtr rdb = db->info();
      if (rdb) {
        rdb->setKeys(v.keys);
      }
    }
  }

  emit finishedLoadDatabaseContent(v);
}

void IServer::handleLoadServerInfoHistoryEvent(events::ServerInfoHistoryResponceEvent *ev) {
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
    for (int i = 0; i < databases_.size(); ++i) {
      IDatabaseSPtr db = databases_[i];
      IDataBaseInfoSPtr info = db->info();
      if (info->name() == inf->name()) {
        info->setIsDefault(true);
      } else {
        info->setIsDefault(false);
      }
    }
  }

  emit finishedSetDefaultDatabase(v);
}

void IServer::handleCommandResponceEvent(events::CommandResponceEvent* ev) {
  auto v = ev->value();
  common::Error er(v.errorInfo());
  if (er && er->isError()) {
      LOG_ERROR(er, true);
  }
  emit finishedExecuteCommand(v);
}

void IServer::processConfigArgs(const events_info::ProcessConfigArgsInfoRequest &req) {
  QEvent *ev = new events::ProcessConfigArgsRequestEvent(this, req);
  notify(ev);
}

void IServer::processDiscoveryInfo(const events_info::DiscoveryInfoRequest& req) {
  emit startedLoadDiscoveryInfo(req);
  QEvent *ev = new events::DiscoveryInfoRequestEvent(this, req);
  notify(ev);
}

}  // namespace fastonosql
