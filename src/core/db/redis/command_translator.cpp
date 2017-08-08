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

#include "core/db/redis/command_translator.h"

#include <common/sprintf.h>

#define REDIS_SET_KEY_PATTERN_2ARGS_SS "SET %s %s"
#define REDIS_SET_KEY_LIST_PATTERN_2ARGS_SS "LPUSH %s %s"
#define REDIS_SET_KEY_SET_PATTERN_2ARGS_SS "SADD %s %s"
#define REDIS_SET_KEY_ZSET_PATTERN_2ARGS_SS "ZADD %s %s"
#define REDIS_SET_KEY_HASHM_PATTERN_2ARGS_SS "HMSET %s %s"

#define REDIS_COMMONTYPE_GET_KEY_COMMAND COMMONTYPE_GET_KEY_COMMAND
#define REDIS_LISTTYPE_GET_KEY_COMMAND "LRANGE"
#define REDIS_SETTYPE_GET_KEY_COMMAND "SMEMBERS"
#define REDIS_ZSETTYPE_GET_KEY_COMMAND "ZRANGE"
#define REDIS_HASHTYPE_GET_KEY_COMMAND "HGETALL"

#define REDIS_COMMONTYPE_GET_KEY_COMMAND_PATTERN_1ARGS_S REDIS_COMMONTYPE_GET_KEY_COMMAND " %s"
#define REDIS_LISTTYPE_GET_KEY_COMMAND_PATTERN_1ARGS_S REDIS_LISTTYPE_GET_KEY_COMMAND " %s 0 -1"
#define REDIS_SETTYPE_GET_KEY_COMMAND_PATTERN_1ARGS_S REDIS_SETTYPE_GET_KEY_COMMAND " %s"
#define REDIS_ZSETTYPE_GET_KEY_COMMAND_PATTERN_1ARGS_S REDIS_ZSETTYPE_GET_KEY_COMMAND " %s 0 -1 WITHSCORES"
#define REDIS_HASHTYPE_GET_KEY_COMMAND_PATTERN_1ARGS_S REDIS_HASHTYPE_GET_KEY_COMMAND " %s"

#define REDIS_DELETE_KEY_PATTERN_1ARGS_S "DEL %s"

#define REDIS_RENAME_KEY_PATTERN_2ARGS_SS "RENAME %s %s"

#define REDIS_CHANGE_TTL_2ARGS_SI "EXPIRE %s %d"
#define REDIS_PERSIST_KEY_1ARGS_S "PERSIST %s"

#define REDIS_GET_TTL_1ARGS_S "TTL %s"

#define REDIS_PUBLISH_2ARGS_SS "PUBLISH %s %s"
#define REDIS_SUBSCRIBE_1ARGS_S "SUBSCRIBE %s"

