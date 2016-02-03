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

#include "core/idriver.h"

#include "core/leveldb/leveldb_settings.h"

namespace fastonosql {

static const CommandInfo leveldbCommands[] = {
  CommandInfo("PUT", "<key> <value>",
              "Set the value of a key.",
              UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("GET", "<key>",
              "Get the value of a key.",
              UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("DEL", "<key>",
              "Delete key.",
              UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("KEYS", "<key_start> <key_end> <limit>",
              "Find all keys matching the given limits.",
              UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 3, 0),
  CommandInfo("INFO", "<args>",
              "These command return database information.",
              UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("QUIT", "-",
              "Close the connection.",
              UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 0, 0),
  //======= extended =======//
  CommandInfo("INTERRUPT", "-",
              "Command execution interrupt",
              UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("DBSIZE", "-",
              "Return the number of keys in the selected database",
              UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 0, 0)
  //======= extended =======//
};

common::Error testConnection(LeveldbConnectionSettings* settings);

class LeveldbDriver
      : public IDriver {
  Q_OBJECT
 public:
  explicit LeveldbDriver(IConnectionSettingsBaseSPtr settings);
  virtual ~LeveldbDriver();

  virtual bool isConnected() const;
  virtual bool isAuthenticated() const;
  common::net::hostAndPort address() const;
  virtual std::string outputDelemitr() const;

  static const char* versionApi();

 private:
  virtual void initImpl();
  virtual void clearImpl();

  virtual common::Error executeImpl(FastoObject* out, int argc, char **argv);
  virtual common::Error serverInfo(ServerInfo** info);
  virtual common::Error serverDiscoveryInfo(ServerInfo** sinfo, ServerDiscoveryInfo** dinfo,
                                            DataBaseInfo** dbinfo);
  virtual common::Error currentDataBaseInfo(DataBaseInfo** info);

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
  ServerInfoSPtr makeServerInfoFromString(const std::string& val);

  struct pimpl;
  pimpl* const impl_;
};

}  // namespace fastonosql
