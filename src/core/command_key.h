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

#include <vector>
#include <utility>
#include <string>

#include "global/global.h"
#include "core/db_key.h"

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

template<typename Command>
FastoObjectCommand* createCommand(FastoObject* parent, const std::string& input,
                                  common::Value::CommandLoggingType ct) {
  if (!parent) {
    DNOTREACHED();
    return nullptr;
  }

  std::string stable_input = stableCommand(input);
  if (stable_input.empty()) {
    DNOTREACHED();
    return nullptr;
  }

  common::CommandValue* cmd = common::Value::createCommand(stable_input, ct);
  FastoObjectCommand* fs = new Command(parent, cmd, parent->delemitr(), parent->nsSeparator());
  parent->addChildren(fs);
  return fs;
}

template<typename Command>
FastoObjectCommand* createCommand(FastoObjectIPtr parent, const std::string& input,
                                  common::Value::CommandLoggingType ct) {
  return createCommand<Command>(parent.get(), input, ct);
}

}  // namespace core
}  // namespace fastonosql
