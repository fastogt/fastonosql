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

#include "core/icommand_translator.h"

extern "C" {
#include "sds.h"
}

#include <common/sprintf.h>

namespace fastonosql {
namespace core {

ICommandTranslator::ICommandTranslator(const std::vector<CommandHolder>& commands)
    : commands_(commands) {}

ICommandTranslator::~ICommandTranslator() {}

common::Error ICommandTranslator::SelectDBCommand(const std::string& name,
                                                  std::string* cmdstring) const {
  if (!cmdstring) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  *cmdstring = common::MemSPrintf(SELECTDB_COMMAND_1S, name);
  return common::Error();
}

common::Error ICommandTranslator::FlushDBCommand(std::string* cmdstring) const {
  if (!cmdstring) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  *cmdstring = FLUSHDB_COMMAND;
  return common::Error();
}

common::Error ICommandTranslator::DeleteKeyCommand(const NKey& key, std::string* cmdstring) const {
  if (!cmdstring) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  return DeleteKeyCommandImpl(key, cmdstring);
}

common::Error ICommandTranslator::RenameKeyCommand(const NKey& key,
                                                   const std::string& new_name,
                                                   std::string* cmdstring) const {
  if (!cmdstring) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  return RenameKeyCommandImpl(key, new_name, cmdstring);
}

common::Error ICommandTranslator::CreateKeyCommand(const NDbKValue& key,
                                                   std::string* cmdstring) const {
  if (!cmdstring) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  return CreateKeyCommandImpl(key, cmdstring);
}

common::Error ICommandTranslator::LoadKeyCommand(const NKey& key,
                                                 common::Value::Type type,
                                                 std::string* cmdstring) const {
  if (!cmdstring) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  return LoadKeyCommandImpl(key, type, cmdstring);
}

common::Error ICommandTranslator::ChangeKeyTTLCommand(const NKey& key,
                                                      ttl_t ttl,
                                                      std::string* cmdstring) const {
  if (!cmdstring) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  return ChangeKeyTTLCommandImpl(key, ttl, cmdstring);
}

common::Error ICommandTranslator::LoadKeyTTLCommand(const NKey& key, std::string* cmdstring) const {
  if (!cmdstring) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  return LoadKeyTTLCommandImpl(key, cmdstring);
}

bool ICommandTranslator::IsLoadKeyCommand(const std::string& cmd, std::string* key) const {
  if (cmd.empty() || !key) {
    return false;
  }

  int argc;
  sds* argv = sdssplitargslong(cmd.c_str(), &argc);
  if (!argv) {
    return false;
  }

  const char** standart_argv = const_cast<const char**>(argv);
  const CommandInfo* cmdh = NULL;
  size_t off = 0;
  common::Error err = TestCommandLine(argc, standart_argv, &cmdh, &off);
  if (err && err->isError()) {
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

common::Error ICommandTranslator::NotSupported(const std::string& cmd) {
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

common::Error ICommandTranslator::TestCommandLine(int argc,
                                                  const char** argv,
                                                  const CommandInfo** info,
                                                  size_t* off) const {
  if (!off || !info) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  for (size_t i = 0; i < commands_.size(); ++i) {
    const CommandHolder* cmd = &commands_[i];
    size_t loff = 0;
    if (cmd->IsCommand(argc, argv, &loff)) {
      int argc_to_call = argc - loff;
      const char** argv_to_call = argv + loff;
      common::Error err = cmd->TestArgs(argc_to_call, argv_to_call);
      if (err && err->isError()) {
        return err;
      }

      *info = cmd;
      *off = loff;
      return common::Error();
    }
  }

  return UnknownSequence(argc, argv);
}

}  // namespace core
}  // namespace fastonosql
