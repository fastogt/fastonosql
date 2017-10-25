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

#include "core/icommand_translator.h"

namespace fastonosql {
namespace core {

class ICommandTranslatorBase : public ICommandTranslator {
 public:
  explicit ICommandTranslatorBase(const std::vector<CommandHolder>& commands,
                                  const std::vector<CommandHolder>& extended_commands);
  virtual ~ICommandTranslatorBase();

  virtual const char* GetDBName() const override = 0;

 private:
  virtual common::Error ChangeKeyTTLCommandImpl(const NKey& key, ttl_t ttl, command_buffer_t* cmdstring) const override;
  virtual common::Error LoadKeyTTLCommandImpl(const NKey& key, command_buffer_t* cmdstring) const override;
  virtual common::Error PublishCommandImpl(const NDbPSChannel& channel,
                                           const std::string& message,
                                           command_buffer_t* cmdstring) const override;
  virtual common::Error SubscribeCommandImpl(const NDbPSChannel& channel, command_buffer_t* cmdstring) const override;
};

}  // namespace core
}  // namespace fastonosql
