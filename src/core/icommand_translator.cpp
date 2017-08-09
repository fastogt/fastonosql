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
  size_t commands_count = common::Tokenize(cmd, "\n", &commands);
  if (!commands_count) {
    return common::make_error_value("Invaid command line.", common::ErrorValue::E_ERROR);
  }

  std::vector<std::string> stable_commands;
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

  *cmdstring = common::MemSPrintf(SELECTDB_COMMAND_1S, name);
  return common::Error();
}

common::Error ICommandTranslator::FlushDBCommand(command_buffer_t* cmdstring) const {
  if (!cmdstring) {
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  *cmdstring = FLUSHDB_COMMAND;
  return common::Error();
}

common::Error ICommandTranslator::DeleteKeyCommand(const NKey& key, command_buffer_t* cmdstring) const {
  if (!cmdstring) {
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  return DeleteKeyCommandImpl(key, cmdstring);
}

common::Error ICommandTranslator::RenameKeyCommand(const NKey& key,
                                                   const std::string& new_name,
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

  const char* ccmd = common::utils::c_strornull(cmd);
  if (!ccmd) {
    return false;
  }

  int argc;
  sds* argv = sdssplitargslong(ccmd, &argc);
  if (!argv) {
    return false;
  }

  const char** standart_argv = const_cast<const char**>(argv);
  const CommandHolder* cmdh = nullptr;
  size_t off = 0;
  common::Error err = TestCommandLineArgs(argc, standart_argv, &cmdh, &off);
  if (err && err->IsError()) {
    sdsfreesplitres(argv, argc);
    return false;
  }

  if (IsLoadKeyCommandImpl(*cmdh)) {
    *key = argv[off];
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

common::Error ICommandTranslator::InvalidInputArguments(const command_buffer_t& cmd) {
  std::string buff = common::MemSPrintf("Invalid input argument(s) for command: %s.", cmd);
  return common::make_error_value(buff, common::ErrorValue::E_ERROR);
}

common::Error ICommandTranslator::NotSupported(const command_buffer_t& cmd) {
  std::string buff = common::MemSPrintf("Not supported command: %s.", cmd);
  return common::make_error_value(buff, common::ErrorValue::E_ERROR);
}

common::Error ICommandTranslator::UnknownSequence(int argc, const char** argv) {
  std::string result;
  for (int i = 0; i < argc; ++i) {
    result += argv[i];
    if (i != argc - 1) {
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

common::Error ICommandTranslator::FindCommand(int argc,
                                              const char** argv,
                                              const CommandHolder** info,
                                              size_t* off) const {
  if (!info || !off) {
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  for (size_t i = 0; i < commands_.size(); ++i) {
    const CommandHolder* cmd = &commands_[i];
    size_t loff = 0;
    if (cmd->IsCommand(argc, argv, &loff)) {
      *info = cmd;
      *off = loff;
      return common::Error();
    }
  }

  return UnknownSequence(argc, argv);
}

common::Error ICommandTranslator::TestCommandArgs(const CommandHolder* cmd,
                                                  int argc_to_call,
                                                  const char** argv_to_call) const {
  if (!cmd) {
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  return cmd->TestArgs(argc_to_call, argv_to_call);
}

common::Error ICommandTranslator::TestCommandLine(const command_buffer_t& cmd) const {
  const char* ccmd = common::utils::c_strornull(cmd);
  if (!ccmd) {
    return false;
  }

  int argc;
  sds* argv = sdssplitargslong(ccmd, &argc);
  if (!argv) {
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  const char** standart_argv = const_cast<const char**>(argv);
  const CommandHolder* cmdh = nullptr;
  size_t loff = 0;
  common::Error err = TestCommandLineArgs(argc, standart_argv, &cmdh, &loff);
  if (err && err->IsError()) {
    sdsfreesplitres(argv, argc);
    return err;
  }

  return common::Error();
}

common::Error ICommandTranslator::TestCommandLineArgs(int argc,
                                                      const char** argv,
                                                      const CommandHolder** info,
                                                      size_t* off) const {
  const CommandHolder* cmd = nullptr;
  size_t loff = 0;
  common::Error err = FindCommand(argc, argv, &cmd, &loff);
  if (err && err->IsError()) {
    return err;
  }

  int argc_to_call = argc - loff;
  const char** argv_to_call = argv + loff;
  err = TestCommandArgs(cmd, argc_to_call, argv_to_call);
  if (err && err->IsError()) {
    return err;
  }

  *info = cmd;
  *off = loff;
  return common::Error();
}

}  // namespace core
}  // namespace fastonosql
