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

#include "core/db/memcached/command_translator.h"

#include <common/convert2string.h>

#include "core/global.h"

#define MEMCACHED_GET_KEY_COMMAND COMMONTYPE_GET_KEY_COMMAND
#define MEMCACHED_SET_KEY_COMMAND COMMONTYPE_SET_KEY_COMMAND
#define MEMCACHED_DELETE_KEY_COMMAND "DEL"
#define MEMCACHED_RENAME_KEY_COMMAND "RENAME"
#define MEMCACHED_CHANGE_TTL_COMMAND "EXPIRE"
#define MEMCACHED_GET_TTL_COMMAND "TTL"

namespace fastonosql {
namespace core {
namespace memcached {

CommandTranslator::CommandTranslator(const std::vector<CommandHolder>& commands) : ICommandTranslator(commands) {}

common::Error CommandTranslator::CreateKeyCommandImpl(const NDbKValue& key, command_buffer_t* cmdstring) const {
  const NKey cur = key.GetKey();
  key_t key_str = cur.GetKey();
  std::string value_str = key.ValueString();
  command_buffer_writer_t wr;
  wr << MAKE_COMMAND_BUFFER(MEMCACHED_SET_KEY_COMMAND) << MAKE_COMMAND_BUFFER(" ") << key_str.GetKey()
     << MAKE_COMMAND_BUFFER(" ") << value_str;
  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::LoadKeyCommandImpl(const NKey& key,
                                                    common::Value::Type type,
                                                    command_buffer_t* cmdstring) const {
  UNUSED(type);

  key_t key_str = key.GetKey();
  command_buffer_writer_t wr;
  wr << MAKE_COMMAND_BUFFER(MEMCACHED_GET_KEY_COMMAND) << MAKE_COMMAND_BUFFER(" ") << key_str.GetKey();
  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::DeleteKeyCommandImpl(const NKey& key, command_buffer_t* cmdstring) const {
  key_t key_str = key.GetKey();
  command_buffer_writer_t wr;
  wr << MAKE_COMMAND_BUFFER(MEMCACHED_DELETE_KEY_COMMAND) << MAKE_COMMAND_BUFFER(" ") << key_str.GetKey();
  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::RenameKeyCommandImpl(const NKey& key,
                                                      const string_key_t& new_name,
                                                      command_buffer_t* cmdstring) const {
  key_t key_str = key.GetKey();
  command_buffer_writer_t wr;
  wr << MAKE_COMMAND_BUFFER(MEMCACHED_RENAME_KEY_COMMAND) << MAKE_COMMAND_BUFFER(" ") << key_str.GetKey()
     << MAKE_COMMAND_BUFFER(" ") << new_name;
  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::ChangeKeyTTLCommandImpl(const NKey& key,
                                                         ttl_t ttl,
                                                         command_buffer_t* cmdstring) const {
  key_t key_str = key.GetKey();
  command_buffer_writer_t wr;
  wr << MAKE_COMMAND_BUFFER(MEMCACHED_CHANGE_TTL_COMMAND) << MAKE_COMMAND_BUFFER(" ") << key_str.GetKey()
     << MAKE_COMMAND_BUFFER(" ") << common::ConvertToString(ttl);
  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::LoadKeyTTLCommandImpl(const NKey& key, command_buffer_t* cmdstring) const {
  key_t key_str = key.GetKey();
  command_buffer_writer_t wr;
  wr << MAKE_COMMAND_BUFFER(MEMCACHED_GET_TTL_COMMAND) << MAKE_COMMAND_BUFFER(" ") << key_str.GetKey();
  *cmdstring = wr.str();
  return common::Error();
}

bool CommandTranslator::IsLoadKeyCommandImpl(const CommandInfo& cmd) const {
  return cmd.IsEqualName(MEMCACHED_GET_KEY_COMMAND);
}

common::Error CommandTranslator::PublishCommandImpl(const NDbPSChannel& channel,
                                                    const std::string& message,
                                                    command_buffer_t* cmdstring) const {
  UNUSED(channel);
  UNUSED(message);
  UNUSED(cmdstring);

  std::string errorMsg =
      common::MemSPrintf("Sorry, but now " PROJECT_NAME_TITLE " not supported publish command for Memcached.");
  return common::make_error_value(errorMsg, common::ErrorValue::E_ERROR);
}

common::Error CommandTranslator::SubscribeCommandImpl(const NDbPSChannel& channel, command_buffer_t* cmdstring) const {
  UNUSED(channel);
  UNUSED(cmdstring);

  std::string errorMsg =
      common::MemSPrintf("Sorry, but now " PROJECT_NAME_TITLE " not supported subscribe command for Memcached.");
  return common::make_error_value(errorMsg, common::ErrorValue::E_ERROR);
}

}  // namespace memcached
}  // namespace core
}  // namespace fastonosql
