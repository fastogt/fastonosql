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

#include "core/ssdb/command_translator.h"

#include <common/sprintf.h>

#include "global/global.h"

#define SSDB_SET_KEY_PATTERN_2ARGS_SS "SET %s %s"
#define SSDB_SET_KEY_LIST_PATTERN_2ARGS_SS "LPUSH %s %s"
#define SSDB_SET_KEY_SET_PATTERN_2ARGS_SS "SADD %s %s"
#define SSDB_SET_KEY_ZSET_PATTERN_2ARGS_SS "ZADD %s %s"
#define SSDB_SET_KEY_HASH_PATTERN_2ARGS_SS "HMSET %s %s"

#define SSDB_GET_KEY_PATTERN_1ARGS_S "GET %s"
#define SSDB_GET_KEY_LIST_PATTERN_1ARGS_S "LRANGE %s 0 -1"
#define SSDB_GET_KEY_SET_PATTERN_1ARGS_S "SMEMBERS %s"
#define SSDB_GET_KEY_ZSET_PATTERN_1ARGS_S "ZRANGE %s 0 -1"
#define SSDB_GET_KEY_HASH_PATTERN_1ARGS_S "HGET %s"

#define SSDB_DELETE_KEY_PATTERN_1ARGS_S "DEL %s"
#define SSDB_RENAME_KEY_PATTERN_2ARGS_SS "RENAME %s %s"

namespace fastonosql {
namespace core {
namespace ssdb {

CommandTranslator::CommandTranslator() {}

common::Error CommandTranslator::CreateKeyCommandImpl(const NDbKValue& key,
                                                      std::string* cmdstring) const {
  std::string patternResult;
  NValue val = key.value();
  common::Value* rval = val.get();
  std::string key_str = key.keyString();
  std::string value_str = common::ConvertToString(rval, " ");
  common::Value::Type type = key.type();
  if (type == common::Value::TYPE_ARRAY) {
    patternResult = common::MemSPrintf(SSDB_SET_KEY_LIST_PATTERN_2ARGS_SS, key_str, value_str);
  } else if (type == common::Value::TYPE_SET) {
    patternResult = common::MemSPrintf(SSDB_SET_KEY_SET_PATTERN_2ARGS_SS, key_str, value_str);
  } else if (type == common::Value::TYPE_ZSET) {
    patternResult = common::MemSPrintf(SSDB_SET_KEY_ZSET_PATTERN_2ARGS_SS, key_str, value_str);
  } else if (type == common::Value::TYPE_HASH) {
    patternResult = common::MemSPrintf(SSDB_SET_KEY_HASH_PATTERN_2ARGS_SS, key_str, value_str);
  } else {
    patternResult = common::MemSPrintf(SSDB_SET_KEY_PATTERN_2ARGS_SS, key_str, value_str);
  }

  *cmdstring = patternResult;
  return common::Error();
}

common::Error CommandTranslator::LoadKeyCommandImpl(const NKey& key,
                                                    common::Value::Type type,
                                                    std::string* cmdstring) const {
  std::string patternResult;
  std::string key_str = key.key();
  if (type == common::Value::TYPE_ARRAY) {
    patternResult = common::MemSPrintf(SSDB_GET_KEY_LIST_PATTERN_1ARGS_S, key_str);
  } else if (type == common::Value::TYPE_SET) {
    patternResult = common::MemSPrintf(SSDB_GET_KEY_SET_PATTERN_1ARGS_S, key_str);
  } else if (type == common::Value::TYPE_ZSET) {
    patternResult = common::MemSPrintf(SSDB_GET_KEY_ZSET_PATTERN_1ARGS_S, key_str);
  } else if (type == common::Value::TYPE_HASH) {
    patternResult = common::MemSPrintf(SSDB_GET_KEY_HASH_PATTERN_1ARGS_S, key_str);
  } else {
    patternResult = common::MemSPrintf(SSDB_GET_KEY_PATTERN_1ARGS_S, key_str);
  }

  *cmdstring = patternResult;
  return common::Error();
}

common::Error CommandTranslator::DeleteKeyCommandImpl(const NKey& key,
                                                      std::string* cmdstring) const {
  std::string key_str = key.key();
  *cmdstring = common::MemSPrintf(SSDB_DELETE_KEY_PATTERN_1ARGS_S, key_str);
  return common::Error();
}

common::Error CommandTranslator::RenameKeyCommandImpl(const NKey& key,
                                                      const std::string& new_name,
                                                      std::string* cmdstring) const {
  std::string key_str = key.key();
  *cmdstring = common::MemSPrintf(SSDB_RENAME_KEY_PATTERN_2ARGS_SS, key_str, new_name);
  return common::Error();
}

common::Error CommandTranslator::ChangeKeyTTLCommandImpl(const NKey& key,
                                                         ttl_t ttl,
                                                         std::string* cmdstring) const {
  UNUSED(key);
  UNUSED(ttl);
  UNUSED(cmdstring);

  std::string errorMsg = common::MemSPrintf("Sorry, but now " PROJECT_NAME_TITLE
                                            " not supported change ttl command for SSDB.");
  return common::make_error_value(errorMsg, common::ErrorValue::E_ERROR);
}

}
}  // namespace core
}  // namespace fastonosql
