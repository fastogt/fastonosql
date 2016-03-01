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

#include <string>

#include "core/idriver.h"
#include "core/leveldb/leveldb_raw.h"

namespace fastonosql {
namespace leveldb {

class LeveldbDriver
      : public IDriverLocal {
  Q_OBJECT
 public:
  explicit LeveldbDriver(IConnectionSettingsBaseSPtr settings);
  virtual ~LeveldbDriver();

  virtual bool isConnected() const;
  virtual bool isAuthenticated() const;
  virtual std::string path() const;
  virtual std::string outputDelemitr() const;

 private:
  virtual void initImpl();
  virtual void clearImpl();

  virtual common::Error executeImpl(int argc, char **argv, FastoObject* out);
  virtual common::Error serverInfo(IServerInfo** info);
  virtual common::Error serverDiscoveryInfo(ServerDiscoveryInfo** dinfo, IServerInfo** sinfo,
                                            IDataBaseInfo** dbinfo);
  virtual common::Error currentDataBaseInfo(IDataBaseInfo** info);

  virtual void handleConnectEvent(events::ConnectRequestEvent* ev);
  virtual void handleDisconnectEvent(events::DisconnectRequestEvent* ev);
  virtual void handleExecuteEvent(events::ExecuteRequestEvent* ev);
  virtual void handleLoadDatabaseInfosEvent(events::LoadDatabasesInfoRequestEvent* ev);
  virtual void handleLoadServerInfoEvent(events::ServerInfoRequestEvent* ev);
  virtual void handleProcessCommandLineArgs(events::ProcessConfigArgsRequestEvent* ev);

  // ============== commands =============//
  virtual common::Error commandDeleteImpl(CommandDeleteKey* command,
                                          std::string* cmdstring) const WARN_UNUSED_RESULT;
  virtual common::Error commandLoadImpl(CommandLoadKey* command,
                                        std::string* cmdstring) const WARN_UNUSED_RESULT;
  virtual common::Error commandCreateImpl(CommandCreateKey* command,
                                          std::string* cmdstring) const WARN_UNUSED_RESULT;
  virtual common::Error commandChangeTTLImpl(CommandChangeTTL* command,
                                             std::string* cmdstring) const WARN_UNUSED_RESULT;
  // ============== commands =============//

  // ============== database =============//
  virtual void handleLoadDatabaseContentEvent(events::LoadDatabaseContentRequestEvent* ev);
  virtual void handleSetDefaultDatabaseEvent(events::SetDefaultDatabaseRequestEvent* ev);
  // ============== database =============//
  // ============== command =============//
  virtual void handleCommandRequestEvent(events::CommandRequestEvent* ev);
  // ============== command =============//
  IServerInfoSPtr makeServerInfoFromString(const std::string& val);

  LeveldbRaw* const impl_;
};

}  // namespace leveldb
}  // namespace fastonosql
