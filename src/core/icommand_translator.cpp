/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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
#include <common/string_util.h>
#include <common/utils.h>

#include "core/types.h"

namespace fastonosql {
namespace core {

common::Error ParseCommands(const command_buffer_t& cmd, std::vector<command_buffer_t>* cmds) {
  if (cmd.empty()) {
    return common::make_error_value("Empty command line.", common::ErrorValue::E_ERROR);
  }

  std::vector<command_buffer_t> commands;
  size_t commands_count = common::Tokenize(cmd, {'\n'}, &commands);
  if (!commands_count) {
    return common::make_error_value("Invaid command line.", common::ErrorValue::E_ERROR);
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

common::Error ICommandTranslator::SelectDBCommand(const std::string& name, command_buffer_t* cmdstring) const {
  if (!cmdstring) {
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  command_buffer_writer_t wr;
  wr << MAKE_COMMAND_BUFFER(DB_SELECTDB_COMMAND) << MAKE_COMMAND_BUFFER(" ") << name;
  *cmdstring = wr.str();
  return common::Error();
}

common::Error ICommandTranslator::FlushDBCommand(command_buffer_t* cmdstring) const {
  if (!cmdstring) {
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  command_buffer_writer_t wr;
  wr << MAKE_COMMAND_BUFFER(DB_FLUSHDB_COMMAND);
  *cmdstring = wr.str();
  return common::Error();
}

common::Error ICommandTranslator::DeleteKeyCommand(const NKey& key, command_buffer_t* cmdstring) const {
  if (!cmdstring) {
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  return DeleteKeyCommandImpl(key, cmdstring);
}

common::Error ICommandTranslator::RenameKeyCommand(const NKey& key,
                                                   const key_t& new_name,
                                                   command_buffer_t* cmdstring) const {
  if (!cmdstring) {
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  return RenameKeyCommandImpl(key, new_name, cmdstring);
}

common::Error ICommandTranslator::CreateKeyCommand(const NDbKValue& key, command_buffer_t* cmdstring) const {
  if (!cmdstring) {
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  return CreateKeyCommandImpl(key, cmdstring);
}

common::Error ICommandTranslator::LoadKeyCommand(const NKey& key,
                                                 common::Value::Type type,
                                                 command_buffer_t* cmdstring) const {
  if (!cmdstring) {
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  return LoadKeyCommandImpl(key, type, cmdstring);
}

common::Error ICommandTranslator::ChangeKeyTTLCommand(const NKey& key, ttl_t ttl, command_buffer_t* cmdstring) const {
  if (!cmdstring) {
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  return ChangeKeyTTLCommandImpl(key, ttl, cmdstring);
}

common::Error ICommandTranslator::LoadKeyTTLCommand(const NKey& key, command_buffer_t* cmdstring) const {
  if (!cmdstring) {
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  return LoadKeyTTLCommandImpl(key, cmdstring);
}

bool ICommandTranslator::IsLoadKeyCommand(const command_buffer_t& cmd, string_key_t* key) const {
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
  if (err && err->IsError()) {
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
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  return PublishCommandImpl(channel, message, cmdstring);
}

common::Error ICommandTranslator::SubscribeCommand(const NDbPSChannel& channel, command_buffer_t* cmdstring) const {
  if (!cmdstring) {
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  return SubscribeCommandImpl(channel, cmdstring);
}

common::Error ICommandTranslator::InvalidInputArguments(const std::string& cmd) {
  std::string buff = common::MemSPrintf("Invalid input argument(s) for command: %s.", cmd);
  return common::make_error_value(buff, common::ErrorValue::E_ERROR);
}

common::Error ICommandTranslator::NotSupported(const std::string& cmd) {
  std::string buff = common::MemSPrintf("Not supported command: %s.", cmd);
  return common::make_error_value(buff, common::ErrorValue::E_ERROR);
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
  return common::make_error_value(buff, common::ErrorValue::E_ERROR);
}

std::vector<CommandInfo> ICommandTranslator::Commands() const {
  std::vector<CommandInfo> cmds;
  for (size_t i = 0; i < commands_.size(); ++i) {
    const CommandHolder* cmd = &commands_[i];
    cmds.push_back(*cmd);
  }
  return cmds;
}

common::Error ICommandTranslator::FindCommand(commands_args_t argv, const CommandHolder** info, size_t* off) const {
  if (!info || !off) {
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
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
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  return cmd->TestArgs(argv);
}

common::Error ICommandTranslator::TestCommandLine(const command_buffer_t& cmd) const {
  command_buffer_t stabled_command = StableCommand(cmd);
  if (stabled_command.empty()) {
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  int argc;
  sds* argv = sdssplitargslong(stabled_command.data(), &argc);
  if (!argv) {
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  commands_args_t standart_argv;
  for (int i = 0; i < argc; ++i) {
    standart_argv.push_back(command_buffer_t(argv[i], sdslen(argv[i])));
  }
  const CommandHolder* cmdh = nullptr;
  size_t loff = 0;
  common::Error err = TestCommandLineArgs(standart_argv, &cmdh, &loff);
  if (err && err->IsError()) {
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
  if (err && err->IsError()) {
    return err;
  }

  commands_args_t stabled;
  for (size_t i = loff; i < argv.size(); ++i) {
    stabled.push_back(argv[i]);
  }
  err = TestCommandArgs(cmd, stabled);
  if (err && err->IsError()) {
    return err;
  }

  *info = cmd;
  *off = loff;
  return common::Error();
}

}  // namespace core
}  // namespace fastonosql
