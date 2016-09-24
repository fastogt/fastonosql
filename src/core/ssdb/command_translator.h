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

#pragma once

#include "core/icommand_translator.h"

namespace fastonosql {
namespace core {
namespace ssdb {
class CommandTranslator : public ICommandTranslator {
 public:
  typedef NKey key_t;
  typedef NDbKValue key_and_value_t;
  CommandTranslator();

 private:
  virtual common::Error createKeyCommandImpl(const key_and_value_t& key,
                                             std::string* cmdstring) const override;
  virtual common::Error loadKeyCommandImpl(const key_t& key,
                                           common::Value::Type type,
                                           std::string* cmdstring) const override;
  virtual common::Error deleteKeyCommandImpl(const key_t& key,
                                             std::string* cmdstring) const override;
  virtual common::Error changeKeyTTLCommandImpl(const key_t& key,
                                                ttl_t ttl,
                                                std::string* cmdstring) const override;
};
}
}  // namespace core
}  // namespace fastonosql
