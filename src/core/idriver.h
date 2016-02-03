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

#include <string>

#include <QObject>

#include "common/net/net.h"

#include "core/connection_settings.h"
#include "core/events/events.h"

class QThread;

namespace common {
namespace file_system {
class File;
}
}

namespace fastonosql {

class IDriver
  : public QObject, private IFastoObjectObserver {
  Q_OBJECT
 public:
  IDriver(IConnectionSettingsBaseSPtr settings, connectionTypes type);
  virtual ~IDriver();

  static void reply(QObject* reciver, QEvent* ev);

  // sync methods
  connectionTypes connectionType() const;
  IConnectionSettingsBaseSPtr settings() const;

  ServerDiscoveryInfoSPtr serverDiscoveryInfo() const;
  ServerInfoSPtr serverInfo() const;
  DataBaseInfoSPtr currentDatabaseInfo() const;

  void start();
  void stop();
  common::Error commandByType(CommandKeySPtr command,
                              std::string* cmdstring) const WARN_UNUSED_RESULT;

  virtual void interrupt();
  virtual bool isConnected() const = 0;
  virtual bool isAuthenticated() const = 0;
  virtual common::net::hostAndPort address() const = 0;
  virtual std::string outputDelemitr() const = 0;

 Q_SIGNALS:
  void addedChild(FastoObject* child);
  void itemUpdated(FastoObject* item, common::Value* val);
  void serverInfoSnapShoot(ServerInfoSnapShoot shot);

 private Q_SLOTS:
  void init();
  void clear();

 protected:
  virtual void customEvent(QEvent *event);
  virtual void timerEvent(QTimerEvent* event);

  void notifyProgress(QObject *reciver, int value);

 protected:
  // handle server events
  virtual void handleConnectEvent(events::ConnectRequestEvent* ev) = 0;
  virtual void handleDisconnectEvent(events::DisconnectRequestEvent* ev) = 0;
  virtual void handleExecuteEvent(events::ExecuteRequestEvent* ev) = 0;
  virtual void handleLoadServerInfoEvent(events::ServerInfoRequestEvent* ev) = 0;
  virtual void handleLoadServerPropertyEvent(events::ServerPropertyInfoRequestEvent* ev);
  virtual void handleServerPropertyChangeEvent(events::ChangeServerPropertyInfoRequestEvent* ev);
  virtual void handleShutdownEvent(events::ShutDownRequestEvent* ev);
  virtual void handleBackupEvent(events::BackupRequestEvent* ev);
  virtual void handleExportEvent(events::ExportRequestEvent* ev);
  virtual void handleChangePasswordEvent(events::ChangePasswordRequestEvent* ev);
  virtual void handleChangeMaxConnectionEvent(events::ChangeMaxConnectionRequestEvent* ev);

  // handle database events
  virtual void handleLoadDatabaseInfosEvent(events::LoadDatabasesInfoRequestEvent* ev) = 0;
  virtual void handleLoadDatabaseContentEvent(events::LoadDatabaseContentRequestEvent* ev) = 0;
  virtual void handleSetDefaultDatabaseEvent(events::SetDefaultDatabaseRequestEvent* ev) = 0;

  // handle command events
  virtual void handleCommandRequestEvent(events::CommandRequestEvent* ev) = 0;

  const IConnectionSettingsBaseSPtr settings_;
  bool interrupt_;

  class RootLocker
  {
  public:
    RootLocker(IDriver* parent, QObject* reciver, const std::string& text);
    ~RootLocker();

    FastoObjectIPtr root_;

  private:
    FastoObjectIPtr createRoot(QObject* reciver, const std::string& text);

    IDriver* parent_;
    QObject* reciver_;
    const common::time64_t tstart_;
  };

  RootLocker make_locker(QObject* reciver, const std::string& text) {
    return RootLocker(this, reciver, text);
  }

  void setCurrentDatabaseInfo(DataBaseInfo* inf);

  common::Error execute(FastoObjectCommand* cmd) WARN_UNUSED_RESULT;

 private:
  virtual common::Error executeImpl(FastoObject* out, int argc, char **argv) = 0;

  // handle info events
  void handleLoadServerInfoHistoryEvent(events::ServerInfoHistoryRequestEvent *ev);
  void handleDiscoveryInfoRequestEvent(events::DiscoveryInfoRequestEvent* ev);

  void handleClearServerHistoryRequestEvent(events::ClearServerHistoryRequestEvent *ev);

  // notification of execute events
  virtual void addedChildren(FastoObject *child);
  virtual void updated(FastoObject* item, common::Value* val);

  // internal methods
  virtual ServerInfoSPtr makeServerInfoFromString(const std::string& val) = 0;
  virtual common::Error serverInfo(ServerInfo** info) = 0;
  virtual common::Error serverDiscoveryInfo(ServerInfo** sinfo, ServerDiscoveryInfo** dinfo,
                                            DataBaseInfo** dbinfo) = 0;
  virtual common::Error currentDataBaseInfo(DataBaseInfo** info) = 0;
  virtual void initImpl() = 0;
  virtual void clearImpl() = 0;

  virtual void handleProcessCommandLineArgs(events::ProcessConfigArgsRequestEvent* ev) = 0;

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
  ServerInfoSPtr serverInfo_;
  ServerDiscoveryInfoSPtr serverDiscInfo_;
  DataBaseInfoSPtr currentDatabaseInfo_;

  QThread* thread_;
  int timer_info_id_;
  common::file_system::File* log_file_;
  const connectionTypes type_;
};

}  // namespace fastonosql
