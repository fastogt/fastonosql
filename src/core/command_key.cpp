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

#include "core/command_key.h"

#include <vector>
#include <string>

#include "common/string_util.h"
#include "common/sprintf.h"

namespace fastonosql {
namespace core {

CommandKey::CommandKey(const NDbKValue &key, cmdtype type)
  : type_(type), key_(key) {
}

CommandKey::cmdtype CommandKey::type() const {
  return type_;
}

NDbKValue CommandKey::key() const {
  return key_;
}

CommandKey::~CommandKey() {
}

CommandDeleteKey::CommandDeleteKey(const NDbKValue &key)
  : CommandKey(key, C_DELETE) {
}

CommandLoadKey::CommandLoadKey(const NDbKValue &key)
  : CommandKey(key, C_LOAD) {
}

CommandCreateKey::CommandCreateKey(const NDbKValue& dbv)
  : CommandKey(dbv, C_CREATE) {
}

CommandChangeTTL::CommandChangeTTL(const NDbKValue& dbv, ttl_t newTTL)
  : CommandKey(dbv, C_CHANGE_TTL), new_ttl_(newTTL) {
}

ttl_t CommandChangeTTL::newTTL() const {
  return new_ttl_;
}

NDbKValue CommandChangeTTL::newKey() const {
  NDbKValue nk = key();
  nk.setTTL(new_ttl_);
  return nk;
}

NValue CommandCreateKey::value() const {
  return key_.value();
}

}  // namespace core
}  // namespace fastonosql
