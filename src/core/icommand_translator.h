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

#include "core/command_holder.h"
#include "core/db_key.h"  // for NKey, NDbKValue, ttl_t
#include "core/db_ps_channel.h"

#define DB_FLUSHDB_COMMAND "FLUSHDB"    // exist for all
#define DB_SELECTDB_COMMAND "SELECT"    // exist for all
#define DB_INFO_COMMAND "INFO"          // exist for all
#define DB_HELP_COMMAND "HELP"          // exist for all
#define DB_DBKCOUNT_COMMAND "DBKCOUNT"  // exist for all
#define DB_QUIT_COMMAND "QUIT"          // exist for all

#define DB_SET_TTL_COMMAND "EXPIRE"
#define DB_GET_TTL_COMMAND "TTL"

#define DB_CREATEDB_COMMAND "CREATEDB"
#define DB_REMOVEDB_COMMAND "REMOVEDB"

#define DB_PUBLISH_COMMAND "PUBLISH"
#define DB_SUBSCRIBE_COMMAND "SUBSCRIBE"

#define DB_GET_KEY_COMMAND "GET"        // exist for all
#define DB_SET_KEY_COMMAND "SET"        // exist for all
#define DB_DELETE_KEY_COMMAND "DEL"     // exist for all
#define DB_RENAME_KEY_COMMAND "RENAME"  // exist for all
#define DB_KEYS_COMMAND "KEYS"          // exist for all
#define DB_SCAN_COMMAND "SCAN"          // exist for all

#define DB_GET_CONFIG_COMMAND "CONFIG GET"
#define DB_GET_DATABASES_COMMAND DB_GET_CONFIG_COMMAND " databases"

namespace fastonosql {
namespace core {

// GET alex\nSET alex
// should return vector of 2 commands {{"GET","alex"}, {"SET", "alex"}}
common::Error ParseCommands(const command_buffer_t& cmd, std::vector<command_buffer_t>* cmds);

class ICommandTranslator {
 public:
  explicit ICommandTranslator(const std::vector<CommandHolder>& commands);
  virtual ~ICommandTranslator();

  virtual const char* GetDBName() const = 0;

  common::Error GetDatabasesCommand(command_buffer_t* cmdstring) const WARN_UNUSED_RESULT;
  common::Error RemoveDBCommand(const std::string& name, command_buffer_t* cmdstring) const WARN_UNUSED_RESULT;
  common::Error CreateDBCommand(const std::string& name, command_buffer_t* cmdstring) const WARN_UNUSED_RESULT;

  common::Error SelectDBCommand(const std::string& name, command_buffer_t* cmdstring) const WARN_UNUSED_RESULT;
  common::Error FlushDBCommand(command_buffer_t* cmdstring) const WARN_UNUSED_RESULT;
  common::Error CreateKeyCommand(const NDbKValue& key, command_buffer_t* cmdstring) const WARN_UNUSED_RESULT;
  common::Error LoadKeyCommand(const NKey& key,
                               common::Value::Type type,
                               command_buffer_t* cmdstring) const WARN_UNUSED_RESULT;
  common::Error DeleteKeyCommand(const NKey& key, command_buffer_t* cmdstring) const WARN_UNUSED_RESULT;
  common::Error RenameKeyCommand(const NKey& key,
                                 const key_t& new_name,
                                 command_buffer_t* cmdstring) const WARN_UNUSED_RESULT;
  common::Error ChangeKeyTTLCommand(const NKey& key, ttl_t ttl, command_buffer_t* cmdstring) const WARN_UNUSED_RESULT;
  common::Error LoadKeyTTLCommand(const NKey& key, command_buffer_t* cmdstring) const WARN_UNUSED_RESULT;

  bool IsLoadKeyCommand(const command_buffer_t& cmd, string_key_t* key) const WARN_UNUSED_RESULT;

  common::Error PublishCommand(const NDbPSChannel& channel,
                               const std::string& message,
                               command_buffer_t* cmdstring) const WARN_UNUSED_RESULT;

  common::Error SubscribeCommand(const NDbPSChannel& channel, command_buffer_t* cmdstring) const WARN_UNUSED_RESULT;

  std::vector<CommandInfo> Commands() const;
  common::Error FindCommand(const std::string& command_first_name, const CommandHolder** info) const WARN_UNUSED_RESULT;
  common::Error FindCommand(commands_args_t argv, const CommandHolder** info, size_t* off) const WARN_UNUSED_RESULT;

  common::Error TestCommandArgs(const CommandHolder* cmd, commands_args_t argv) const WARN_UNUSED_RESULT;
  common::Error TestCommandLine(const command_buffer_t& cmd) const WARN_UNUSED_RESULT;
  common::Error TestCommandLineArgs(commands_args_t argv,
                                    const CommandHolder** info,
                                    size_t* off) const WARN_UNUSED_RESULT;

  static common::Error InvalidInputArguments(const std::string& cmd);
  static common::Error NotSupported(const std::string& cmd);
  static common::Error UnknownCommand(const std::string& cmd);
  static common::Error UnknownSequence(commands_args_t argv);

 private:
  virtual common::Error CreateKeyCommandImpl(const NDbKValue& key, command_buffer_t* cmdstring) const = 0;
  virtual common::Error LoadKeyCommandImpl(const NKey& key,
                                           common::Value::Type type,
                                           command_buffer_t* cmdstring) const = 0;
  virtual common::Error DeleteKeyCommandImpl(const NKey& key, command_buffer_t* cmdstring) const = 0;
  virtual common::Error RenameKeyCommandImpl(const NKey& key,
                                             const key_t& new_name,
                                             command_buffer_t* cmdstring) const = 0;
  virtual common::Error ChangeKeyTTLCommandImpl(const NKey& key, ttl_t ttl, command_buffer_t* cmdstring) const = 0;
  virtual common::Error LoadKeyTTLCommandImpl(const NKey& key, command_buffer_t* cmdstring) const = 0;
  virtual common::Error PublishCommandImpl(const NDbPSChannel& channel,
                                           const std::string& message,
                                           command_buffer_t* cmdstring) const = 0;
  virtual common::Error SubscribeCommandImpl(const NDbPSChannel& channel, command_buffer_t* cmdstring) const = 0;

  virtual bool IsLoadKeyCommandImpl(const CommandInfo& cmd) const = 0;

  const std::vector<CommandHolder> commands_;
};

typedef std::shared_ptr<ICommandTranslator> translator_t;

}  // namespace core
}  // namespace fastonosql
