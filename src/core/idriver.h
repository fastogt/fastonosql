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

#include <QObject>

#include "common/error.h"      // for Error
#include "common/macros.h"     // for WARN_UNUSED_RESULT
#include "common/net/types.h"  // for HostAndPort
#include "common/types.h"      // for time64_t
#include "core/connection_settings.h"
#include "core/connection_types.h"  // for connectionTypes
#include "core/cdb_connection_client.h"
#include "core/types.h"  // for IDataBaseInfo (ptr only), etc
#include "core/icommand_translator.h"

#include "core/events/events.h"

#include "global/global.h"  // for FastoObject, etc

class QThread;

namespace common {
namespace file_system {
class File;
}
}

namespace fastonosql {
namespace core {

class IDriver : public QObject,
                public CDBConnectionClient,
                private FastoObject::IFastoObjectObserver {
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
  void serverInfoSnapShoot(ServerInfoSnapShoot shot);

  void keyRemoved(core::IDataBaseInfoSPtr db, core::NKey key);
  void keyAdded(core::IDataBaseInfoSPtr db, core::NDbKValue key);
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
  virtual void handleLoadDatabaseInfosEvent(
      events::LoadDatabasesInfoRequestEvent* ev);  // call currentDatabaseInfo
  virtual void handleClearDatabaseEvent(events::ClearDatabaseRequestEvent* ev);
  virtual void handleSetDefaultDatabaseEvent(events::SetDefaultDatabaseRequestEvent* ev);

  const IConnectionSettingsBaseSPtr settings_;

  class RootLocker {
   public:
    RootLocker(IDriver* parent, QObject* receiver, const std::string& text, bool silence);
    ~RootLocker();

    FastoObjectIPtr root() const;

   private:
    FastoObjectIPtr root_;
    IDriver* parent_;
    QObject* receiver_;
    const common::time64_t tstart_;
    const bool silence_;
  };

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

  // notification of execute events
  virtual void addedChildren(FastoObjectIPtr child) override;
  virtual void updated(FastoObject* item, FastoObject::value_t val) override;

  virtual void onCurrentDataBaseChanged(IDataBaseInfo* info) override;
  virtual void onKeysRemoved(const keys_t& keys) override;
  virtual void onKeysAdded(const keys_value_t& keys) override;
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

class IDriverLocal : public IDriver {
  Q_OBJECT
 public:
  virtual std::string path() const = 0;

 protected:
  explicit IDriverLocal(IConnectionSettingsBaseSPtr settings);
};

class IDriverRemote : public IDriver {
  Q_OBJECT
 public:
  virtual common::net::HostAndPort host() const = 0;

 protected:
  explicit IDriverRemote(IConnectionSettingsBaseSPtr settings);
};

}  // namespace core
}  // namespace fastonosql
