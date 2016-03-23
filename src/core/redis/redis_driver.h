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

#include <vector>
#include <string>

#include "core/idriver.h"

#include "core/redis/redis_raw.h"

namespace fastonosql {
namespace redis {

class RedisDriver
  : public IDriverRemote, public IRedisRawOwner {
 Q_OBJECT
 public:
  explicit RedisDriver(IConnectionSettingsBaseSPtr settings);
  virtual ~RedisDriver();

  virtual bool isConnected() const;
  virtual bool isAuthenticated() const;
  virtual common::net::hostAndPort host() const;
  virtual std::string outputDelemitr() const;

 private:
  virtual bool isInterrupted() const;
  virtual void currentDataBaseChanged(IDataBaseInfo* info);

  virtual void initImpl();
  virtual void clearImpl();

  virtual common::Error executeImpl(int argc, char** argv, FastoObject* out);

  virtual common::Error serverInfo(IServerInfo** info);
  virtual common::Error serverDiscoveryInfo(ServerDiscoveryInfo** dinfo, IServerInfo** sinfo,
                                            IDataBaseInfo** dbinfo);
  virtual common::Error currentDataBaseInfo(IDataBaseInfo** info);

  virtual void handleConnectEvent(events::ConnectRequestEvent* ev);
  virtual void handleDisconnectEvent(events::DisconnectRequestEvent* ev);
  virtual void handleExecuteEvent(events::ExecuteRequestEvent* ev);
  virtual void handleLoadDatabaseInfosEvent(events::LoadDatabasesInfoRequestEvent* ev);
  virtual void handleLoadServerPropertyEvent(events::ServerPropertyInfoRequestEvent* ev);
  virtual void handleServerPropertyChangeEvent(events::ChangeServerPropertyInfoRequestEvent* ev);
  virtual void handleProcessCommandLineArgs(events::ProcessConfigArgsRequestEvent* ev);
  virtual void handleShutdownEvent(events::ShutDownRequestEvent* ev);
  virtual void handleBackupEvent(events::BackupRequestEvent* ev);
  virtual void handleExportEvent(events::ExportRequestEvent* ev);
  virtual void handleChangePasswordEvent(events::ChangePasswordRequestEvent* ev);
  virtual void handleChangeMaxConnectionEvent(events::ChangeMaxConnectionRequestEvent* ev);

  virtual common::Error commandDeleteImpl(CommandDeleteKey* command,
                                          std::string* cmdstring) const WARN_UNUSED_RESULT;
  virtual common::Error commandLoadImpl(CommandLoadKey* command,
                                        std::string* cmdstring) const WARN_UNUSED_RESULT;
  virtual common::Error commandCreateImpl(CommandCreateKey* command,
                                          std::string* cmdstring) const WARN_UNUSED_RESULT;
  virtual common::Error commandChangeTTLImpl(CommandChangeTTL* command,
                                             std::string* cmdstring) const WARN_UNUSED_RESULT;

  virtual void handleLoadDatabaseContentEvent(events::LoadDatabaseContentRequestEvent* ev);
  virtual void handleClearDatabaseEvent(events::ClearDatabaseRequestEvent* ev);
  virtual void handleSetDefaultDatabaseEvent(events::SetDefaultDatabaseRequestEvent* ev);

  virtual void handleCommandRequestEvent(events::CommandRequestEvent* ev);

  IServerInfoSPtr makeServerInfoFromString(const std::string& val);

  RedisRaw* const impl_;

  common::Error interacteveMode(events::ProcessConfigArgsRequestEvent* ev);
  common::Error latencyMode(events::ProcessConfigArgsRequestEvent* ev);
  common::Error slaveMode(events::ProcessConfigArgsRequestEvent* ev);
  common::Error getRDBMode(events::ProcessConfigArgsRequestEvent* ev);
  common::Error findBigKeysMode(events::ProcessConfigArgsRequestEvent* ev);
  common::Error statMode(events::ProcessConfigArgsRequestEvent* ev);
  common::Error scanMode(events::ProcessConfigArgsRequestEvent* ev);
};

}  // namespace redis
}  // namespace fastonosql
