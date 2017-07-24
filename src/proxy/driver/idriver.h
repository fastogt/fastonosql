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

#include <string>  // for string

#include <QObject>

#include <common/error.h>   // for Error
#include <common/macros.h>  // for WARN_UNUSED_RESULT
#include <common/value.h>   // for Value, Value::CommandLogging...

#include "core/connection_types.h"     // for core::connectionTypes
#include "core/db_key.h"               // for NKey (ptr only), NDbKValue (...
#include "core/icommand_translator.h"  // for translator_t

#include "core/database/idatabase_info.h"                    // for IDataBaseInfoSPtr, etc
#include "core/internal/cdb_connection_client.h"             // for CDBConnectionClient
#include "core/server/iserver_info.h"                        // for IServerInfoSPtr, etc
#include "proxy/connection_settings/iconnection_settings.h"  // for IConnectionSettingsBaseSPtr
#include "proxy/events/events.h"                             // for BackupRequestEvent, ChangeMa...

#include "core/global.h"  // for FastoObject (ptr only), etc

class QEvent;
class QThread;  // lines 37-37
class QTimerEvent;
namespace common {
namespace file_system {
class File;
}
}  // namespace common

namespace fastonosql {
namespace proxy {

class IDriver : public QObject, public core::CDBConnectionClient {
  Q_OBJECT
 public:
  virtual ~IDriver();

  static void Reply(QObject* reciver, QEvent* ev);

  // sync methods
  core::connectionTypes Type() const;
  connection_path_t ConnectionPath() const;

  virtual core::translator_t Translator() const = 0;

  void Start();
  void Stop();

  void Interrupt();

  virtual bool IsInterrupted() const = 0;
  virtual void SetInterrupted(bool interrupted) = 0;

  virtual bool IsConnected() const = 0;
  virtual bool IsAuthenticated() const = 0;

  virtual std::string Delimiter() const = 0;
  virtual std::string NsSeparator() const = 0;

 Q_SIGNALS:
  void ChildAdded(core::FastoObjectIPtr child);
  void ItemUpdated(core::FastoObject* item, common::ValueSPtr val);
  void ServerInfoSnapShoot(core::ServerInfoSnapShoot shot);

  void FlushedDB();
  void CurrentDataBaseChanged(core::IDataBaseInfoSPtr db);
  void KeyRemoved(core::NKey key);
  void KeyAdded(core::NDbKValue key);
  void KeyRenamed(core::NKey key, std::string new_name);
  void KeyLoaded(core::NDbKValue key);
  void KeyTTLChanged(core::NKey key, core::ttl_t ttl);
  void KeyTTLLoaded(core::NKey key, core::ttl_t ttl);
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
  virtual void HandleShutdownEvent(events::ShutDownRequestEvent* ev);
  virtual void HandleBackupEvent(events::BackupRequestEvent* ev);
  virtual void HandleExportEvent(events::ExportRequestEvent* ev);
  virtual void HandleChangePasswordEvent(events::ChangePasswordRequestEvent* ev);
  virtual void HandleChangeMaxConnectionEvent(events::ChangeMaxConnectionRequestEvent* ev);
  virtual void HandleLoadDatabaseInfosEvent(events::LoadDatabasesInfoRequestEvent* ev);

  const IConnectionSettingsBaseSPtr settings_;

  common::Error Execute(core::FastoObjectCommandIPtr cmd) WARN_UNUSED_RESULT;
  virtual core::FastoObjectCommandIPtr CreateCommand(core::FastoObject* parent,
                                                     const std::string& input,
                                                     core::CmdLoggingType ct) = 0;

  virtual core::FastoObjectCommandIPtr CreateCommandFast(const std::string& input,
                                                         core::CmdLoggingType ct) = 0;

 private:
  virtual common::Error SyncConnect() WARN_UNUSED_RESULT = 0;
  virtual common::Error SyncDisconnect() WARN_UNUSED_RESULT = 0;
  void HandleLoadServerInfoEvent(events::ServerInfoRequestEvent* ev);  // call ServerInfo
  void HandleLoadServerInfoHistoryEvent(events::ServerInfoHistoryRequestEvent* ev);
  void HandleDiscoveryInfoEvent(events::DiscoveryInfoRequestEvent* ev);
  void HandleClearServerHistoryEvent(events::ClearServerHistoryRequestEvent* ev);

  virtual common::Error ExecuteImpl(const std::string& command, core::FastoObject* out) = 0;

  virtual void OnFlushedCurrentDB() override;
  virtual void OnCurrentDataBaseChanged(core::IDataBaseInfo* info) override;
  virtual void OnKeysRemoved(const core::NKeys& keys) override;
  virtual void OnKeyAdded(const core::NDbKValue& key) override;
  virtual void OnKeyLoaded(const core::NDbKValue& key) override;
  virtual void OnKeyRenamed(const core::NKey& key, const std::string& new_key) override;
  virtual void OnKeyTTLChanged(const core::NKey& key, core::ttl_t ttl) override;
  virtual void OnKeyTTLLoaded(const core::NKey& key, core::ttl_t ttl) override;
  virtual void OnQuited() override;

  // internal methods
  virtual core::IServerInfoSPtr MakeServerInfoFromString(const std::string& val) = 0;
  virtual common::Error CurrentServerInfo(core::IServerInfo** info) = 0;
  virtual common::Error ServerDiscoveryInfo(core::IServerInfo** sinfo,
                                            core::IDataBaseInfo** dbinfo);
  virtual common::Error CurrentDataBaseInfo(core::IDataBaseInfo** info) = 0;
  virtual void InitImpl() = 0;
  virtual void ClearImpl() = 0;

 private:
  QThread* thread_;
  int timer_info_id_;
  common::file_system::ANSIFile* log_file_;
};

}  // namespace proxy
}  // namespace fastonosql
