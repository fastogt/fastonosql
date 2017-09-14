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

#include "core/icommand_translator_base.h"

#include <common/sprintf.h>

namespace fastonosql {
namespace core {

ICommandTranslatorBase::ICommandTranslatorBase(const std::vector<CommandHolder>& commands)
    : ICommandTranslator(commands) {}

ICommandTranslatorBase::~ICommandTranslatorBase() {}

common::Error ICommandTranslatorBase::ChangeKeyTTLCommandImpl(const NKey& key,
                                                              ttl_t ttl,
                                                              command_buffer_t* cmdstring) const {
  UNUSED(key);
  UNUSED(ttl);
  UNUSED(cmdstring);

  const std::string error_msg =
      common::MemSPrintf("Sorry, but now " PROJECT_NAME_TITLE " not supported change ttl command for %s.", GetDBName());
  return common::make_error(error_msg);
}

common::Error ICommandTranslatorBase::LoadKeyTTLCommandImpl(const NKey& key, command_buffer_t* cmdstring) const {
  UNUSED(key);
  UNUSED(cmdstring);

  const std::string error_msg =
      common::MemSPrintf("Sorry, but now " PROJECT_NAME_TITLE " not supported get ttl command for %s.", GetDBName());
  return common::make_error(error_msg);
}

common::Error ICommandTranslatorBase::PublishCommandImpl(const NDbPSChannel& channel,
                                                         const std::string& message,
                                                         command_buffer_t* cmdstring) const {
  UNUSED(channel);
  UNUSED(message);
  UNUSED(cmdstring);

  const std::string error_msg =
      common::MemSPrintf("Sorry, but now " PROJECT_NAME_TITLE " not supported publish command for %s.", GetDBName());
  return common::make_error(error_msg);
}

common::Error ICommandTranslatorBase::SubscribeCommandImpl(const NDbPSChannel& channel,
                                                           command_buffer_t* cmdstring) const {
  UNUSED(channel);
  UNUSED(cmdstring);

  const std::string error_msg =
      common::MemSPrintf("Sorry, but now " PROJECT_NAME_TITLE " not supported subscribe command for %s.", GetDBName());
  return common::make_error(error_msg);
}

}  // namespace core
}  // namespace fastonosql
