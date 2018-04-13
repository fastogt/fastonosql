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

#pragma once

#include "core/icommand_translator.h"  // for translator_t

#include "core/display_strategy.h"

#include "proxy/events/events.h"        // for BackupResponceEvent, etc
#include "proxy/proxy_fwd.h"            // for IDatabaseSPtr
#include "proxy/server/iserver_base.h"  // for IServerBase

namespace fastonosql {
namespace proxy {

class IDriver;
class IServer : public IServerBase, public std::enable_shared_from_this<IServer> {
  Q_OBJECT
 public:
  typedef core::IDataBaseInfoSPtr database_t;
  typedef std::vector<database_t> databases_t;
  virtual ~IServer();

  // sync methods
  void StopCurrentEvent();
  bool IsConnected() const;
  bool IsCanRemote() const;
  bool IsSupportTTLKeys() const;
  bool IsCanCreateDatabase() const;
  bool IsCanRemoveDatabase() const;

  core::translator_t GetTranslator() const;

  core::connectionTypes GetType() const;
  virtual std::string GetName() const override;

  database_t GetCurrentDatabaseInfo() const;
  core::IServerInfoSPtr GetCurrentServerInfo() const;

  std::string GetDelimiter() const;
  std::string GetNsSeparator() const;
  core::NsDisplayStrategy GetNsDisplayStrategy() const;
  IDatabaseSPtr CreateDatabaseByInfo(core::IDataBaseInfoSPtr inf);
  database_t FindDatabase(core::IDataBaseInfoSPtr inf) const;

 Q_SIGNALS:  // only direct connections
  void ConnectStarted(const events_info::ConnectInfoRequest& req);
  void ConnectFinished(const events_info::ConnectInfoResponce& res);

  void DisconnectStarted(const events_info::DisConnectInfoRequest& req);
  void DisconnectFinished(const events_info::DisConnectInfoResponce& res);

  void BackupStarted(const events_info::BackupInfoRequest& req);
  void BackupFinished(const events_info::BackupInfoResponce& res);

  void ExportStarted(const events_info::RestoreInfoRequest& req);
  void ExportFinished(const events_info::RestoreInfoResponce& res);

  void ExecuteStarted(const events_info::ExecuteInfoRequest& req);
  void ExecuteFinished(const events_info::ExecuteInfoResponce& res);

  void LoadDatabasesStarted(const events_info::LoadDatabasesInfoRequest& req);
  void LoadDatabasesFinished(const events_info::LoadDatabasesInfoResponce& res);

  void LoadServerInfoStarted(const events_info::ServerInfoRequest& req);
  void LoadServerInfoFinished(const events_info::ServerInfoResponce& res);

  void LoadServerHistoryInfoStarted(const events_info::ServerInfoHistoryRequest& req);
  void LoadServerHistoryInfoFinished(const events_info::ServerInfoHistoryResponce& res);

  void ClearServerHistoryStarted(const events_info::ClearServerHistoryRequest& req);
  void ClearServerHistoryFinished(const events_info::ClearServerHistoryResponce& req);

  void LoadServerPropertyStarted(const events_info::ServerPropertyInfoRequest& req);
  void LoadServerPropertyFinished(const events_info::ServerPropertyInfoResponce& res);

  void ChangeServerPropertyStarted(const events_info::ChangeServerPropertyInfoRequest& req);
  void ChangeServerPropertyFinished(const events_info::ChangeServerPropertyInfoResponce& res);

  void LoadServerChannelsStarted(const events_info::LoadServerChannelsRequest& req);
  void LoadServerChannelsFinished(const events_info::LoadServerChannelsResponce& res);

  void ProgressChanged(const events_info::ProgressInfoResponce& res);

  void ModeEntered(const events_info::EnterModeInfo& res);
  void ModeLeaved(const events_info::LeaveModeInfo& res);

  void RootCreated(const events_info::CommandRootCreatedInfo& res);
  void RootCompleated(const events_info::CommandRootCompleatedInfo& res);

  void LoadDataBaseContentStarted(const events_info::LoadDatabaseContentRequest& req);
  void LoadDatabaseContentFinished(const events_info::LoadDatabaseContentResponce& res);

  void LoadDiscoveryInfoStarted(const events_info::DiscoveryInfoRequest& res);
  void LoadDiscoveryInfoFinished(const events_info::DiscoveryInfoResponce& res);

  void RedirectRequested(const common::net::HostAndPortAndSlot& host, const events_info::ExecuteInfoRequest& req);
 Q_SIGNALS:
  void ChildAdded(core::FastoObjectIPtr child);
  void ItemUpdated(core::FastoObject* item, common::ValueSPtr val);
  void ServerInfoSnapShooted(core::ServerInfoSnapShoot shot);

  void DatabaseCreated(core::IDataBaseInfoSPtr db);
  void DatabaseRemoved(core::IDataBaseInfoSPtr db);
  void DatabaseFlushed(core::IDataBaseInfoSPtr db);
  void DatabaseChanged(core::IDataBaseInfoSPtr db);

  void KeyAdded(core::IDataBaseInfoSPtr db, core::NDbKValue key);
  void KeyRemoved(core::IDataBaseInfoSPtr db, core::NKey key);
  void KeyLoaded(core::IDataBaseInfoSPtr db, core::NDbKValue key);
  void KeyRenamed(core::IDataBaseInfoSPtr db, core::NKey key, core::key_t new_name);
  void KeyTTLChanged(core::IDataBaseInfoSPtr db, core::NKey key, core::ttl_t ttl);
  void ModuleLoaded(core::ModuleInfo module);
  void ModuleUnLoaded(core::ModuleInfo module);
  void Disconnected();

