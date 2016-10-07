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

#include "core/db_connection/cdb_connection_client.h"      // for CDBConnectionClient
#include "core/connection_settings/connection_settings.h"  // for IConnectionSettingsBaseSPtr
#include "core/connection_types.h"                         // for connectionTypes
#include "core/db_key.h"                                   // for NKey (ptr only), NDbKValue (...
#include "core/events/events.h"                            // for BackupRequestEvent, ChangeMa...
#include "core/translator/icommand_translator.h"           // for translator_t
#include "core/driver/root_locker.h"

class QEvent;
class QThread;  // lines 37-37
class QTimerEvent;
namespace common {
namespace file_system {
class File;
}
}  // lines 41-41

namespace fastonosql {
namespace core {

class IDriver : public QObject, public CDBConnectionClient {
  Q_OBJECT
 public:
  virtual ~IDriver();

  static void reply(QObject* reciver, QEvent* ev);

  // sync methods
  connectionTypes type() const;
  IConnectionSettings::connection_path_t connectionPath() const;

  IServerInfoSPtr serverInfo() const;
  IDataBaseInfoSPtr currentDatabaseInfo() const;
  virtual translator_t translator() const = 0;

  void start();
  void stop();

  void interrupt();

  virtual bool isInterrupted() const = 0;
  virtual void setInterrupted(bool interrupted) = 0;

  virtual bool isConnected() const = 0;
  virtual bool isAuthenticated() const = 0;

  virtual std::string delimiter() const = 0;
  virtual std::string nsSeparator() const = 0;

 Q_SIGNALS:
  void addedChild(FastoObjectIPtr child);
  void itemUpdated(FastoObject* item, common::ValueSPtr val);
  void serverInfoSnapShoot(core::ServerInfoSnapShoot shot);

  void keyRemoved(core::IDataBaseInfoSPtr db, core::NKey key);
  void keyAdded(core::IDataBaseInfoSPtr db, core::NDbKValue key);
  void keyRenamed(core::IDataBaseInfoSPtr db, core::NKey key, std::string new_name);
  void keyLoaded(core::IDataBaseInfoSPtr db, core::NDbKValue key);
  void keyTTLChanged(core::IDataBaseInfoSPtr db, core::NKey key, core::ttl_t ttl);

 private Q_SLOTS:
  void init();
  void clear();

 protected:
  virtual void customEvent(QEvent* event);
  virtual void timerEvent(QTimerEvent* event);

  void notifyProgress(QObject* reciver, int value);

 protected:
  explicit IDriver(IConnectionSettingsBaseSPtr settings);

  // handle server events
  virtual void handleConnectEvent(events::ConnectRequestEvent* ev);
  virtual void handleDisconnectEvent(events::DisconnectRequestEvent* ev);

  virtual void handleProcessCommandLineArgs(events::ProcessConfigArgsRequestEvent* ev) = 0;
  virtual void handleExecuteEvent(events::ExecuteRequestEvent* ev);

  virtual void handleLoadDatabaseContentEvent(events::LoadDatabaseContentRequestEvent* ev) = 0;

  virtual void handleLoadServerPropertyEvent(events::ServerPropertyInfoRequestEvent* ev);
  virtual void handleServerPropertyChangeEvent(events::ChangeServerPropertyInfoRequestEvent* ev);
  virtual void handleShutdownEvent(events::ShutDownRequestEvent* ev);
  virtual void handleBackupEvent(events::BackupRequestEvent* ev);
  virtual void handleExportEvent(events::ExportRequestEvent* ev);
  virtual void handleChangePasswordEvent(events::ChangePasswordRequestEvent* ev);
  virtual void handleChangeMaxConnectionEvent(events::ChangeMaxConnectionRequestEvent* ev);
  virtual void handleLoadDatabaseInfosEvent(events::LoadDatabasesInfoRequestEvent* ev);
  virtual void handleClearDatabaseEvent(events::ClearDatabaseRequestEvent* ev);
  virtual void handleSetDefaultDatabaseEvent(events::SetDefaultDatabaseRequestEvent* ev);

  const IConnectionSettingsBaseSPtr settings_;

  RootLocker make_locker(QObject* reciver, const std::string& text, bool silence) {
    return RootLocker(this, reciver, text, silence);
  }

  common::Error execute(FastoObjectCommandIPtr cmd) WARN_UNUSED_RESULT;
  virtual FastoObjectCommandIPtr createCommand(FastoObject* parent,
                                               const std::string& input,
                                               common::Value::CommandLoggingType ct) = 0;

  virtual FastoObjectCommandIPtr createCommandFast(const std::string& input,
                                                   common::Value::CommandLoggingType ct) = 0;

 private:
  virtual common::Error syncConnect() WARN_UNUSED_RESULT = 0;
  virtual common::Error syncDisconnect() WARN_UNUSED_RESULT = 0;
  void handleLoadServerInfoEvent(events::ServerInfoRequestEvent* ev);  // call serverInfo
  void handleLoadServerInfoHistoryEvent(events::ServerInfoHistoryRequestEvent* ev);
  void handleDiscoveryInfoRequestEvent(events::DiscoveryInfoRequestEvent* ev);
  void handleClearServerHistoryRequestEvent(events::ClearServerHistoryRequestEvent* ev);

  virtual common::Error executeImpl(int argc, const char** argv, FastoObject* out) = 0;

  virtual void onCurrentDataBaseChanged(IDataBaseInfo* info) override;
  virtual void onKeysRemoved(const keys_t& keys) override;
  virtual void onKeyAdded(const key_and_value_t& key) override;
  virtual void onKeyLoaded(const key_and_value_t& key) override;
  virtual void onKeyRenamed(const key_t& key, const std::string& new_key) override;
  virtual void onKeyTTLChanged(const key_t& key, ttl_t ttl) override;

  // internal methods
  virtual IServerInfoSPtr makeServerInfoFromString(const std::string& val) = 0;
  virtual common::Error serverInfo(IServerInfo** info) = 0;
  virtual common::Error serverDiscoveryInfo(IServerInfo** sinfo, IDataBaseInfo** dbinfo);
  virtual common::Error currentDataBaseInfo(IDataBaseInfo** info) = 0;
  virtual void initImpl() = 0;
  virtual void clearImpl() = 0;

 private:
  IServerInfoSPtr server_info_;
  IDataBaseInfoSPtr current_database_info_;

  QThread* thread_;
  int timer_info_id_;
  common::file_system::File* log_file_;
};

}  // namespace core
}  // namespace fastonosql
