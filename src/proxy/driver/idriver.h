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

#include <QObject>

#include "core/icommand_translator.h"             // for translator_t
#include "core/internal/cdb_connection_client.h"  // for CDBConnectionClient
#include "core/module_info.h"

#include "proxy/connection_settings/iconnection_settings.h"  // for IConnectionSettingsBaseSPtr
#include "proxy/events/events.h"                             // for BackupRequestEvent, ChangeMa...

class QThread;  // lines 37-37
namespace common {
namespace file_system {
class ANSIFile;
}
}  // namespace common

namespace fastonosql {
namespace proxy {

// slot signal naming
// updateValue => valueUpdated

class IDriver : public QObject, public core::CDBConnectionClient {
  Q_OBJECT
 public:
  virtual ~IDriver();

  static void Reply(QObject* reciver, QEvent* ev);

  // sync methods
  void PrepareSettings();
  core::connectionTypes GetType() const;
  connection_path_t GetConnectionPath() const;
  std::string GetDelimiter() const;
  std::string GetNsSeparator() const;
  core::NsDisplayStrategy GetNsDisplayStrategy() const;

  virtual core::translator_t GetTranslator() const = 0;

  void Start();
  void Stop();

  void Interrupt();

  virtual bool IsInterrupted() const = 0;
  virtual void SetInterrupted(bool interrupted) = 0;

  virtual bool IsConnected() const = 0;
  virtual bool IsAuthenticated() const = 0;

 Q_SIGNALS:
  void ChildAdded(core::FastoObjectIPtr child);
  void ItemUpdated(core::FastoObject* item, common::ValueSPtr val);
  void ServerInfoSnapShooted(core::ServerInfoSnapShoot shot);

  void DBRemoved(core::IDataBaseInfoSPtr db);
  void DBCreated(core::IDataBaseInfoSPtr db);
  void DBFlushed();
  void DBChanged(core::IDataBaseInfoSPtr db);

  void KeyRemoved(core::NKey key);
  void KeyAdded(core::NDbKValue key);
  void KeyRenamed(core::NKey key, core::key_t new_name);
  void KeyLoaded(core::NDbKValue key);
  void KeyTTLChanged(core::NKey key, core::ttl_t ttl);
  void KeyTTLLoaded(core::NKey key, core::ttl_t ttl);
  void ModuleLoaded(core::ModuleInfo module);
  void ModuleUnLoaded(core::ModuleInfo module);
  void Disconnected();

 private Q_SLOTS:
  void Init();
  void Clear();

 protected:
  virtual void customEvent(QEvent* event) override;
  virtual void timerEvent(QTimerEvent* event) override;

  void NotifyProgress(QObject* reciver, int value);

 protected:
  explicit IDriver(IConnectionSettingsBaseSPtr settings);

  // handle server events
  virtual void HandleConnectEvent(events::ConnectRequestEvent* ev);
  virtual void HandleDisconnectEvent(events::DisconnectRequestEvent* ev);

  virtual void HandleExecuteEvent(events::ExecuteRequestEvent* ev);

  virtual void HandleLoadDatabaseContentEvent(events::LoadDatabaseContentRequestEvent* ev) = 0;

  virtual void HandleLoadServerPropertyEvent(events::ServerPropertyInfoRequestEvent* ev);
  virtual void HandleServerPropertyChangeEvent(events::ChangeServerPropertyInfoRequestEvent* ev);
  virtual void HandleLoadServerChannelsRequestEvent(events::LoadServerChannelsRequestEvent* ev);
  virtual void HandleBackupEvent(events::BackupRequestEvent* ev);
  virtual void HandleRestoreEvent(events::RestoreRequestEvent* ev);
  virtual void HandleLoadDatabaseInfosEvent(events::LoadDatabasesInfoRequestEvent* ev);

  template <typename T>
  inline std::shared_ptr<T> GetSpecificSettings() const {
    return std::static_pointer_cast<T>(settings_);
  }

  common::Error Execute(core::FastoObjectCommandIPtr cmd) WARN_UNUSED_RESULT;
  virtual core::FastoObjectCommandIPtr CreateCommand(core::FastoObject* parent,
                                                     const core::command_buffer_t& input,
                                                     core::CmdLoggingType ct) = 0;

  virtual core::FastoObjectCommandIPtr CreateCommandFast(const core::command_buffer_t& input,
                                                         core::CmdLoggingType ct) = 0;

  virtual core::IDataBaseInfoSPtr CreateDatabaseInfo(const std::string& name, bool is_default, size_t size) = 0;

 private:
  virtual common::Error SyncConnect() WARN_UNUSED_RESULT = 0;
  virtual common::Error SyncDisconnect() WARN_UNUSED_RESULT = 0;
  void HandleLoadServerInfoEvent(events::ServerInfoRequestEvent* ev);  // call ServerInfo
  void HandleLoadServerInfoHistoryEvent(events::ServerInfoHistoryRequestEvent* ev);
  void HandleDiscoveryInfoEvent(events::DiscoveryInfoRequestEvent* ev);
  void HandleClearServerHistoryEvent(events::ClearServerHistoryRequestEvent* ev);

  virtual common::Error ExecuteImpl(const core::command_buffer_t& command, core::FastoObject* out) = 0;

  virtual void OnCreatedDB(core::IDataBaseInfo* info) override;
  virtual void OnRemovedDB(core::IDataBaseInfo* info) override;

  virtual void OnFlushedCurrentDB() override;
  virtual void OnChangedCurrentDB(core::IDataBaseInfo* info) override;

  virtual void OnRemovedKeys(const core::NKeys& keys) override;
  virtual void OnAddedKey(const core::NDbKValue& key) override;
  virtual void OnLoadedKey(const core::NDbKValue& key) override;
  virtual void OnRenamedKey(const core::NKey& key, const core::key_t& new_key) override;
  virtual void OnChangedKeyTTL(const core::NKey& key, core::ttl_t ttl) override;
  virtual void OnLoadedKeyTTL(const core::NKey& key, core::ttl_t ttl) override;
  virtual void OnUnLoadedModule(const core::ModuleInfo& module) override;
  virtual void OnLoadedModule(const core::ModuleInfo& module) override;
  virtual void OnQuited() override;

 private:
  virtual core::IServerInfoSPtr MakeServerInfoFromString(const std::string& val) = 0;
  virtual void InitImpl() = 0;
  virtual void ClearImpl() = 0;

  virtual common::Error GetCurrentServerInfo(core::IServerInfo** info) = 0;
  virtual common::Error GetServerCommands(std::vector<const core::CommandInfo*>* commands) = 0;
  virtual common::Error GetServerLoadedModules(std::vector<core::ModuleInfo>* modules) = 0;
  virtual common::Error GetCurrentDataBaseInfo(core::IDataBaseInfo** info) = 0;

  common::Error GetServerDiscoveryInfo(core::IDataBaseInfo** dbinfo,
                                       std::vector<const core::CommandInfo*>* commands,
                                       std::vector<core::ModuleInfo>* modules);

  const IConnectionSettingsBaseSPtr settings_;
  QThread* thread_;
  int timer_info_id_;
  common::file_system::ANSIFile* log_file_;
};

}  // namespace proxy
}  // namespace fastonosql
