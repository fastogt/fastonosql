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

#include <common/convert2string.h>

#define SSDB_SET_KEY_COMMAND COMMONTYPE_SET_KEY_COMMAND
#define SSDB_SET_KEY_ARRAY_COMMAND "LPUSH"
#define SSDB_SET_KEY_SET_COMMAND "SADD"
#define SSDB_SET_KEY_ZSET_COMMAND "ZADD"
#define SSDB_SET_KEY_HASH_COMMAND "HMSET"

#define SSDB_GET_KEY_COMMAND COMMONTYPE_GET_KEY_COMMAND
#define SSDB_GET_KEY_ARRAY_COMMAND "LRANGE"
#define SSDB_GET_KEY_SET_COMMAND "SMEMBERS"
#define SSDB_GET_KEY_ZSET_COMMAND "ZRANGE"
#define SSDB_GET_KEY_HASH_COMMAND "HGET"

#define SSDB_DELETE_KEY_COMMAND "DEL"
#define SSDB_RENAME_KEY_COMMAND "RENAME"
#define SSDB_CHANGE_TTL_COMMAND "EXPIRE"
#define SSDB_GET_TTL_COMMAND "TTL"

namespace fastonosql {
namespace core {
namespace ssdb {

CommandTranslator::CommandTranslator(const std::vector<CommandHolder>& commands) : ICommandTranslator(commands) {}

common::Error CommandTranslator::CreateKeyCommandImpl(const NDbKValue& key, command_buffer_t* cmdstring) const {
  command_buffer_writer_t wr;
  const NKey cur = key.GetKey();
  key_t key_str = cur.GetKey();
  std::string value_str = key.ValueString();
  common::Value::Type type = key.GetType();
  if (type == common::Value::TYPE_ARRAY) {
    wr << MAKE_COMMAND_BUFFER(SSDB_SET_KEY_ARRAY_COMMAND) << MAKE_COMMAND_BUFFER(" ") << key_str.GetKey()
       << MAKE_COMMAND_BUFFER(" ") << value_str;
  } else if (type == common::Value::TYPE_SET) {
    wr << MAKE_COMMAND_BUFFER(SSDB_SET_KEY_SET_COMMAND) << MAKE_COMMAND_BUFFER(" ") << key_str.GetKey()
       << MAKE_COMMAND_BUFFER(" ") << value_str;
  } else if (type == common::Value::TYPE_ZSET) {
    wr << MAKE_COMMAND_BUFFER(SSDB_SET_KEY_ZSET_COMMAND) << MAKE_COMMAND_BUFFER(" ") << key_str.GetKey()
       << MAKE_COMMAND_BUFFER(" ") << value_str;
  } else if (type == common::Value::TYPE_HASH) {
    wr << MAKE_COMMAND_BUFFER(SSDB_SET_KEY_HASH_COMMAND) << MAKE_COMMAND_BUFFER(" ") << key_str.GetKey()
       << MAKE_COMMAND_BUFFER(" ") << value_str;
  } else {
    wr << MAKE_COMMAND_BUFFER(SSDB_SET_KEY_COMMAND) << MAKE_COMMAND_BUFFER(" ") << key_str.GetKey()
       << MAKE_COMMAND_BUFFER(" ") << value_str;
  }

  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::LoadKeyCommandImpl(const NKey& key,
                                                    common::Value::Type type,
                                                    command_buffer_t* cmdstring) const {
  command_buffer_writer_t wr;
  key_t key_str = key.GetKey();
  if (type == common::Value::TYPE_ARRAY) {
    wr << MAKE_COMMAND_BUFFER(SSDB_GET_KEY_ARRAY_COMMAND) << MAKE_COMMAND_BUFFER(" ") << key_str.GetKey()
       << MAKE_COMMAND_BUFFER(" 0 -1");
  } else if (type == common::Value::TYPE_SET) {
    wr << MAKE_COMMAND_BUFFER(SSDB_GET_KEY_SET_COMMAND) << MAKE_COMMAND_BUFFER(" ") << key_str.GetKey();
  } else if (type == common::Value::TYPE_ZSET) {
    wr << MAKE_COMMAND_BUFFER(SSDB_GET_KEY_ZSET_COMMAND) << MAKE_COMMAND_BUFFER(" ") << key_str.GetKey()
       << MAKE_COMMAND_BUFFER(" 0 -1");
  } else if (type == common::Value::TYPE_HASH) {
    wr << MAKE_COMMAND_BUFFER(SSDB_GET_KEY_HASH_COMMAND) << MAKE_COMMAND_BUFFER(" ") << key_str.GetKey();
  } else {
    wr << MAKE_COMMAND_BUFFER(SSDB_GET_KEY_COMMAND) << MAKE_COMMAND_BUFFER(" ") << key_str.GetKey();
  }

  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::DeleteKeyCommandImpl(const NKey& key, command_buffer_t* cmdstring) const {
  key_t key_str = key.GetKey();
  command_buffer_writer_t wr;
  wr << MAKE_COMMAND_BUFFER(SSDB_DELETE_KEY_COMMAND) << MAKE_COMMAND_BUFFER(" ") << key_str.GetKey();
  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::RenameKeyCommandImpl(const NKey& key,
                                                      const string_key_t& new_name,
                                                      command_buffer_t* cmdstring) const {
  key_t key_str = key.GetKey();
  command_buffer_writer_t wr;
  wr << MAKE_COMMAND_BUFFER(SSDB_RENAME_KEY_COMMAND) << MAKE_COMMAND_BUFFER(" ") << key_str.GetKey()
     << MAKE_COMMAND_BUFFER(" ") << new_name;
  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::ChangeKeyTTLCommandImpl(const NKey& key,
                                                         ttl_t ttl,
                                                         command_buffer_t* cmdstring) const {
  key_t key_str = key.GetKey();
  command_buffer_writer_t wr;
  wr << MAKE_COMMAND_BUFFER(SSDB_CHANGE_TTL_COMMAND) << MAKE_COMMAND_BUFFER(" ") << key_str.GetKey()
     << MAKE_COMMAND_BUFFER(" ") << common::ConvertToString(ttl);
  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::LoadKeyTTLCommandImpl(const NKey& key, command_buffer_t* cmdstring) const {
  key_t key_str = key.GetKey();
  command_buffer_writer_t wr;
  wr << MAKE_COMMAND_BUFFER(SSDB_GET_TTL_COMMAND) << MAKE_COMMAND_BUFFER(" ") << key_str.GetKey();
  *cmdstring = wr.str();
  return common::Error();
}

bool CommandTranslator::IsLoadKeyCommandImpl(const CommandInfo& cmd) const {
  return cmd.IsEqualName(SSDB_GET_KEY_COMMAND);
}

common::Error CommandTranslator::PublishCommandImpl(const NDbPSChannel& channel,
                                                    const std::string& message,
                                                    command_buffer_t* cmdstring) const {
  UNUSED(channel);
  UNUSED(message);
  UNUSED(cmdstring);

  static const std::string error_msg = "Sorry, but now " PROJECT_NAME_TITLE " not supported publish command for SSDB.";
  return common::make_error_value(error_msg, common::ErrorValue::E_ERROR);
}

common::Error CommandTranslator::SubscribeCommandImpl(const NDbPSChannel& channel, command_buffer_t* cmdstring) const {
  UNUSED(channel);
  UNUSED(cmdstring);

  static const std::string error_msg =
      "Sorry, but now " PROJECT_NAME_TITLE " not supported subscribe command for SSDB.";
  return common::make_error_value(error_msg, common::ErrorValue::E_ERROR);
}

}  // namespace ssdb
}  // namespace core
}  // namespace fastonosql
