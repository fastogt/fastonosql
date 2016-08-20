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

#include <memory>                       // for shared_ptr
#include <string>                       // for string

#include "common/macros.h"              // for DNOTREACHED
#include "common/value.h"               // for Value, etc

#include "core/db_key.h"                // for NDbKValue, ttl_t, NValue

namespace fastonosql {
namespace core {

class CommandKey {
 public:
  enum cmdtype {
    C_DELETE,
    C_LOAD,
    C_CREATE,
    C_CHANGE_TTL
  };

  cmdtype type() const;
  NDbKValue key() const;

  virtual ~CommandKey();

 protected:
  CommandKey(const NDbKValue& key, cmdtype type);

  const cmdtype type_;
  const NDbKValue key_;
};

class CommandDeleteKey
      : public CommandKey {
 public:
  explicit CommandDeleteKey(const NDbKValue& key);
};

class CommandLoadKey
  : public CommandKey {
 public:
  explicit CommandLoadKey(const NDbKValue& key);
};

class CommandCreateKey
  : public CommandKey {
 public:
  explicit CommandCreateKey(const NDbKValue& dbv);
  NValue value() const;
};

class CommandChangeTTL
  : public CommandKey {
 public:
  CommandChangeTTL(const NDbKValue& dbv, ttl_t newTTL);
  ttl_t newTTL() const;
  NDbKValue newKey() const;

 private:
  ttl_t new_ttl_;
};

typedef common::shared_ptr<CommandKey> CommandKeySPtr;

}  // namespace core
}  // namespace fastonosql
