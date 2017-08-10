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

#include <common/convert2string.h>

#define REDIS_SET_KEY_COMMAND COMMONTYPE_SET_KEY_COMMAND
#define REDIS_SET_KEY_ARRAY_COMMAND "LPUSH"
#define REDIS_SET_KEY_SET_COMMAND "SADD"
#define REDIS_SET_KEY_ZSET_COMMAND "ZADD"
#define REDIS_SET_KEY_HASH_COMMAND "HMSET"

#define REDIS_GET_KEY_COMMAND COMMONTYPE_GET_KEY_COMMAND
#define REDIS_GET_KEY_ARRAY_COMMAND "LRANGE"
#define REDIS_GET_KEY_SET_COMMAND "SMEMBERS"
#define REDIS_GET_KEY_ZSET_COMMAND "ZRANGE"
#define REDIS_GET_KEY_HASH_COMMAND "HGETALL"

#define REDIS_DELETE_KEY_COMMAND "DEL"
#define REDIS_RENAME_KEY_COMMAND "RENAME"
#define REDIS_CHANGE_TTL_COMMAND "EXPIRE"
#define REDIS_PERSIST_KEY_COMMAND "PERSIST"

#define REDIS_GET_TTL_COMMAND "TTL"
#define REDIS_PUBLISH_COMMAND "PUBLISH"
#define REDIS_SUBSCRIBE_COMMAND "SUBSCRIBE"

namespace fastonosql {
namespace core {
namespace redis {

CommandTranslator::CommandTranslator(const std::vector<CommandHolder>& commands) : ICommandTranslator(commands) {}

common::Error CommandTranslator::CreateKeyCommandImpl(const NDbKValue& key, command_buffer_t* cmdstring) const {
  const NKey cur = key.GetKey();
  key_t key_str = cur.GetKey();
  std::string value_str = key.ValueString();
  command_buffer_writer_t wr;
  common::Value::Type type = key.GetType();
  if (type == common::Value::TYPE_ARRAY) {
    wr << REDIS_SET_KEY_ARRAY_COMMAND << " " << key_str.GetKey() << " " << value_str;
  } else if (type == common::Value::TYPE_SET) {
    wr << REDIS_SET_KEY_SET_COMMAND << " " << key_str.GetKey() << " " << value_str;
  } else if (type == common::Value::TYPE_ZSET) {
    wr << REDIS_SET_KEY_ZSET_COMMAND << " " << key_str.GetKey() << " " << value_str;
  } else if (type == common::Value::TYPE_HASH) {
    wr << REDIS_SET_KEY_HASH_COMMAND << " " << key_str.GetKey() << " " << value_str;
  } else {
    wr << REDIS_SET_KEY_COMMAND << " " << key_str.GetKey() << " " << value_str;
  }

  *cmdstring = wr.GetBuffer();
  return common::Error();
}

common::Error CommandTranslator::LoadKeyCommandImpl(const NKey& key,
                                                    common::Value::Type type,
                                                    command_buffer_t* cmdstring) const {
  key_t key_str = key.GetKey();
  command_buffer_writer_t wr;
  if (type == common::Value::TYPE_ARRAY) {
    wr << REDIS_GET_KEY_ARRAY_COMMAND << " " << key_str.GetKey() << " 0 -1";
  } else if (type == common::Value::TYPE_SET) {
    wr << REDIS_GET_KEY_SET_COMMAND << " " << key_str.GetKey();
  } else if (type == common::Value::TYPE_ZSET) {
    wr << REDIS_GET_KEY_ZSET_COMMAND << " " << key_str.GetKey() << " 0 -1 WITHSCORES";
  } else if (type == common::Value::TYPE_HASH) {
    wr << REDIS_GET_KEY_HASH_COMMAND << " " << key_str.GetKey();
  } else {
    wr << REDIS_GET_KEY_COMMAND << " " << key_str.GetKey();
  }

  *cmdstring = wr.GetBuffer();
  return common::Error();
}

common::Error CommandTranslator::DeleteKeyCommandImpl(const NKey& key, command_buffer_t* cmdstring) const {
  key_t key_str = key.GetKey();
  command_buffer_writer_t wr;
  wr << REDIS_DELETE_KEY_COMMAND << " " << key_str.GetKey();
  *cmdstring = wr.GetBuffer();
  return common::Error();
}

common::Error CommandTranslator::RenameKeyCommandImpl(const NKey& key,
                                                      const string_key_t& new_name,
                                                      command_buffer_t* cmdstring) const {
  key_t key_str = key.GetKey();
  command_buffer_writer_t wr;
  wr << REDIS_RENAME_KEY_COMMAND << " " << key_str.GetKey() << " " << new_name;
  *cmdstring = wr.GetBuffer();
  return common::Error();
}

common::Error CommandTranslator::ChangeKeyTTLCommandImpl(const NKey& key,
                                                         ttl_t ttl,
                                                         command_buffer_t* cmdstring) const {
  key_t key_str = key.GetKey();
  command_buffer_writer_t wr;
  if (ttl == NO_TTL) {
    wr << REDIS_PERSIST_KEY_COMMAND << " " << key_str.GetKey();
  } else {
    wr << REDIS_CHANGE_TTL_COMMAND << " " << key_str.GetKey() << " " << common::ConvertToString(ttl);
  }

  *cmdstring = wr.GetBuffer();
  return common::Error();
}

common::Error CommandTranslator::LoadKeyTTLCommandImpl(const NKey& key, command_buffer_t* cmdstring) const {
  key_t key_str = key.GetKey();
  command_buffer_writer_t wr;
  wr << REDIS_GET_TTL_COMMAND << " " << key_str.GetKey();
  *cmdstring = wr.GetBuffer();
  return common::Error();
}

bool CommandTranslator::IsLoadKeyCommandImpl(const CommandInfo& cmd) const {
  return cmd.IsEqualName(REDIS_GET_KEY_COMMAND) || cmd.IsEqualName(REDIS_GET_KEY_ARRAY_COMMAND) ||
         cmd.IsEqualName(REDIS_GET_KEY_SET_COMMAND) || cmd.IsEqualName(REDIS_GET_KEY_ZSET_COMMAND) ||
         cmd.IsEqualName(REDIS_GET_KEY_HASH_COMMAND);
}

common::Error CommandTranslator::PublishCommandImpl(const NDbPSChannel& channel,
                                                    const std::string& message,
                                                    command_buffer_t* cmdstring) const {
  std::string channel_str = channel.Name();
  command_buffer_writer_t wr;
  wr << REDIS_PUBLISH_COMMAND << " " << channel_str << " " << message;
  *cmdstring = wr.GetBuffer();
  return common::Error();
}

common::Error CommandTranslator::SubscribeCommandImpl(const NDbPSChannel& channel, command_buffer_t* cmdstring) const {
  std::string channel_str = channel.Name();
  command_buffer_writer_t wr;
  wr << REDIS_SUBSCRIBE_COMMAND << " " << channel_str;
  *cmdstring = wr.GetBuffer();
  return common::Error();
}

}  // namespace redis
}  // namespace core
}  // namespace fastonosql