namespace fastonosql {
namespace core {
namespace redis {

CommandTranslator::CommandTranslator(const std::vector<CommandHolder>& commands) : ICommandTranslator(commands) {}

common::Error CommandTranslator::CreateKeyCommandImpl(const NDbKValue& key, std::string* cmdstring) const {
  std::string pattern_result;
  const NKey cur = key.GetKey();
  string_key_t key_str = cur.GetKey();
  std::string value_str = key.ValueString();
  common::Value::Type type = key.GetType();
  if (type == common::Value::TYPE_ARRAY) {
    pattern_result = common::MemSPrintf(REDIS_SET_KEY_LIST_PATTERN_2ARGS_SS, key_str, value_str);
  } else if (type == common::Value::TYPE_SET) {
    pattern_result = common::MemSPrintf(REDIS_SET_KEY_SET_PATTERN_2ARGS_SS, key_str, value_str);
  } else if (type == common::Value::TYPE_ZSET) {
    pattern_result = common::MemSPrintf(REDIS_SET_KEY_ZSET_PATTERN_2ARGS_SS, key_str, value_str);
  } else if (type == common::Value::TYPE_HASH) {
    pattern_result = common::MemSPrintf(REDIS_SET_KEY_HASHM_PATTERN_2ARGS_SS, key_str, value_str);
  } else {
    pattern_result = common::MemSPrintf(REDIS_SET_KEY_PATTERN_2ARGS_SS, key_str, value_str);
  }

  *cmdstring = pattern_result;
  return common::Error();
}

common::Error CommandTranslator::LoadKeyCommandImpl(const NKey& key,
                                                    common::Value::Type type,
                                                    std::string* cmdstring) const {
  std::string patternResult;
  string_key_t key_str = key.GetKey();
  if (type == common::Value::TYPE_ARRAY) {
    patternResult = common::MemSPrintf(REDIS_LISTTYPE_GET_KEY_COMMAND_PATTERN_1ARGS_S, key_str);
  } else if (type == common::Value::TYPE_SET) {
    patternResult = common::MemSPrintf(REDIS_SETTYPE_GET_KEY_COMMAND_PATTERN_1ARGS_S, key_str);
  } else if (type == common::Value::TYPE_ZSET) {
    patternResult = common::MemSPrintf(REDIS_ZSETTYPE_GET_KEY_COMMAND_PATTERN_1ARGS_S, key_str);
  } else if (type == common::Value::TYPE_HASH) {
    patternResult = common::MemSPrintf(REDIS_HASHTYPE_GET_KEY_COMMAND_PATTERN_1ARGS_S, key_str);
  } else {
    patternResult = common::MemSPrintf(REDIS_COMMONTYPE_GET_KEY_COMMAND_PATTERN_1ARGS_S, key_str);
  }

  *cmdstring = patternResult;
  return common::Error();
}

common::Error CommandTranslator::DeleteKeyCommandImpl(const NKey& key, std::string* cmdstring) const {
  string_key_t key_str = key.GetKey();
  *cmdstring = common::MemSPrintf(REDIS_DELETE_KEY_PATTERN_1ARGS_S, key_str);
  return common::Error();
}

common::Error CommandTranslator::RenameKeyCommandImpl(const NKey& key,
                                                      const std::string& new_name,
                                                      std::string* cmdstring) const {
  string_key_t key_str = key.GetKey();
  *cmdstring = common::MemSPrintf(REDIS_RENAME_KEY_PATTERN_2ARGS_SS, key_str, new_name);
  return common::Error();
}

common::Error CommandTranslator::ChangeKeyTTLCommandImpl(const NKey& key, ttl_t ttl, std::string* cmdstring) const {
  std::string patternResult;
  string_key_t key_str = key.GetKey();
  if (ttl == NO_TTL) {
    patternResult = common::MemSPrintf(REDIS_PERSIST_KEY_1ARGS_S, key_str);
  } else {
    patternResult = common::MemSPrintf(REDIS_CHANGE_TTL_2ARGS_SI, key_str, ttl);
  }

  *cmdstring = patternResult;
  return common::Error();
}

common::Error CommandTranslator::LoadKeyTTLCommandImpl(const NKey& key, std::string* cmdstring) const {
  string_key_t key_str = key.GetKey();
  *cmdstring = common::MemSPrintf(REDIS_GET_TTL_1ARGS_S, key_str);
  return common::Error();
}

bool CommandTranslator::IsLoadKeyCommandImpl(const CommandInfo& cmd) const {
  return cmd.IsEqualName(REDIS_COMMONTYPE_GET_KEY_COMMAND) || cmd.IsEqualName(REDIS_LISTTYPE_GET_KEY_COMMAND) ||
         cmd.IsEqualName(REDIS_SETTYPE_GET_KEY_COMMAND) || cmd.IsEqualName(REDIS_ZSETTYPE_GET_KEY_COMMAND) ||
         cmd.IsEqualName(REDIS_HASHTYPE_GET_KEY_COMMAND);
}

common::Error CommandTranslator::PublishCommandImpl(const NDbPSChannel& channel,
                                                    const std::string& message,
                                                    std::string* cmdstring) const {
  std::string channel_str = channel.Name();
  *cmdstring = common::MemSPrintf(REDIS_PUBLISH_2ARGS_SS, channel_str, message);
  return common::Error();
}

common::Error CommandTranslator::SubscribeCommandImpl(const NDbPSChannel& channel, std::string* cmdstring) const {
  std::string channel_str = channel.Name();
  *cmdstring = common::MemSPrintf(REDIS_SUBSCRIBE_1ARGS_S, channel_str);
  return common::Error();
}

}  // namespace redis
}  // namespace core
}  // namespace fastonosql
