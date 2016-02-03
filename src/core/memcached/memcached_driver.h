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

#include "core/memcached/memcached_settings.h"

namespace fastonosql {

// TODO: cas command implementation
static const CommandInfo memcachedCommands[] = {
  CommandInfo("QUIT", "-",
              "Close the connection.", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("VERBOSITY", "<level>",
              "Change the verbosity ouptut of Memcached server.", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0),
  CommandInfo("VERSION", "-",
              "Return the Memcached server version.", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("STATS", "[<args>]",
              "These command can return various stats that we will explain.", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 0, 1),
  CommandInfo("FLUSH_ALL", "[<time>]",
              "Flush the server key/value pair (invalidating them) after a optional [<time>] period.\n"
              "It always return OK", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 0, 1),
  CommandInfo("DELETE", "<key> [<time>]",
              "Delete key/value pair in Memcached", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 1),
  CommandInfo("INCR", "<key> <value>",
              "Increment value associated with key in Memcached, item must exist, increment command will not create it.\n"
              "The limit of increment is the 64 bit mark.", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("INTERRUPT", "-",
              "Command execution interrupt",
              UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 0, 0),
  CommandInfo("DECR", "<key> <value>",
              "Decrement value associated with key in Memcached, item must exist, decrement command will not create it.", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 2, 0),
  CommandInfo("PREPEND", "<key> <flags> <exptime> <bytes>",
              "Add value to an existing key before existing data.\n"
              "Prepend does not take <flags> or <exptime> parameters but you must provide them!", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 4, 0),
  CommandInfo("APPEND", "<key> <flags> <exptime> <value>",
              "Add value to an existing key after existing data.\n"
              "Append does not take <flags> or <exptime> parameters but you must provide them!", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 4, 0),
  CommandInfo("REPLACE", "<key> <flags> <exptime> <value>",
              "Store key/value pair in Memcached, but only if the server already hold data for this key.", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 4, 0),
  CommandInfo("ADD", "<key> <flags> <exptime> <value>",
              "Store key/value pair in Memcached, but only if the server doesn't already hold data for this key.", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 4, 0),
  CommandInfo("SET", "<key> <flags> <exptime> <value>",
              "Set the string value of a key.", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 4, 0),
  CommandInfo("GET", "<key>",
              "Get the value of a key.", UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0)
};

common::Error testConnection(MemcachedConnectionSettings* settings);

class MemcachedDriver
  : public IDriver {
  Q_OBJECT
 public:
  explicit MemcachedDriver(IConnectionSettingsBaseSPtr settings);
  virtual ~MemcachedDriver();

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
