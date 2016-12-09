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
  for (size_t i = 0; i < commands_.size(); ++i) {
    size_t off = 0;
    if (commands_[i].IsCommand(argc, standart_argv, &off)) {
      CommandHolder cmd = commands_[i];
      if (IsLoadKeyCommandImpl(cmd)) {
        *key = argv[off];
        sdsfreesplitres(argv, argc);
        return true;
      }
      sdsfreesplitres(argv, argc);
      return false;
    }
  }

  sdsfreesplitres(argv, argc);
  return false;
}

}  // namespace core
}  // namespace fastonosql
