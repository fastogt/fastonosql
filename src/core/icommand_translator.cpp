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

#include "core/icommand_translator.h"

extern "C" {
#include "sds.h"
}

#include <common/convert2string.h>
#include <common/sprintf.h>

namespace fastonosql {
namespace core {

common::Error ParseCommands(const command_buffer_t& cmd, std::vector<command_buffer_t>* cmds) {
  if (cmd.empty()) {
    return common::make_error("Empty command line.");
  }

  std::vector<command_buffer_t> commands;
  size_t commands_count = common::Tokenize(cmd, {'\n'}, &commands);
  if (!commands_count) {
    return common::make_error("Invaid command line.");
  }

  std::vector<command_buffer_t> stable_commands;
  for (command_buffer_t input : commands) {
    command_buffer_t stable_input = StableCommand(input);
    if (stable_input.empty()) {
      continue;
    }
    stable_commands.push_back(stable_input);
  }

  *cmds = stable_commands;
  return common::Error();
}

ICommandTranslator::ICommandTranslator(const std::vector<CommandHolder>& commands) : commands_(commands) {}

ICommandTranslator::~ICommandTranslator() {}

common::Error ICommandTranslator::GetDatabasesCommand(command_buffer_t* cmdstring) const {
  if (!cmdstring) {
    return common::make_error_inval();
  }

  *cmdstring = DB_GET_DATABASES_COMMAND;
  return common::Error();
}

common::Error ICommandTranslator::RemoveDBCommand(const std::string& name, command_buffer_t* cmdstring) const {
  if (!cmdstring) {
    return common::make_error_inval();
  }

  command_buffer_writer_t wr;
  wr << DB_REMOVEDB_COMMAND " " << name;
  *cmdstring = wr.str();
  return common::Error();
}

common::Error ICommandTranslator::CreateDBCommand(const std::string& name, command_buffer_t* cmdstring) const {
  if (!cmdstring) {
    return common::make_error_inval();
  }

  command_buffer_writer_t wr;
  wr << DB_CREATEDB_COMMAND " " << name;
  *cmdstring = wr.str();
  return common::Error();
}

common::Error ICommandTranslator::SelectDBCommand(const std::string& name, command_buffer_t* cmdstring) const {
  if (!cmdstring) {
    return common::make_error_inval();
  }

  command_buffer_writer_t wr;
  wr << DB_SELECTDB_COMMAND " " << name;
  *cmdstring = wr.str();
  return common::Error();
}

common::Error ICommandTranslator::FlushDBCommand(command_buffer_t* cmdstring) const {
  if (!cmdstring) {
    return common::make_error_inval();
  }

  *cmdstring = DB_FLUSHDB_COMMAND;
  return common::Error();
}

common::Error ICommandTranslator::DeleteKeyCommand(const NKey& key, command_buffer_t* cmdstring) const {
  if (!cmdstring) {
    return common::make_error_inval();
  }

  return DeleteKeyCommandImpl(key, cmdstring);
}

common::Error ICommandTranslator::RenameKeyCommand(const NKey& key,
                                                   const key_t& new_name,
                                                   command_buffer_t* cmdstring) const {
  if (!cmdstring) {
    return common::make_error_inval();
  }

  return RenameKeyCommandImpl(key, new_name, cmdstring);
}

common::Error ICommandTranslator::CreateKeyCommand(const NDbKValue& key, command_buffer_t* cmdstring) const {
  if (!cmdstring) {
    return common::make_error_inval();
  }

  return CreateKeyCommandImpl(key, cmdstring);
}

common::Error ICommandTranslator::LoadKeyCommand(const NKey& key,
                                                 common::Value::Type type,
                                                 command_buffer_t* cmdstring) const {
  if (!cmdstring) {
    return common::make_error_inval();
  }

  return LoadKeyCommandImpl(key, type, cmdstring);
}

common::Error ICommandTranslator::ChangeKeyTTLCommand(const NKey& key, ttl_t ttl, command_buffer_t* cmdstring) const {
  if (!cmdstring) {
    return common::make_error_inval();
  }

  return ChangeKeyTTLCommandImpl(key, ttl, cmdstring);
}

common::Error ICommandTranslator::LoadKeyTTLCommand(const NKey& key, command_buffer_t* cmdstring) const {
  if (!cmdstring) {
    return common::make_error_inval();
  }

  return LoadKeyTTLCommandImpl(key, cmdstring);
}

bool ICommandTranslator::IsLoadKeyCommand(const command_buffer_t& cmd, readable_string_t* key) const {
  if (!key) {
    return false;
  }

  command_buffer_t stabled_command = StableCommand(cmd);
  if (stabled_command.empty()) {
    return false;
  }

  int argc;
  sds* argv = sdssplitargslong(stabled_command.data(), &argc);
  if (!argv) {
    return false;
  }

  commands_args_t standart_argv;
  for (int i = 0; i < argc; ++i) {
    standart_argv.push_back(command_buffer_t(argv[i], sdslen(argv[i])));
  }

  const CommandHolder* cmdh = nullptr;
  size_t off = 0;
  common::Error err = TestCommandLineArgs(standart_argv, &cmdh, &off);
  if (err) {
    sdsfreesplitres(argv, argc);
    return false;
  }

  if (IsLoadKeyCommandImpl(*cmdh)) {
    *key = command_buffer_t(argv[off], strlen(argv[off]));
    sdsfreesplitres(argv, argc);
    return true;
  }

  sdsfreesplitres(argv, argc);
  return false;
}

common::Error ICommandTranslator::PublishCommand(const NDbPSChannel& channel,
                                                 const std::string& message,
                                                 command_buffer_t* cmdstring) const {
  if (!cmdstring || message.empty()) {
    return common::make_error_inval();
  }

  return PublishCommandImpl(channel, message, cmdstring);
}

common::Error ICommandTranslator::SubscribeCommand(const NDbPSChannel& channel, command_buffer_t* cmdstring) const {
  if (!cmdstring) {
    return common::make_error_inval();
  }

  return SubscribeCommandImpl(channel, cmdstring);
}

common::Error ICommandTranslator::InvalidInputArguments(const std::string& cmd) {
  std::string buff = common::MemSPrintf("Invalid input argument(s) for command: %s.", cmd);
  return common::make_error(buff);
}

common::Error ICommandTranslator::NotSupported(const std::string& cmd) {
  std::string buff = common::MemSPrintf("Not supported command: %s.", cmd);
  return common::make_error(buff);
}

common::Error ICommandTranslator::UnknownCommand(const std::string& cmd) {
  std::string buff = common::MemSPrintf("Unknown sequence: '%s'.", cmd);
  return common::make_error(buff);
}

common::Error ICommandTranslator::UnknownSequence(commands_args_t argv) {
  std::string result;
  for (size_t i = 0; i < argv.size(); ++i) {
    result += common::ConvertToString(argv[i]);
    if (i != argv.size() - 1) {
      result += " ";
    }
  }
  std::string buff = common::MemSPrintf("Unknown sequence: '%s'.", result);
  return common::make_error(buff);
}

std::vector<CommandInfo> ICommandTranslator::Commands() const {
  std::vector<CommandInfo> cmds;
  for (size_t i = 0; i < commands_.size(); ++i) {
    const CommandHolder* cmd = &commands_[i];
    cmds.push_back(*cmd);
  }
  return cmds;
}

common::Error ICommandTranslator::FindCommand(const std::string& command_first_name, const CommandHolder** info) const {
  if (!info || command_first_name.empty()) {
    return common::make_error_inval();
  }

  for (size_t i = 0; i < commands_.size(); ++i) {
    const CommandHolder* cmd = &commands_[i];
    if (cmd->IsEqualFirstName(command_first_name)) {
      *info = cmd;
      return common::Error();
    }
  }

  return UnknownCommand(command_first_name);
}

common::Error ICommandTranslator::FindCommand(commands_args_t argv, const CommandHolder** info, size_t* off) const {
  if (!info || !off) {
    return common::make_error_inval();
  }

  for (size_t i = 0; i < commands_.size(); ++i) {
    const CommandHolder* cmd = &commands_[i];
    size_t loff = 0;
    if (cmd->IsCommand(argv, &loff)) {
      *info = cmd;
      *off = loff;
      return common::Error();
    }
  }

  return UnknownSequence(argv);
}

common::Error ICommandTranslator::TestCommandArgs(const CommandHolder* cmd, commands_args_t argv) const {
  if (!cmd) {
    return common::make_error_inval();
  }

  return cmd->TestArgs(argv);
}

common::Error ICommandTranslator::TestCommandLine(const command_buffer_t& cmd) const {
  command_buffer_t stabled_command = StableCommand(cmd);
  if (stabled_command.empty()) {
    return common::make_error_inval();
  }

  int argc;
  sds* argv = sdssplitargslong(stabled_command.data(), &argc);
  if (!argv) {
    return common::make_error_inval();
  }

  commands_args_t standart_argv;
  for (int i = 0; i < argc; ++i) {
    standart_argv.push_back(command_buffer_t(argv[i], sdslen(argv[i])));
  }
  const CommandHolder* cmdh = nullptr;
  size_t loff = 0;
  common::Error err = TestCommandLineArgs(standart_argv, &cmdh, &loff);
  if (err) {
    sdsfreesplitres(argv, argc);
    return err;
  }

  return common::Error();
}

common::Error ICommandTranslator::TestCommandLineArgs(commands_args_t argv,
                                                      const CommandHolder** info,
                                                      size_t* off) const {
  const CommandHolder* cmd = nullptr;
  size_t loff = 0;
  common::Error err = FindCommand(argv, &cmd, &loff);
  if (err) {
    return err;
  }

  commands_args_t stabled;
  for (size_t i = loff; i < argv.size(); ++i) {
    stabled.push_back(argv[i]);
  }
  err = TestCommandArgs(cmd, stabled);
  if (err) {
    return err;
  }

  *info = cmd;
  *off = loff;
  return common::Error();
}

}  // namespace core
}  // namespace fastonosql
