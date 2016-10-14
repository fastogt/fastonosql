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

#include "core/translator/icommand_translator.h"

namespace fastonosql {
namespace core {

common::Error ICommandTranslator::deleteKeyCommand(const NKey& key, std::string* cmdstring) const {
  if (!cmdstring) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  return deleteKeyCommandImpl(key, cmdstring);
}

common::Error ICommandTranslator::renameKeyCommand(const NKey& key,
                                                   const std::string& new_name,
                                                   std::string* cmdstring) const {
  if (!cmdstring) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  return renameKeyCommandImpl(key, new_name, cmdstring);
}

common::Error ICommandTranslator::createKeyCommand(const NDbKValue& key,
                                                   std::string* cmdstring) const {
  if (!cmdstring) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  return createKeyCommandImpl(key, cmdstring);
}

common::Error ICommandTranslator::loadKeyCommand(const NKey& key,
                                                 common::Value::Type type,
                                                 std::string* cmdstring) const {
  if (!cmdstring) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  return loadKeyCommandImpl(key, type, cmdstring);
}

common::Error ICommandTranslator::changeKeyTTLCommand(const NKey& key,
                                                      ttl_t ttl,
                                                      std::string* cmdstring) const {
  if (!cmdstring) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  return changeKeyTTLCommandImpl(key, ttl, cmdstring);
}

}  // namespace core
}  // namespace fastonosql
