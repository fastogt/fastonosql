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

#include "core/db/ssdb/command_translator.h"

#include <common/sprintf.h>

#define SSDB_SET_KEY_PATTERN_2ARGS_SS "SET %s %s"
#define SSDB_SET_KEY_LIST_PATTERN_2ARGS_SS "LPUSH %s %s"
#define SSDB_SET_KEY_SET_PATTERN_2ARGS_SS "SADD %s %s"
#define SSDB_SET_KEY_ZSET_PATTERN_2ARGS_SS "ZADD %s %s"
#define SSDB_SET_KEY_HASH_PATTERN_2ARGS_SS "HMSET %s %s"

#define SSDB_COMMONTYPE_GET_KEY_COMMAND COMMONTYPE_GET_KEY_COMMAND
#define SSDB_GET_KEY_PATTERN_1ARGS_S SSDB_COMMONTYPE_GET_KEY_COMMAND " %s"
#define SSDB_GET_KEY_LIST_PATTERN_1ARGS_S "LRANGE %s 0 -1"
#define SSDB_GET_KEY_SET_PATTERN_1ARGS_S "SMEMBERS %s"
#define SSDB_GET_KEY_ZSET_PATTERN_1ARGS_S "ZRANGE %s 0 -1"
#define SSDB_GET_KEY_HASH_PATTERN_1ARGS_S "HGET %s"
#define SSDB_DELETE_KEY_PATTERN_1ARGS_S "DEL %s"
#define SSDB_RENAME_KEY_PATTERN_2ARGS_SS "RENAME %s %s"

#define SSDB_CHANGE_TTL_2ARGS_SI "EXPIRE %s %d"
#define SSDB_GET_TTL_1ARGS_S "TTL %s"

namespace fastonosql {
namespace core {
namespace ssdb {

CommandTranslator::CommandTranslator(const std::vector<CommandHolder>& commands) : ICommandTranslator(commands) {}

common::Error CommandTranslator::CreateKeyCommandImpl(const NDbKValue& key, std::string* cmdstring) const {
  std::string pattern_result;
  const NKey cur = key.GetKey();
  key_t key_str = cur.GetKey();
  std::string value_str = key.ValueString();
  common::Value::Type type = key.GetType();
  if (type == common::Value::TYPE_ARRAY) {
    pattern_result = common::MemSPrintf(SSDB_SET_KEY_LIST_PATTERN_2ARGS_SS, key_str, value_str);
  } else if (type == common::Value::TYPE_SET) {
    pattern_result = common::MemSPrintf(SSDB_SET_KEY_SET_PATTERN_2ARGS_SS, key_str, value_str);
  } else if (type == common::Value::TYPE_ZSET) {
    pattern_result = common::MemSPrintf(SSDB_SET_KEY_ZSET_PATTERN_2ARGS_SS, key_str, value_str);
  } else if (type == common::Value::TYPE_HASH) {
    pattern_result = common::MemSPrintf(SSDB_SET_KEY_HASH_PATTERN_2ARGS_SS, key_str, value_str);
  } else {
    pattern_result = common::MemSPrintf(SSDB_SET_KEY_PATTERN_2ARGS_SS, key_str, value_str);
  }

  *cmdstring = pattern_result;
  return common::Error();
}

common::Error CommandTranslator::LoadKeyCommandImpl(const NKey& key,
                                                    common::Value::Type type,
                                                    std::string* cmdstring) const {
  std::string patternResult;
  key_t key_str = key.GetKey();
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

common::Error CommandTranslator::DeleteKeyCommandImpl(const NKey& key, std::string* cmdstring) const {
  key_t key_str = key.GetKey();
  *cmdstring = common::MemSPrintf(SSDB_DELETE_KEY_PATTERN_1ARGS_S, key_str);
  return common::Error();
}

common::Error CommandTranslator::RenameKeyCommandImpl(const NKey& key,
                                                      const std::string& new_name,
                                                      std::string* cmdstring) const {
  key_t key_str = key.GetKey();
  *cmdstring = common::MemSPrintf(SSDB_RENAME_KEY_PATTERN_2ARGS_SS, key_str, new_name);
  return common::Error();
}

common::Error CommandTranslator::ChangeKeyTTLCommandImpl(const NKey& key, ttl_t ttl, std::string* cmdstring) const {
  key_t key_str = key.GetKey();
  *cmdstring = common::MemSPrintf(SSDB_CHANGE_TTL_2ARGS_SI, key_str, ttl);
  return common::Error();
}

common::Error CommandTranslator::LoadKeyTTLCommandImpl(const NKey& key, std::string* cmdstring) const {
  key_t key_str = key.GetKey();
  *cmdstring = common::MemSPrintf(SSDB_GET_TTL_1ARGS_S, key_str);
  return common::Error();
}

bool CommandTranslator::IsLoadKeyCommandImpl(const CommandInfo& cmd) const {
  return cmd.IsEqualName(SSDB_COMMONTYPE_GET_KEY_COMMAND);
}

common::Error CommandTranslator::PublishCommandImpl(const NDbPSChannel& channel,
                                                    const std::string& message,
                                                    std::string* cmdstring) const {
  UNUSED(channel);
  UNUSED(message);
  UNUSED(cmdstring);

  static const std::string error_msg = "Sorry, but now " PROJECT_NAME_TITLE " not supported publish command for SSDB.";
  return common::make_error_value(error_msg, common::ErrorValue::E_ERROR);
}

common::Error CommandTranslator::SubscribeCommandImpl(const NDbPSChannel& channel, std::string* cmdstring) const {
  UNUSED(channel);
  UNUSED(cmdstring);

  static const std::string error_msg =
      "Sorry, but now " PROJECT_NAME_TITLE " not supported subscribe command for SSDB.";
  return common::make_error_value(error_msg, common::ErrorValue::E_ERROR);
}

}  // namespace ssdb
}  // namespace core
}  // namespace fastonosql
