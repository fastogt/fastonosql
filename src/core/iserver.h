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

#pragma once

#include "core/core_fwd.h"

#include "core/events/events.h"

namespace fastonosql {

class IServerBase
      : public QObject {
  Q_OBJECT
 public:
  virtual QString name() const = 0;
  virtual ~IServerBase();
};

class IServer
  : public IServerBase, public std::enable_shared_from_this<IServer> {
 Q_OBJECT
 friend class ServersManager;
 public:
  typedef std::vector<IDatabaseSPtr> databases_container_t;

  IServer(IDriverSPtr drv, bool isSuperServer);
  virtual ~IServer();

  //sync methods
  void stopCurrentEvent();
  bool isConnected() const;
  bool isAuthenticated() const;

  bool isSuperServer() const;

  bool isLocalHost() const;

  connectionTypes type() const;
  QString name() const;
  IDriverSPtr driver() const;

  DataBaseInfoSPtr currentDatabaseInfo() const;
  ServerDiscoveryInfoSPtr discoveryInfo() const;
  ServerInfoSPtr serverInfo() const;

  QString address() const;
  QString outputDelemitr() const;
  IDatabaseSPtr findDatabaseByInfo(DataBaseInfoSPtr inf) const;
  IDatabaseSPtr findDatabaseByName(const std::string& name) const;

  virtual void syncWithServer(IServer* src);
  virtual void unSyncFromServer(IServer* src);

 Q_SIGNALS: //only direct connections
  void startedConnect(const EventsInfo::ConnectInfoRequest& req);
  void finishedConnect(const EventsInfo::ConnectInfoResponce& res);

  void startedDisconnect(const EventsInfo::DisConnectInfoRequest& req);
  void finishedDisconnect(const EventsInfo::DisConnectInfoResponce& res);

  void startedShutdown(const EventsInfo::ShutDownInfoRequest& req);
  void finishedShutdown(const EventsInfo::ShutDownInfoResponce& res);

  void startedBackup(const EventsInfo::BackupInfoRequest& req);
  void finishedBackup(const EventsInfo::BackupInfoResponce& res);

  void startedExport(const EventsInfo::ExportInfoRequest& req);
  void finishedExport(const EventsInfo::ExportInfoResponce& res);

  void startedChangePassword(const EventsInfo::ChangePasswordRequest& req);
  void finishedChangePassword(const EventsInfo::ChangePasswordResponce& res);

  void startedChangeMaxConnection(const EventsInfo::ChangeMaxConnectionRequest& req);
  void finishedChangeMaxConnection(const EventsInfo::ChangeMaxConnectionResponce& res);

  void startedExecute(const EventsInfo::ExecuteInfoRequest& req);

  void startedLoadDatabases(const EventsInfo::LoadDatabasesInfoRequest& req);
  void finishedLoadDatabases(const EventsInfo::LoadDatabasesInfoResponce& res);

  void startedLoadServerInfo(const EventsInfo::ServerInfoRequest& req);
  void finishedLoadServerInfo(const EventsInfo::ServerInfoResponce& res);

  void startedLoadServerHistoryInfo(const EventsInfo::ServerInfoHistoryRequest& req);
  void finishedLoadServerHistoryInfo(const EventsInfo::ServerInfoHistoryResponce& res);

  void startedClearServerHistory(const EventsInfo::ClearServerHistoryRequest& req);
  void finishedClearServerHistory(const EventsInfo::ClearServerHistoryResponce& req);

  void startedLoadServerProperty(const EventsInfo::ServerPropertyInfoRequest& req);
  void finishedLoadServerProperty(const EventsInfo::ServerPropertyInfoResponce& res);

  void startedChangeServerProperty(const EventsInfo::ChangeServerPropertyInfoRequest& req);
  void finishedChangeServerProperty(const EventsInfo::ChangeServerPropertyInfoResponce& res);

  void progressChanged(const EventsInfo::ProgressInfoResponce& res);

  void enteredMode(const EventsInfo::EnterModeInfo& res);
  void leavedMode(const EventsInfo::LeaveModeInfo& res);

  void rootCreated(const EventsInfo::CommandRootCreatedInfo& res);
  void rootCompleated(const EventsInfo::CommandRootCompleatedInfo& res);

  void startedLoadDataBaseContent(const EventsInfo::LoadDatabaseContentRequest& req);
  void finishedLoadDatabaseContent(const EventsInfo::LoadDatabaseContentResponce& res);

  void startedSetDefaultDatabase(const EventsInfo::SetDefaultDatabaseRequest& req);
  void finishedSetDefaultDatabase(const EventsInfo::SetDefaultDatabaseResponce& res);

  void startedExecuteCommand(const EventsInfo::CommandRequest& req);
  void finishedExecuteCommand(const EventsInfo::CommandResponce& res);

  void startedLoadDiscoveryInfo(const EventsInfo::DiscoveryInfoRequest& res);
  void finishedLoadDiscoveryInfo(const EventsInfo::DiscoveryInfoResponce& res);

 Q_SIGNALS:
  void addedChild(FastoObject *child);
  void itemUpdated(FastoObject* item, common::Value* val);
  void serverInfoSnapShoot(ServerInfoSnapShoot shot);

 public:
  //async methods
  void connect(const EventsInfo::ConnectInfoRequest &req); //signals: startedConnect, finishedConnect
  void disconnect(const EventsInfo::DisConnectInfoRequest &req); //signals: startedDisconnect, finishedDisconnect
  void loadDatabases(const EventsInfo::LoadDatabasesInfoRequest &req); //signals: startedLoadDatabases, finishedLoadDatabases
  void loadDatabaseContent(const EventsInfo::LoadDatabaseContentRequest &req); //signals: startedLoadDataBaseContent, finishedLoadDatabaseContent
  void setDefaultDb(const EventsInfo::SetDefaultDatabaseRequest &req); //signals: startedSetDefaultDatabase, finishedSetDefaultDatabase
  void execute(const EventsInfo::ExecuteInfoRequest &req); //signals: startedExecute
  void executeCommand(const EventsInfo::CommandRequest &req); //signals: startedExecuteCommand, finishedExecuteCommand
  void shutDown(const EventsInfo::ShutDownInfoRequest &req); //signals: startedShutdown, finishedShutdown
  void backupToPath(const EventsInfo::BackupInfoRequest &req); //signals: startedBackup, finishedBackup
  void exportFromPath(const EventsInfo::ExportInfoRequest &req); //signals: startedExport, finishedExport
  void changePassword(const EventsInfo::ChangePasswordRequest &req); //signals: startedChangePassword, finishedChangePassword
  void setMaxConnection(const EventsInfo::ChangeMaxConnectionRequest &req);//signals: startedChangeMaxConnection, finishedChangeMaxConnection
  void loadServerInfo(const EventsInfo::ServerInfoRequest &req); //signals: startedLoadServerInfo, finishedLoadServerInfo
  void serverProperty(const EventsInfo::ServerPropertyInfoRequest &req); //signals: startedLoadServerProperty, finishedLoadServerProperty
  void requestHistoryInfo(const EventsInfo::ServerInfoHistoryRequest &req); //signals: startedLoadServerHistoryInfo, finishedLoadServerHistoryInfo
  void clearHistory(const EventsInfo::ClearServerHistoryRequest &req); //signals: startedClearServerHistory, finishedClearServerHistory
  void changeProperty(const EventsInfo::ChangeServerPropertyInfoRequest &req); //signals: startedChangeServerProperty, finishedChangeServerProperty

 protected:
  virtual void customEvent(QEvent* event);

  virtual IDatabaseSPtr createDatabase(DataBaseInfoSPtr info) = 0;
  void notify(QEvent* ev);

  // handle server events
  virtual void handleConnectEvent(events::ConnectResponceEvent* ev);
  virtual void handleDisconnectEvent(events::DisconnectResponceEvent* ev);
  virtual void handleLoadServerInfoEvent(events::ServerInfoResponceEvent* ev);
  virtual void handleLoadServerPropertyEvent(events::ServerPropertyInfoResponceEvent* ev);
  virtual void handleServerPropertyChangeEvent(events::ChangeServerPropertyInfoResponceEvent* ev);
  virtual void handleShutdownEvent(events::ShutDownResponceEvent* ev);
  virtual void handleBackupEvent(events::BackupResponceEvent* ev);
  virtual void handleExportEvent(events::ExportResponceEvent* ev);
  virtual void handleChangePasswordEvent(events::ChangePasswordResponceEvent* ev);
  virtual void handleChangeMaxConnection(events::ChangeMaxConnectionResponceEvent* ev);

  // handle database events
  virtual void handleLoadDatabaseInfosEvent(events::LoadDatabasesInfoResponceEvent* ev);
  virtual void handleLoadDatabaseContentEvent(events::LoadDatabaseContentResponceEvent* ev);
  virtual void handleSetDefaultDatabaseEvent(events::SetDefaultDatabaseResponceEvent* ev);

  // handle command events
  virtual void handleCommandResponceEvent(events::CommandResponceEvent* ev);

  const IDriverSPtr drv_;
  databases_container_t databases_;

 private:
  // handle info events
  void handleLoadServerInfoHistoryEvent(events::ServerInfoHistoryResponceEvent* ev);

  void handleDiscoveryInfoResponceEvent(events::DiscoveryInfoResponceEvent* ev);

  void handleClearServerHistoryResponceEvent(events::ClearServerHistoryResponceEvent* ev);

  void processConfigArgs(const EventsInfo::ProcessConfigArgsInfoRequest &req);
  void processDiscoveryInfo(const EventsInfo::DiscoveryInfoRequest &req);

  bool isSuperServer_;
};

}
