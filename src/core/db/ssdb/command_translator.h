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

#pragma once

#include <string>  // for string

#include <common/error.h>  // for Error
#include <common/value.h>  // for Value, Value::Type

#include "core/db_key.h"               // for NDbKValue, NKey, ttl_t
#include "core/icommand_translator.h"  // for ICommandTranslator

namespace fastonosql {
namespace core {
namespace ssdb {

class CommandTranslator : public ICommandTranslator {
 public:
  explicit CommandTranslator(const std::vector<CommandHolder>& commands);

 private:
  virtual common::Error CreateKeyCommandImpl(const NDbKValue& key, command_buffer_t* cmdstring) const override;
  virtual common::Error LoadKeyCommandImpl(const NKey& key,
                                           common::Value::Type type,
                                           command_buffer_t* cmdstring) const override;
  virtual common::Error DeleteKeyCommandImpl(const NKey& key, command_buffer_t* cmdstring) const override;
  virtual common::Error RenameKeyCommandImpl(const NKey& key,
                                             const string_key_t& new_name,
                                             command_buffer_t* cmdstring) const override;
  virtual common::Error ChangeKeyTTLCommandImpl(const NKey& key, ttl_t ttl, command_buffer_t* cmdstring) const override;
  virtual common::Error LoadKeyTTLCommandImpl(const NKey& key, command_buffer_t* cmdstring) const override;

  virtual bool IsLoadKeyCommandImpl(const CommandInfo& cmd) const override;

  virtual common::Error PublishCommandImpl(const NDbPSChannel& channel,
                                           const std::string& message,
                                           command_buffer_t* cmdstring) const override;
  virtual common::Error SubscribeCommandImpl(const NDbPSChannel& channel, command_buffer_t* cmdstring) const override;
};

}  // namespace ssdb
}  // namespace core
}  // namespace fastonosql
