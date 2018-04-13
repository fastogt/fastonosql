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

#include "core/db/redis_compatible/command_translator.h"

#include "core/connection_types.h"
#include "core/db/redis/internal/modules.h"
#include "core/value.h"

#define REDIS_SET_KEY_COMMAND DB_SET_KEY_COMMAND
#define REDIS_SET_KEY_ARRAY_COMMAND "LPUSH"
#define REDIS_SET_KEY_SET_COMMAND "SADD"
#define REDIS_SET_KEY_ZSET_COMMAND "ZADD"
#define REDIS_SET_KEY_HASH_COMMAND "HMSET"
#define REDIS_SET_KEY_STREAM_COMMAND "XADD"
#define REDIS_SET_KEY_JSON_COMMAND "JSON.SET"

#define REDIS_GET_KEY_COMMAND DB_GET_KEY_COMMAND
#define REDIS_GET_KEY_ARRAY_COMMAND "LRANGE"
#define REDIS_GET_KEY_SET_COMMAND "SMEMBERS"
#define REDIS_GET_KEY_ZSET_COMMAND "ZRANGE"
#define REDIS_GET_KEY_HASH_COMMAND "HGETALL"
#define REDIS_GET_KEY_STREAM_COMMAND "XRANGE"
#define REDIS_GET_KEY_JSON_COMMAND "JSON.GET"

#define REDIS_DELETE_KEY_COMMAND DB_DELETE_KEY_COMMAND
#define REDIS_RENAME_KEY_COMMAND DB_RENAME_KEY_COMMAND
#define REDIS_CHANGE_TTL_COMMAND DB_SET_TTL_COMMAND
#define REDIS_PERSIST_KEY_COMMAND "PERSIST"

#define REDIS_GET_TTL_COMMAND DB_GET_TTL_COMMAND
#define REDIS_PUBLISH_COMMAND DB_PUBLISH_COMMAND
#define REDIS_SUBSCRIBE_COMMAND DB_SUBSCRIBE_COMMAND

#define REDIS_ZRANGE "ZRANGE"

#define REDIS_MGET "MGET"
#define REDIS_MSETNX "MSETNX"
#define REDIS_MSET "MSET"
#define REDIS_HGETALL "HGETALL"

#define REDIS_SMEMBERS "SMEMBERS"

#define REDIS_LRANGE "LRANGE"

#define REDIS_SETEX "SETEX"
#define REDIS_SETNX "SETNX"

#define REDIS_DECR "DECR"
#define REDIS_DECRBY "DECRBY"

#define REDIS_INCR "INCR"
#define REDIS_INCRBY "INCRBY"
#define REDIS_INCRBYFLOAT "INCRBYFLOAT"

#define REDIS_MODULE_LOAD "MODULE LOAD"
#define REDIS_MODULE_UNLOAD "MODULE UNLOAD"

