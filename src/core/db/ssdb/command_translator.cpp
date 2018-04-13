/*  Copyright (C) 2014-2018 FastoGT. All right reserved.

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

#include "core/connection_types.h"

#define SSDB_SET_KEY_COMMAND DB_SET_KEY_COMMAND
#define SSDB_SET_KEY_ARRAY_COMMAND "LPUSH"
#define SSDB_SET_KEY_SET_COMMAND "SADD"
#define SSDB_SET_KEY_ZSET_COMMAND "ZADD"
#define SSDB_SET_KEY_HASH_COMMAND "HMSET"

#define SSDB_GET_KEY_COMMAND DB_GET_KEY_COMMAND
#define SSDB_GET_KEY_ARRAY_COMMAND "LRANGE"
#define SSDB_GET_KEY_SET_COMMAND "SMEMBERS"
#define SSDB_GET_KEY_ZSET_COMMAND "ZRANGE"
#define SSDB_GET_KEY_HASH_COMMAND "HGET"

#define SSDB_DELETE_KEY_COMMAND DB_DELETE_KEY_COMMAND
#define SSDB_RENAME_KEY_COMMAND DB_RENAME_KEY_COMMAND
#define SSDB_CHANGE_TTL_COMMAND DB_SET_TTL_COMMAND
#define SSDB_GET_TTL_COMMAND DB_GET_TTL_COMMAND

namespace fastonosql {
namespace core {
namespace ssdb {

CommandTranslator::CommandTranslator(const std::vector<CommandHolder>& commands) : ICommandTranslatorBase(commands) {}

const char* CommandTranslator::GetDBName() const {
  return ConnectionTraits<SSDB>::GetDBName();
}

common::Error CommandTranslator::CreateKeyCommandImpl(const NDbKValue& key, command_buffer_t* cmdstring) const {
  command_buffer_writer_t wr;
  const NKey cur = key.GetKey();
  key_t key_str = cur.GetKey();
  NValue value = key.GetValue();
  value_t value_str = value.GetValue();
  common::Value::Type type = key.GetType();
  if (type == common::Value::TYPE_ARRAY) {
    wr << SSDB_SET_KEY_ARRAY_COMMAND " " << key_str.GetForCommandLine() << " " << value_str.GetForCommandLine();
  } else if (type == common::Value::TYPE_SET) {
    wr << SSDB_SET_KEY_SET_COMMAND " " << key_str.GetForCommandLine() << " " << value_str.GetForCommandLine();
  } else if (type == common::Value::TYPE_ZSET) {
    wr << SSDB_SET_KEY_ZSET_COMMAND " " << key_str.GetForCommandLine() << " " << value_str.GetForCommandLine();
  } else if (type == common::Value::TYPE_HASH) {
    wr << SSDB_SET_KEY_HASH_COMMAND " " << key_str.GetForCommandLine() << " " << value_str.GetForCommandLine();
  } else {
    wr << SSDB_SET_KEY_COMMAND " " << key_str.GetForCommandLine() << " " << value_str.GetForCommandLine();
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
    wr << SSDB_GET_KEY_ARRAY_COMMAND " " << key_str.GetForCommandLine() << " 0 -1";
  } else if (type == common::Value::TYPE_SET) {
    wr << SSDB_GET_KEY_SET_COMMAND " " << key_str.GetForCommandLine();
  } else if (type == common::Value::TYPE_ZSET) {
    wr << SSDB_GET_KEY_ZSET_COMMAND " " << key_str.GetForCommandLine() << " 0 -1";
  } else if (type == common::Value::TYPE_HASH) {
    wr << SSDB_GET_KEY_HASH_COMMAND " " << key_str.GetForCommandLine();
  } else {
    wr << SSDB_GET_KEY_COMMAND " " << key_str.GetForCommandLine();
  }

  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::DeleteKeyCommandImpl(const NKey& key, command_buffer_t* cmdstring) const {
  key_t key_str = key.GetKey();
  command_buffer_writer_t wr;
  wr << SSDB_DELETE_KEY_COMMAND " " << key_str.GetForCommandLine();
  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::RenameKeyCommandImpl(const NKey& key,
                                                      const key_t& new_name,
                                                      command_buffer_t* cmdstring) const {
  key_t key_str = key.GetKey();
  command_buffer_writer_t wr;
  wr << SSDB_RENAME_KEY_COMMAND " " << key_str.GetForCommandLine() << " " << new_name.GetForCommandLine();
  *cmdstring = wr.str();
  return common::Error();
}

bool CommandTranslator::IsLoadKeyCommandImpl(const CommandInfo& cmd) const {
  return cmd.IsEqualName(SSDB_GET_KEY_COMMAND) || cmd.IsEqualName(SSDB_GET_KEY_ARRAY_COMMAND) ||
         cmd.IsEqualName(SSDB_GET_KEY_SET_COMMAND) || cmd.IsEqualName(SSDB_GET_KEY_ZSET_COMMAND) ||
         cmd.IsEqualName(SSDB_GET_KEY_HASH_COMMAND);
}

common::Error CommandTranslator::ChangeKeyTTLCommandImpl(const NKey& key,
                                                         ttl_t ttl,
                                                         command_buffer_t* cmdstring) const {
  key_t key_str = key.GetKey();
  command_buffer_writer_t wr;
  wr << SSDB_CHANGE_TTL_COMMAND " " << key_str.GetForCommandLine() << " " << ttl;
  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::LoadKeyTTLCommandImpl(const NKey& key, command_buffer_t* cmdstring) const {
  key_t key_str = key.GetKey();
  command_buffer_writer_t wr;
  wr << SSDB_GET_TTL_COMMAND " " << key_str.GetForCommandLine();
  *cmdstring = wr.str();
  return common::Error();
}

}  // namespace ssdb
}  // namespace core
}  // namespace fastonosql
