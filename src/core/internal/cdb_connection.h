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

#include <common/sprintf.h>

#include "core/connection_commands_traits.h"
#include "core/internal/cdb_connection_client.h"
#include "core/internal/command_handler.h"  // for CommandHandler, etc
#include "core/internal/db_connection.h"    // for DBConnection

#include "core/database/idatabase_info.h"

namespace fastonosql {
namespace core {
namespace internal {

command_buffer_t GetKeysPattern(uint64_t cursor_in, const std::string& pattern, uint64_t count_keys);  // for SCAN

// for all commands:
// 1) test input
// 2) test connection state
// 3) exec inner api command, return value via CheckResultCommand

// methods naming
// voi CreateDB() => void OnCreatedDB()

template <typename NConnection, typename Config, connectionTypes connection_type>
class CDBConnection : public DBConnection<NConnection, Config, connection_type>,
                      public CommandHandler,
                      public ConnectionCommandsTraits<connection_type> {
 public:
  typedef DBConnection<NConnection, Config, connection_type> db_base_class;
  typedef ConnectionCommandsTraits<connection_type> connection_traits_class;

  CDBConnection(CDBConnectionClient* client, ICommandTranslator* translator)
      : db_base_class(), CommandHandler(translator), client_(client) {}
  virtual ~CDBConnection() {}

  virtual std::string GetCurrentDBName() const;                                      //
  common::Error Help(commands_args_t argv, std::string* answer) WARN_UNUSED_RESULT;  //

  common::Error Scan(uint64_t cursor_in,
                     const std::string& pattern,
                     uint64_t count_keys,
                     std::vector<std::string>* keys_out,
                     uint64_t* cursor_out) WARN_UNUSED_RESULT;  // nvi
  common::Error Keys(const std::string& key_start,
                     const std::string& key_end,
                     uint64_t limit,
                     std::vector<std::string>* ret) WARN_UNUSED_RESULT;                    // nvi
  common::Error DBkcount(size_t* size) WARN_UNUSED_RESULT;                                 // nvi
  common::Error FlushDB() WARN_UNUSED_RESULT;                                              // nvi
  common::Error Select(const std::string& name, IDataBaseInfo** info) WARN_UNUSED_RESULT;  // nvi
  common::Error CreateDB(const std::string& name) WARN_UNUSED_RESULT;                      // nvi
  common::Error RemoveDB(const std::string& name) WARN_UNUSED_RESULT;                      // nvi
  common::Error ConfigGetDatabases(std::vector<std::string>* dbs) WARN_UNUSED_RESULT;      // nvi
  common::Error Delete(const NKeys& keys, NKeys* deleted_keys) WARN_UNUSED_RESULT;         // nvi
  common::Error Set(const NDbKValue& key, NDbKValue* added_key) WARN_UNUSED_RESULT;        // nvi
  common::Error Get(const NKey& key, NDbKValue* loaded_key) WARN_UNUSED_RESULT;            // nvi
  common::Error Rename(const NKey& key, const string_key_t& new_key) WARN_UNUSED_RESULT;   // nvi
  common::Error SetTTL(const NKey& key, ttl_t ttl) WARN_UNUSED_RESULT;                     // nvi
  common::Error GetTTL(const NKey& key, ttl_t* ttl) WARN_UNUSED_RESULT;                    // nvi
  common::Error ModuleLoad(const ModuleInfo& module) WARN_UNUSED_RESULT;                   // nvi
  common::Error ModuleUnLoad(const ModuleInfo& module) WARN_UNUSED_RESULT;                 // nvi
  common::Error Quit() WARN_UNUSED_RESULT;                                                 // nvi

 protected:
  common::Error GenerateError(const std::string& cmd, const std::string& descr) WARN_UNUSED_RESULT {
    const std::string buff = common::MemSPrintf("%s function error: %s", cmd, descr);
    return common::make_error(buff);
  }
  CDBConnectionClient* client_;

 private:
  virtual common::Error ScanImpl(uint64_t cursor_in,
                                 const std::string& pattern,
                                 uint64_t count_keys,
                                 std::vector<std::string>* keys_out,
                                 uint64_t* cursor_out) = 0;
  virtual common::Error KeysImpl(const std::string& key_start,
                                 const std::string& key_end,
                                 uint64_t limit,
                                 std::vector<std::string>* ret) = 0;
  virtual common::Error DBkcountImpl(size_t* size) = 0;
  virtual common::Error FlushDBImpl() = 0;

  virtual common::Error SelectImpl(const std::string& name, IDataBaseInfo** info) = 0;
  virtual common::Error CreateDBImpl(const std::string& name, IDataBaseInfo** info);  // optional
  virtual common::Error RemoveDBImpl(const std::string& name, IDataBaseInfo** info);  // optional

  virtual common::Error ConfigGetDatabasesImpl(std::vector<std::string>* dbs) = 0;
  virtual common::Error DeleteImpl(const NKeys& keys, NKeys* deleted_keys) = 0;
  virtual common::Error SetImpl(const NDbKValue& key, NDbKValue* added_key) = 0;
  virtual common::Error GetImpl(const NKey& key, NDbKValue* loaded_key) = 0;
  virtual common::Error RenameImpl(const NKey& key, string_key_t new_key) = 0;
  virtual common::Error SetTTLImpl(const NKey& key, ttl_t ttl);      // optional
  virtual common::Error GetTTLImpl(const NKey& key, ttl_t* ttl);     // optional
  virtual common::Error ModuleLoadImpl(const ModuleInfo& module);    // optional
  virtual common::Error ModuleUnLoadImpl(const ModuleInfo& module);  // optional
  virtual common::Error QuitImpl() = 0;
};

template <typename NConnection, typename Config, connectionTypes ContType>
std::string CDBConnection<NConnection, Config, ContType>::GetCurrentDBName() const {
  return "default";
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::Help(commands_args_t argv, std::string* answer) {
  size_t argc = argv.size();
  if (!answer) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  if (argc == 0) {
    *answer = common::MemSPrintf(PROJECT_NAME_TITLE
                                 " based on %s %s \n"
                                 "Type: \"help <command>\" for help on <command>\n"
                                 "\"help " ALL_COMMANDS "\" show all supported commands",
                                 connection_traits_class::GetBasedOn(), connection_traits_class::GetVersionApi());

    return common::Error();
  }

  translator_t tran = GetTranslator();
  if (argc == 1 && argv[0] == ALL_COMMANDS) {
    std::vector<CommandInfo> cmds = tran->Commands();
    for (size_t i = 0; i < cmds.size(); ++i) {
      *answer += cmds[i].name;
      if (i != cmds.size() - 1) {
        *answer += "\n";
      }
    }
    return common::Error();
  }

  const CommandHolder* cmd = nullptr;
  size_t off = 0;
  common::Error err = tran->FindCommand(argv, &cmd, &off);
  if (err) {
    return err;
  }

  *answer = common::MemSPrintf(
      "name: %s\n"
      "summary: %s\n"
      "params: %s\n"
      "since: %s\n"
      "example: %s",
      cmd->name, cmd->summary, cmd->params, ConvertVersionNumberToReadableString(cmd->since), cmd->example);
  return common::Error();
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::Scan(uint64_t cursor_in,
                                                                 const std::string& pattern,
                                                                 uint64_t count_keys,
                                                                 std::vector<std::string>* keys_out,
                                                                 uint64_t* cursor_out) {
  if (!keys_out || !cursor_out) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = CDBConnection<NConnection, Config, ContType>::TestIsAuthenticated();
  if (err) {
    return err;
  }

  err = ScanImpl(cursor_in, pattern, count_keys, keys_out, cursor_out);
  if (err) {
    return err;
  }

  return common::Error();
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::Keys(const std::string& key_start,
                                                                 const std::string& key_end,
                                                                 uint64_t limit,
                                                                 std::vector<std::string>* ret) {
  if (!ret) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = CDBConnection<NConnection, Config, ContType>::TestIsAuthenticated();
  if (err) {
    return err;
  }

  err = KeysImpl(key_start, key_end, limit, ret);
  if (err) {
    return err;
  }

  return common::Error();
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::DBkcount(size_t* size) {
  if (!size) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = CDBConnection<NConnection, Config, ContType>::TestIsAuthenticated();
  if (err) {
    return err;
  }

  err = DBkcountImpl(size);
  if (err) {
    return err;
  }

  return common::Error();
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::FlushDB() {
  common::Error err = CDBConnection<NConnection, Config, ContType>::TestIsAuthenticated();
  if (err) {
    return err;
  }

  err = FlushDBImpl();
  if (err) {
    return err;
  }

  if (client_) {
    client_->OnFlushedCurrentDB();
  }

  return common::Error();
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::CreateDB(const std::string& name) {
  if (name.empty()) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  if (name == GetCurrentDBName()) {
    DNOTREACHED();
    return common::make_error("Sorry we can't create selected database.");
  }

  common::Error err = CDBConnection<NConnection, Config, ContType>::TestIsAuthenticated();
  if (err) {
    return err;
  }

  IDataBaseInfo* linfo = nullptr;
  err = CreateDBImpl(name, &linfo);
  if (err) {
    return err;
  }

  if (client_) {
    client_->OnCreatedDB(linfo);
  }

  delete linfo;
  return common::Error();
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::RemoveDB(const std::string& name) {
  if (name.empty()) {
    DNOTREACHED() << "Invalid database name.";
    return common::make_error_inval();
  }

  if (name == GetCurrentDBName()) {
    const std::string error_str = "Sorry we can't remove selected database.";
    DNOTREACHED() << error_str;
    return common::make_error(error_str);
  }

  common::Error err = CDBConnection<NConnection, Config, ContType>::TestIsAuthenticated();
  if (err) {
    return err;
  }

  IDataBaseInfo* linfo = nullptr;
  err = RemoveDBImpl(name, &linfo);
  if (err) {
    return err;
  }

  if (client_) {
    client_->OnRemovedDB(linfo);
  }

  delete linfo;
  return common::Error();
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::ConfigGetDatabases(std::vector<std::string>* dbs) {
  common::Error err = CDBConnection<NConnection, Config, ContType>::TestIsAuthenticated();
  if (err) {
    return err;
  }

  std::vector<std::string> ldbs;
  err = ConfigGetDatabasesImpl(&ldbs);
  if (err) {
    return err;
  }

  *dbs = ldbs;
  return common::Error();
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::Select(const std::string& name, IDataBaseInfo** info) {
  common::Error err = CDBConnection<NConnection, Config, ContType>::TestIsAuthenticated();
  if (err) {
    return err;
  }

  IDataBaseInfo* linfo = nullptr;
  err = SelectImpl(name, &linfo);
  if (err) {
    return err;
  }

  if (client_) {
    client_->OnChangedCurrentDB(linfo);
  }

  if (info) {
    *info = linfo;
  } else {
    delete linfo;
  }

  return common::Error();
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::Delete(const NKeys& keys, NKeys* deleted_keys) {
  if (!deleted_keys) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = CDBConnection<NConnection, Config, ContType>::TestIsAuthenticated();
  if (err) {
    return err;
  }

  err = DeleteImpl(keys, deleted_keys);
  if (err) {
    return err;
  }

  if (client_) {
    client_->OnRemovedKeys(*deleted_keys);
  }

  return common::Error();
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::Set(const NDbKValue& key, NDbKValue* added_key) {
  if (!added_key) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = CDBConnection<NConnection, Config, ContType>::TestIsAuthenticated();
  if (err) {
    return err;
  }

  err = SetImpl(key, added_key);
  if (err) {
    return err;
  }

  if (client_) {
    client_->OnAddedKey(*added_key);
  }

  return common::Error();
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::Get(const NKey& key, NDbKValue* loaded_key) {
  if (!loaded_key) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = CDBConnection<NConnection, Config, ContType>::TestIsAuthenticated();
  if (err) {
    return err;
  }

  err = GetImpl(key, loaded_key);
  if (err) {
    return err;
  }

  if (client_) {
    client_->OnLoadedKey(*loaded_key);
  }

  return common::Error();
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::Rename(const NKey& key, const string_key_t& new_key) {
  common::Error err = CDBConnection<NConnection, Config, ContType>::TestIsAuthenticated();
  if (err) {
    return err;
  }

  err = RenameImpl(key, new_key);
  if (err) {
    return err;
  }

  if (client_) {
    client_->OnRenamedKey(key, new_key);
  }

  return common::Error();
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::SetTTL(const NKey& key, ttl_t ttl) {
  common::Error err = CDBConnection<NConnection, Config, ContType>::TestIsAuthenticated();
  if (err) {
    return err;
  }

  err = SetTTLImpl(key, ttl);
  if (err) {
    return err;
  }

  if (client_) {
    client_->OnChangedKeyTTL(key, ttl);
  }

  return common::Error();
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::GetTTL(const NKey& key, ttl_t* ttl) {
  if (!ttl) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = CDBConnection<NConnection, Config, ContType>::TestIsAuthenticated();
  if (err) {
    return err;
  }

  err = GetTTLImpl(key, ttl);
  if (err) {
    return err;
  }

  if (client_) {
    client_->OnLoadedKeyTTL(key, *ttl);
  }

  return common::Error();
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::ModuleLoad(const ModuleInfo& module) {
  common::Error err = CDBConnection<NConnection, Config, ContType>::TestIsAuthenticated();
  if (err) {
    return err;
  }

  err = ModuleLoadImpl(module);
  if (err) {
    return err;
  }

  if (client_) {
    client_->OnLoadedModule(module);
  }

  return common::Error();
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::ModuleUnLoad(const ModuleInfo& module) {
  common::Error err = CDBConnection<NConnection, Config, ContType>::TestIsAuthenticated();
  if (err) {
    return err;
  }

  err = ModuleUnLoadImpl(module);
  if (err) {
    return err;
  }

  if (client_) {
    client_->OnUnLoadedModule(module);
  }

  return common::Error();
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::Quit() {
  common::Error err = CDBConnection<NConnection, Config, ContType>::TestIsAuthenticated();
  if (err) {
    return err;
  }

  err = QuitImpl();
  if (err) {
    return err;
  }

  if (client_) {
    client_->OnQuited();
  }

  return common::Error();
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::SetTTLImpl(const NKey& key, ttl_t ttl) {
  UNUSED(key);
  UNUSED(ttl);
  const std::string error_msg = common::MemSPrintf(
      "Sorry, but now " PROJECT_NAME_TITLE " for %s not supported TTL commands.", connection_traits_class::GetDBName());
  return common::make_error(error_msg);
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::GetTTLImpl(const NKey& key, ttl_t* ttl) {
  UNUSED(key);
  UNUSED(ttl);
  const std::string error_msg = common::MemSPrintf(
      "Sorry, but now " PROJECT_NAME_TITLE " for %s not supported TTL commands.", connection_traits_class::GetDBName());
  return common::make_error(error_msg);
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::ModuleLoadImpl(const ModuleInfo& module) {
  UNUSED(module);
  const std::string error_msg =
      common::MemSPrintf("Sorry, but now " PROJECT_NAME_TITLE " for %s not supported load module commands.",
                         connection_traits_class::GetDBName());
  return common::make_error(error_msg);
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::ModuleUnLoadImpl(const ModuleInfo& module) {
  UNUSED(module);
  const std::string error_msg =
      common::MemSPrintf("Sorry, but now " PROJECT_NAME_TITLE " for %s not supported unload module commands.",
                         connection_traits_class::GetDBName());
  return common::make_error(error_msg);
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::RemoveDBImpl(const std::string& name,
                                                                         IDataBaseInfo** info) {
  UNUSED(name);
  UNUSED(info);
  const std::string error_msg =
      common::MemSPrintf("Sorry, but now " PROJECT_NAME_TITLE " for %s not supported " DB_REMOVEDB_COMMAND " commands.",
                         connection_traits_class::GetDBName());
  return common::make_error(error_msg);
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::CreateDBImpl(const std::string& name,
                                                                         IDataBaseInfo** info) {
  UNUSED(name);
  UNUSED(info);
  const std::string error_msg =
      common::MemSPrintf("Sorry, but now " PROJECT_NAME_TITLE " for %s not supported " DB_CREATEDB_COMMAND " commands.",
                         connection_traits_class::GetDBName());
  return common::make_error(error_msg);
}

}  // namespace internal
}  // namespace core
}  // namespace fastonosql
