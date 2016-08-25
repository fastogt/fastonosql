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

#include "common/error.h"               // for Error
#include "common/macros.h"              // for WARN_UNUSED_RESULT
#include "common/net/types.h"           // for HostAndPort
#include "common/types.h"               // for time64_t
#include "core/command_key.h"           // for CommandChangeTTL (ptr only), etc
#include "core/connection_types.h"      // for connectionTypes
#include "core/connection_settings.h"
#include "core/types.h"                 // for IDataBaseInfo (ptr only), etc

#include "core/events/events.h"

#include "global/global.h"              // for FastoObject, etc

class QThread;

namespace common {
namespace file_system {
class File;
}
}

namespace fastonosql {
namespace core {

class IDriver
  : public QObject, private FastoObject::IFastoObjectObserver {
  Q_OBJECT
 public:
  virtual ~IDriver();

  static void reply(QObject* reciver, QEvent* ev);

  // sync methods
  connectionTypes type() const;
  IConnectionSettings::connection_path_t connectionPath() const;

  IServerInfoSPtr serverInfo() const;
  IDataBaseInfoSPtr currentDatabaseInfo() const;

  void start();
  void stop();
  common::Error commandByType(CommandKeySPtr command,
                              std::string* cmdstring) const WARN_UNUSED_RESULT;

  void interrupt();
  bool isInterrupted() const;

  virtual bool isConnected() const = 0;
  virtual bool isAuthenticated() const = 0;

  virtual std::string delimiter() const = 0;
  virtual std::string nsSeparator() const = 0;

 Q_SIGNALS:
  void addedChild(FastoObject* child);
  void itemUpdated(FastoObject* item, FastoObject::value_t val);
  void serverInfoSnapShoot(ServerInfoSnapShoot shot);

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
  virtual void handleExecuteEvent(events::ExecuteRequestEvent* ev) = 0;
  virtual void handleCommandRequestEvent(events::CommandRequestEvent* ev) = 0;

  virtual void handleLoadDatabaseContentEvent(events::LoadDatabaseContentRequestEvent* ev) = 0;

  virtual void handleLoadServerPropertyEvent(events::ServerPropertyInfoRequestEvent* ev);
  virtual void handleServerPropertyChangeEvent(events::ChangeServerPropertyInfoRequestEvent* ev);
  virtual void handleShutdownEvent(events::ShutDownRequestEvent* ev);
  virtual void handleBackupEvent(events::BackupRequestEvent* ev);
  virtual void handleExportEvent(events::ExportRequestEvent* ev);
  virtual void handleChangePasswordEvent(events::ChangePasswordRequestEvent* ev);
  virtual void handleChangeMaxConnectionEvent(events::ChangeMaxConnectionRequestEvent* ev);
  virtual void handleLoadDatabaseInfosEvent(events::LoadDatabasesInfoRequestEvent* ev);  // call currentDatabaseInfo
  virtual void handleClearDatabaseEvent(events::ClearDatabaseRequestEvent* ev);
  virtual void handleSetDefaultDatabaseEvent(events::SetDefaultDatabaseRequestEvent* ev);

  const IConnectionSettingsBaseSPtr settings_;

  class RootLocker {
  public:
    RootLocker(IDriver* parent, QObject* receiver, const std::string& text);
    ~RootLocker();

    FastoObjectIPtr root() const;

  private:
    FastoObjectIPtr root_;
    IDriver* parent_;
    QObject* receiver_;
    const common::time64_t tstart_;
  };

  RootLocker make_locker(QObject* reciver, const std::string& text) {
    return RootLocker(this, reciver, text);
  }

  void setCurrentDatabaseInfo(IDataBaseInfo* inf);

  common::Error execute(FastoObjectCommand* cmd) WARN_UNUSED_RESULT;
 private:
  virtual common::Error syncConnect() WARN_UNUSED_RESULT = 0;
  virtual common::Error syncDisconnect() WARN_UNUSED_RESULT = 0;
  void handleLoadServerInfoEvent(events::ServerInfoRequestEvent* ev);  // call serverInfo
  void handleLoadServerInfoHistoryEvent(events::ServerInfoHistoryRequestEvent* ev);
  void handleDiscoveryInfoRequestEvent(events::DiscoveryInfoRequestEvent* ev);
  void handleClearServerHistoryRequestEvent(events::ClearServerHistoryRequestEvent* ev);

  virtual common::Error executeImpl(int argc, char** argv, FastoObject* out) = 0;

  // notification of execute events
  virtual void addedChildren(FastoObject* child);
  virtual void updated(FastoObject* item, FastoObject::value_t val);

  // internal methods
  virtual IServerInfoSPtr makeServerInfoFromString(const std::string& val) = 0;
  virtual common::Error serverInfo(IServerInfo** info) = 0;
  virtual common::Error serverDiscoveryInfo(IServerInfo** sinfo, IDataBaseInfo** dbinfo);
  virtual common::Error currentDataBaseInfo(IDataBaseInfo** info) = 0;
  virtual void initImpl() = 0;
  virtual void clearImpl() = 0;

  // command impl methods
  virtual common::Error commandDeleteImpl(CommandDeleteKey* command,
                                          std::string* cmdstring) const WARN_UNUSED_RESULT = 0;
  virtual common::Error commandLoadImpl(CommandLoadKey* command,
                                        std::string* cmdstring) const WARN_UNUSED_RESULT = 0;
  virtual common::Error commandCreateImpl(CommandCreateKey* command,
                                          std::string* cmdstring) const WARN_UNUSED_RESULT = 0;
  virtual common::Error commandChangeTTLImpl(CommandChangeTTL* command,
                                             std::string* cmdstring) const WARN_UNUSED_RESULT = 0;

 private:
  bool interrupt_;
  IServerInfoSPtr server_info_;
  IDataBaseInfoSPtr current_database_info_;

  QThread* thread_;
  int timer_info_id_;
  common::file_system::File* log_file_;
};

class IDriverLocal
  : public IDriver {
  Q_OBJECT
 public:
  virtual std::string path() const = 0;

 protected:
  explicit IDriverLocal(IConnectionSettingsBaseSPtr settings);
};

class IDriverRemote
  : public IDriver {
  Q_OBJECT
 public:
  virtual common::net::HostAndPort host() const = 0;

 protected:
  explicit IDriverRemote(IConnectionSettingsBaseSPtr settings);
};

}  // namespace core
}  // namespace fastonosql