 public:
  // async methods
  void Connect(const events_info::ConnectInfoRequest& req);              // signals: ConnectStarted, ConnectFinished
  void Disconnect(const events_info::DisConnectInfoRequest& req);        // signals: DisconnectStarted,
                                                                         // DisconnectFinished
  void LoadDatabases(const events_info::LoadDatabasesInfoRequest& req);  // signals: LoadDatabasesStarted,
                                                                         // LoadDatabasesFinished
  void LoadDatabaseContent(const events_info::LoadDatabaseContentRequest& req);  // signals: LoadDataBaseContentStarted,
                                                                                 // LoadDatabaseContentFinished
  void Execute(const events_info::ExecuteInfoRequest& req);                      // signals: ExecuteStarted

  void BackupToPath(const events_info::BackupInfoRequest& req);      // signals: BackupStarted, BackupFinished
  void RestoreFromPath(const events_info::RestoreInfoRequest& req);  // signals: ExportStarted, ExportFinished

  void LoadServerInfo(const events_info::ServerInfoRequest& req);  // signals:
  // LoadServerInfoStarted,
  // LoadServerInfoFinished
  void ServerProperty(const events_info::ServerPropertyInfoRequest& req);     // signals: LoadServerPropertyStarted,
                                                                              // LoadServerPropertyFinished
  void RequestHistoryInfo(const events_info::ServerInfoHistoryRequest& req);  // signals: LoadServerHistoryInfoStarted,
                                                                              // LoadServerHistoryInfoFinished
  void ClearHistory(const events_info::ClearServerHistoryRequest& req);       // signals: ClearServerHistoryStarted,
                                                                              // ClearServerHistoryFinished
  void ChangeProperty(
      const events_info::ChangeServerPropertyInfoRequest& req);  // signals: ChangeServerPropertyStarted,
                                                                 // ChangeServerPropertyFinished

  void LoadChannels(const events_info::LoadServerChannelsRequest& req);  // signals: LoadServerChannelsStarted,
                                                                         // LoadServerChannelsFinished

 protected:
  explicit IServer(IDriver* drv);  // take ownerships

  void StartCheckKeyExistTimer();
  void StopCheckKeyExistTimer();

  virtual void customEvent(QEvent* event) override;
  virtual void timerEvent(QTimerEvent* event) override;

  virtual IDatabaseSPtr CreateDatabase(core::IDataBaseInfoSPtr info) = 0;
  void NotifyStartEvent(QEvent* ev);

  // handle server events
  virtual void HandleConnectEvent(events::ConnectResponceEvent* ev);
  virtual void HandleDisconnectEvent(events::DisconnectResponceEvent* ev);
  virtual void HandleLoadServerInfoEvent(events::ServerInfoResponceEvent* ev);
  virtual void HandleLoadServerPropertyEvent(events::ServerPropertyInfoResponceEvent* ev);
  virtual void HandleServerPropertyChangeEvent(events::ChangeServerPropertyInfoResponceEvent* ev);
  virtual void HandleLoadServerChannelsEvent(events::LoadServerChannelsResponceEvent* ev);
  virtual void HandleBackupEvent(events::BackupResponceEvent* ev);
  virtual void HandleRestoreEvent(events::RestoreResponceEvent* ev);
  virtual void HandleExecuteEvent(events::ExecuteResponceEvent* ev);

  // handle database events
  virtual void HandleLoadDatabaseInfosEvent(events::LoadDatabasesInfoResponceEvent* ev);
  virtual void HandleLoadDatabaseContentEvent(events::LoadDatabaseContentResponceEvent* ev);

  // handle command events
  virtual void HandleDiscoveryInfoResponceEvent(events::DiscoveryInfoResponceEvent* ev);

  IDriver* const drv_;
  databases_t databases_;

 private Q_SLOTS:
  void CreateDB(core::IDataBaseInfoSPtr db);
  void RemoveDatabase(core::IDataBaseInfoSPtr db);
  void FlushCurrentDatabase();
  void ChangeCurrentDatabase(core::IDataBaseInfoSPtr db);

  void RemoveKey(core::NKey key);
  void AddKey(core::NDbKValue key);
  void LoadKey(core::NDbKValue key);
  void RenameKey(core::NKey key, core::key_t new_name);
  void ChangeKeyTTL(core::NKey key, core::ttl_t ttl);
  void LoadKeyTTL(core::NKey key, core::ttl_t ttl);
  void LoadModule(core::ModuleInfo module);
  void UnLoadModule(core::ModuleInfo module);

 private:
  void HandleCheckDBKeys(core::IDataBaseInfoSPtr db, core::ttl_t expired_time);

  void HandleEnterModeEvent(events::EnterModeEvent* ev);
  void HandleLeaveModeEvent(events::LeaveModeEvent* ev);

  // handle info events
  void HandleLoadServerInfoHistoryEvent(events::ServerInfoHistoryResponceEvent* ev);
  void HandleClearServerHistoryResponceEvent(events::ClearServerHistoryResponceEvent* ev);

  void ProcessDiscoveryInfo(const events_info::DiscoveryInfoRequest& req);

  core::IServerInfoSPtr server_info_;
  database_t current_database_info_;
  int timer_check_key_exists_id_;
};

}  // namespace proxy
}  // namespace fastonosql
