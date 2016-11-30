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

#include "core/db/memcached/command_translator.h"

#include <common/sprintf.h>

#include "global/global.h"

#define MEMCACHED_SET_KEY_PATTERN_2ARGS_SS "SET %s 0 0 %s"
#define MEMCACHED_GET_KEY_PATTERN_1ARGS_S "GET %s"
#define MEMCACHED_DELETE_KEY_PATTERN_1ARGS_S "DEL %s"
#define MEMCACHED_RENAME_KEY_PATTERN_2ARGS_SS "RENAME %s %s"
#define MEMCACHED_CHANGE_TTL_2ARGS_SI "EXPIRE %s %d"
#define MEMCACHED_GET_TTL_1ARGS_S "TTL %s"

namespace fastonosql {
namespace core {
namespace memcached {

CommandTranslator::CommandTranslator() {}

common::Error CommandTranslator::CreateKeyCommandImpl(const NDbKValue& key,
                                                      std::string* cmdstring) const {
  std::string key_str = key.KeyString();
  std::string value_str = key.ValueString();
  *cmdstring = common::MemSPrintf(MEMCACHED_SET_KEY_PATTERN_2ARGS_SS, key_str, value_str);
  return common::Error();
}

common::Error CommandTranslator::LoadKeyCommandImpl(const NKey& key,
                                                    common::Value::Type type,
                                                    std::string* cmdstring) const {
  UNUSED(type);

  std::string key_str = key.Key();
  *cmdstring = common::MemSPrintf(MEMCACHED_GET_KEY_PATTERN_1ARGS_S, key_str);
  return common::Error();
}

common::Error CommandTranslator::DeleteKeyCommandImpl(const NKey& key,
                                                      std::string* cmdstring) const {
  std::string key_str = key.Key();
  *cmdstring = common::MemSPrintf(MEMCACHED_DELETE_KEY_PATTERN_1ARGS_S, key_str);
  return common::Error();
}

common::Error CommandTranslator::RenameKeyCommandImpl(const NKey& key,
                                                      const std::string& new_name,
                                                      std::string* cmdstring) const {
  std::string key_str = key.Key();
  *cmdstring = common::MemSPrintf(MEMCACHED_RENAME_KEY_PATTERN_2ARGS_SS, key_str, new_name);
  return common::Error();
}

common::Error CommandTranslator::ChangeKeyTTLCommandImpl(const NKey& key,
                                                         ttl_t ttl,
                                                         std::string* cmdstring) const {
  std::string key_str = key.Key();
  *cmdstring = common::MemSPrintf(MEMCACHED_CHANGE_TTL_2ARGS_SI, key_str, ttl);
  return common::Error();
}

common::Error CommandTranslator::LoadKeyTTLCommandImpl(const NKey& key,
                                                       std::string* cmdstring) const {
  std::string key_str = key.Key();
  *cmdstring = common::MemSPrintf(MEMCACHED_GET_TTL_1ARGS_S, key_str);
  return common::Error();
}

}  // namespace memcached
}  // namespace core
}  // namespace fastonosql
