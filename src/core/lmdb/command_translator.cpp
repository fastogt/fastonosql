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

#include "core/lmdb/command_translator.h"

#include "global/global.h"
#include "common/sprintf.h"

#define LMDB_SET_KEY_PATTERN_2ARGS_SS "SET %s %s"
#define LMDB_GET_KEY_PATTERN_1ARGS_S "GET %s"
#define LMDB_DELETE_KEY_PATTERN_1ARGS_S "DEL %s"

namespace fastonosql {
namespace core {
namespace lmdb {
CommandTranslator::CommandTranslator() {}

common::Error CommandTranslator::createKeyCommandImpl(const key_and_value_t& key,
                                                      std::string* cmdstring) const {
  NValue val = key.value();
  common::Value* rval = val.get();
  std::string key_str = key.keyString();
  std::string value_str = common::ConvertToString(rval, " ");
  *cmdstring = common::MemSPrintf(LMDB_SET_KEY_PATTERN_2ARGS_SS, key_str, value_str);
  return common::Error();
}

common::Error CommandTranslator::loadKeyCommandImpl(const key_t& key,
                                                    common::Value::Type type,
                                                    std::string* cmdstring) const {
  UNUSED(type);

  std::string key_str = key.key();
  *cmdstring = common::MemSPrintf(LMDB_GET_KEY_PATTERN_1ARGS_S, key_str);
  return common::Error();
}

common::Error CommandTranslator::deleteKeyCommandImpl(const key_t& key,
                                                      std::string* cmdstring) const {
  std::string key_str = key.key();
  *cmdstring = common::MemSPrintf(LMDB_DELETE_KEY_PATTERN_1ARGS_S, key_str);
  return common::Error();
}

common::Error CommandTranslator::changeKeyTTLCommandImpl(const key_t& key,
                                                         ttl_t ttl,
                                                         std::string* cmdstring) const {
  UNUSED(key);
  UNUSED(ttl);
  UNUSED(cmdstring);

  std::string errorMsg = common::MemSPrintf("Sorry, but now " PROJECT_NAME_TITLE
                                            " not supported change ttl command for LMDB.");
  return common::make_error_value(errorMsg, common::ErrorValue::E_ERROR);
}
}
}  // namespace core
}  // namespace fastonosql