namespace fastonosql {
namespace core {
namespace redis_compatible {

CommandTranslator::CommandTranslator(const std::vector<CommandHolder>& commands) : ICommandTranslator(commands) {}

const char* CommandTranslator::GetDBName() const {
  return ConnectionTraits<REDIS>::GetDBName();
}

common::Error CommandTranslator::Zrange(const NKey& key,
                                        int start,
                                        int stop,
                                        bool withscores,
                                        command_buffer_t* cmdstring) {
  key_t key_str = key.GetKey();
  command_buffer_writer_t wr;
  wr << REDIS_ZRANGE " " << key_str.GetForCommandLine() << " " << start << " " << stop;
  if (withscores) {
    wr << " WITHSCORES";
  }

  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::Hgetall(const NKey& key, command_buffer_t* cmdstring) {
  if (!cmdstring) {
    return common::make_error_inval();
  }

  key_t key_str = key.GetKey();
  command_buffer_writer_t wr;
  wr << REDIS_HGETALL " " << key_str.GetForCommandLine();
  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::Mset(const std::vector<NDbKValue>& keys, command_buffer_t* cmdstring) {
  if (!cmdstring) {
    return common::make_error_inval();
  }

  command_buffer_writer_t wr;
  wr << REDIS_MSET;
  for (size_t i = 0; i < keys.size(); ++i) {
    NKey key = keys[i].GetKey();
    key_t key_str = key.GetKey();
    NValue value = keys[i].GetValue();
    value_t value_str = value.GetValue();
    wr << " " << key_str.GetForCommandLine() << " " << value_str.GetForCommandLine();
  }
  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::MsetNX(const std::vector<NDbKValue>& keys, command_buffer_t* cmdstring) {
  if (!cmdstring) {
    return common::make_error_inval();
  }

  command_buffer_writer_t wr;
  wr << REDIS_MSETNX;
  for (size_t i = 0; i < keys.size(); ++i) {
    NKey key = keys[i].GetKey();
    key_t key_str = key.GetKey();
    NValue value = keys[i].GetValue();
    value_t value_str = value.GetValue();
    wr << " " << key_str.GetForCommandLine() << " " << value_str.GetForCommandLine();
  }
  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::Mget(const std::vector<NKey>& keys, command_buffer_t* cmdstring) {
  if (keys.empty() || !cmdstring) {
    return common::make_error_inval();
  }

  command_buffer_writer_t wr;
  wr << REDIS_MGET;
  for (size_t i = 0; i < keys.size(); ++i) {
    key_t key_str = keys[i].GetKey();
    wr << " " << key_str.GetForCommandLine();
  }
  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::Smembers(const NKey& key, command_buffer_t* cmdstring) {
  if (!cmdstring) {
    return common::make_error_inval();
  }

  key_t key_str = key.GetKey();
  command_buffer_writer_t wr;
  wr << REDIS_SMEMBERS " " << key_str.GetForCommandLine();
  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::Lrange(const NKey& key, int start, int stop, command_buffer_t* cmdstring) {
  if (!cmdstring) {
    return common::make_error_inval();
  }

  key_t key_str = key.GetKey();
  command_buffer_writer_t wr;
  wr << REDIS_LRANGE " " << key_str.GetForCommandLine() << " " << start << " " << stop;
  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::SetEx(const NDbKValue& key, ttl_t ttl, command_buffer_t* cmdstring) {
  if (!cmdstring) {
    return common::make_error_inval();
  }

  const NKey cur = key.GetKey();
  key_t key_str = cur.GetKey();
  NValue value = key.GetValue();
  value_t value_str = value.GetValue();
  command_buffer_writer_t wr;
  wr << REDIS_SETEX " " << key_str.GetForCommandLine() << " " << ttl << " " << value_str.GetForCommandLine();
  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::SetNX(const NDbKValue& key, command_buffer_t* cmdstring) {
  if (!cmdstring) {
    return common::make_error_inval();
  }

  const NKey cur = key.GetKey();
  key_t key_str = cur.GetKey();
  NValue value = key.GetValue();
  value_t value_str = value.GetValue();
  command_buffer_writer_t wr;
  wr << REDIS_SETNX " " << key_str.GetForCommandLine() << " " << value_str.GetForCommandLine();
  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::Decr(const NKey& key, command_buffer_t* cmdstring) {
  if (!cmdstring) {
    return common::make_error_inval();
  }

  key_t key_str = key.GetKey();
  command_buffer_writer_t wr;
  wr << REDIS_DECR " " << key_str.GetForCommandLine();
  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::DecrBy(const NKey& key, int inc, command_buffer_t* cmdstring) {
  if (!cmdstring) {
    return common::make_error_inval();
  }

  key_t key_str = key.GetKey();
  command_buffer_writer_t wr;
  wr << REDIS_DECRBY " " << key_str.GetForCommandLine() << " " << inc;
  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::Incr(const NKey& key, command_buffer_t* cmdstring) {
  if (!cmdstring) {
    return common::make_error_inval();
  }

  key_t key_str = key.GetKey();
  command_buffer_writer_t wr;
  wr << REDIS_INCR " " << key_str.GetForCommandLine();
  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::IncrBy(const NKey& key, int inc, command_buffer_t* cmdstring) {
  if (!cmdstring) {
    return common::make_error_inval();
  }

  key_t key_str = key.GetKey();
  command_buffer_writer_t wr;
  wr << REDIS_INCRBY " " << key_str.GetForCommandLine() << " " << inc;
  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::IncrByFloat(const NKey& key, double inc, command_buffer_t* cmdstring) {
  if (!cmdstring) {
    return common::make_error_inval();
  }

  key_t key_str = key.GetKey();
  command_buffer_writer_t wr;
  wr << REDIS_INCRBYFLOAT " " << key_str.GetForCommandLine() << " " << inc;
  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::ModuleLoad(const ModuleInfo& module, command_buffer_t* cmdstring) {
  if (!cmdstring) {
    return common::make_error_inval();
  }

  command_buffer_writer_t wr;
  wr << REDIS_MODULE_LOAD " " << module.name;
  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::ModuleUnload(const ModuleInfo& module, command_buffer_t* cmdstring) {
  if (!cmdstring) {
    return common::make_error_inval();
  }

  command_buffer_writer_t wr;
  wr << REDIS_MODULE_UNLOAD " " << module.name;
  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::PExpire(const NKey& key, ttl_t ttl, command_buffer_t* cmdstring) const {
  key_t key_str = key.GetKey();
  command_buffer_writer_t wr;
  if (ttl == NO_TTL) {
    wr << REDIS_PERSIST_KEY_COMMAND " " << key_str.GetForCommandLine();
  } else {
    wr << REDIS_CHANGE_PTTL_COMMAND " " << key_str.GetForCommandLine() << " " << ttl;
  }

  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::PTTL(const NKey& key, command_buffer_t* cmdstring) const {
  key_t key_str = key.GetKey();
  command_buffer_writer_t wr;
  wr << REDIS_GET_PTTL_COMMAND " " << key_str.GetForCommandLine();
  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::CreateKeyCommandImpl(const NDbKValue& key, command_buffer_t* cmdstring) const {
  const NKey cur = key.GetKey();
  key_t key_str = cur.GetKey();
  NValue value = key.GetValue();
  value_t value_str = value.GetValue();
  command_buffer_writer_t wr;
  common::Value::Type type = key.GetType();

  if (type == common::Value::TYPE_ARRAY) {
    wr << REDIS_SET_KEY_ARRAY_COMMAND " " << key_str.GetForCommandLine() << " " << value_str.GetForCommandLine();
  } else if (type == common::Value::TYPE_SET) {
    wr << REDIS_SET_KEY_SET_COMMAND " " << key_str.GetForCommandLine() << " " << value_str.GetForCommandLine();
  } else if (type == common::Value::TYPE_ZSET) {
    wr << REDIS_SET_KEY_ZSET_COMMAND " " << key_str.GetForCommandLine() << " " << value_str.GetForCommandLine();
  } else if (type == common::Value::TYPE_HASH) {
    wr << REDIS_SET_KEY_HASH_COMMAND " " << key_str.GetForCommandLine() << " " << value_str.GetForCommandLine();
  } else if (type == StreamValue::TYPE_STREAM) {  // XADD is complex
    NValue nv = key.GetValue();
    StreamValue* value = static_cast<StreamValue*>(nv.get());
    StreamValue::streams_t streams = value->GetStreams();
    for (size_t i = 0; i < streams.size(); ++i) {
      StreamValue::Stream cur_str = streams[i];
      wr << REDIS_SET_KEY_STREAM_COMMAND " " << key_str.GetForCommandLine() << " " << cur_str.id_;
      for (size_t j = 0; j < cur_str.entries_.size(); ++j) {
        wr << " " << cur_str.entries_[j].name << " " << cur_str.entries_[j].value;
      }
      if (i != streams.size() - 1) {
        wr << "\n";
      }
    }
  } else if (type == JsonValue::TYPE_JSON) {
    wr << REDIS_SET_KEY_JSON_COMMAND " " << key_str.GetForCommandLine() << " . " << value_str.GetForCommandLine();
  } else if (type == GraphValue::TYPE_GRAPH) {
    return NotSupported(REDIS_GRAPH_MODULE_COMMAND("SET"));
  } else if (type == BloomValue::TYPE_BLOOM) {
    return NotSupported(REDIS_BLOOM_MODULE_COMMAND("SET"));
  } else if (type == SearchValue::TYPE_FT_INDEX) {
    return NotSupported(REDIS_SEARCH_MODULE_COMMAND("INDEX.SET"));
  } else if (type == SearchValue::TYPE_FT_TERM) {
    return NotSupported(REDIS_SEARCH_MODULE_COMMAND("TERM.SET"));
  } else {
    wr << REDIS_SET_KEY_COMMAND " " << key_str.GetForCommandLine() << " " << value_str.GetForCommandLine();
  }

  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::LoadKeyCommandImpl(const NKey& key,
                                                    common::Value::Type type,
                                                    command_buffer_t* cmdstring) const {
  key_t key_str = key.GetKey();
  command_buffer_writer_t wr;
  if (type == common::Value::TYPE_ARRAY) {
    wr << REDIS_GET_KEY_ARRAY_COMMAND " " << key_str.GetForCommandLine() << " 0 -1";
  } else if (type == JsonValue::TYPE_JSON) {
    wr << REDIS_GET_KEY_JSON_COMMAND " " << key_str.GetForCommandLine();
  } else if (type == common::Value::TYPE_SET) {
    wr << REDIS_GET_KEY_SET_COMMAND " " << key_str.GetForCommandLine();
  } else if (type == common::Value::TYPE_ZSET) {
    wr << REDIS_GET_KEY_ZSET_COMMAND " " << key_str.GetForCommandLine() << " 0 -1 WITHSCORES";
  } else if (type == common::Value::TYPE_HASH) {
    wr << REDIS_GET_KEY_HASH_COMMAND " " << key_str.GetForCommandLine();
  } else if (type == StreamValue::TYPE_STREAM) {
    wr << REDIS_GET_KEY_STREAM_COMMAND " " << key_str.GetForCommandLine() << " - +";
  } else if (type == GraphValue::TYPE_GRAPH) {
    return NotSupported(REDIS_GRAPH_MODULE_COMMAND("GET"));
  } else if (type == BloomValue::TYPE_BLOOM) {
    return NotSupported(REDIS_BLOOM_MODULE_COMMAND("GET"));
  } else if (type == SearchValue::TYPE_FT_INDEX) {
    return NotSupported(REDIS_SEARCH_MODULE_COMMAND("INDEX.GET"));
  } else if (type == SearchValue::TYPE_FT_TERM) {
    return NotSupported(REDIS_SEARCH_MODULE_COMMAND("DOC.GET"));
  } else {
    wr << REDIS_GET_KEY_COMMAND " " << key_str.GetForCommandLine();
  }

  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::DeleteKeyCommandImpl(const NKey& key, command_buffer_t* cmdstring) const {
  key_t key_str = key.GetKey();
  command_buffer_writer_t wr;
  wr << REDIS_DELETE_KEY_COMMAND " " << key_str.GetForCommandLine();
  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::RenameKeyCommandImpl(const NKey& key,
                                                      const key_t& new_name,
                                                      command_buffer_t* cmdstring) const {
  key_t key_str = key.GetKey();
  command_buffer_writer_t wr;
  wr << REDIS_RENAME_KEY_COMMAND " " << key_str.GetForCommandLine() << " " << new_name.GetForCommandLine();
  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::ChangeKeyTTLCommandImpl(const NKey& key,
                                                         ttl_t ttl,
                                                         command_buffer_t* cmdstring) const {
  key_t key_str = key.GetKey();
  command_buffer_writer_t wr;
  if (ttl == NO_TTL) {
    wr << REDIS_PERSIST_KEY_COMMAND " " << key_str.GetForCommandLine();
  } else {
    wr << REDIS_CHANGE_TTL_COMMAND " " << key_str.GetForCommandLine() << " " << ttl;
  }

  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::LoadKeyTTLCommandImpl(const NKey& key, command_buffer_t* cmdstring) const {
  key_t key_str = key.GetKey();
  command_buffer_writer_t wr;
  wr << REDIS_GET_TTL_COMMAND " " << key_str.GetForCommandLine();
  *cmdstring = wr.str();
  return common::Error();
}

bool CommandTranslator::IsLoadKeyCommandImpl(const CommandInfo& cmd) const {
  return cmd.IsEqualName(REDIS_GET_KEY_COMMAND) || cmd.IsEqualName(REDIS_GET_KEY_ARRAY_COMMAND) ||
         cmd.IsEqualName(REDIS_GET_KEY_SET_COMMAND) || cmd.IsEqualName(REDIS_GET_KEY_ZSET_COMMAND) ||
         cmd.IsEqualName(REDIS_GET_KEY_HASH_COMMAND) || cmd.IsEqualName(REDIS_GET_KEY_JSON_COMMAND);
}

common::Error CommandTranslator::PublishCommandImpl(const NDbPSChannel& channel,
                                                    const std::string& message,
                                                    command_buffer_t* cmdstring) const {
  std::string channel_str = channel.GetName();
  command_buffer_writer_t wr;
  wr << REDIS_PUBLISH_COMMAND " " << channel_str << " " << message;
  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::SubscribeCommandImpl(const NDbPSChannel& channel, command_buffer_t* cmdstring) const {
  std::string channel_str = channel.GetName();
  command_buffer_writer_t wr;
  wr << REDIS_SUBSCRIBE_COMMAND " " << channel_str;
  *cmdstring = wr.str();
  return common::Error();
}

}  // namespace redis_compatible
}  // namespace core
}  // namespace fastonosql
