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
#include <string>

#include "common/value.h"

#define NO_TTL -1

namespace fastonosql {
namespace core {

typedef int32_t ttl_t;

class KeyInfo {
 public:
  typedef std::vector<std::string> splited_namespaces_t;
  KeyInfo(const splited_namespaces_t& splited_namespaces_and_key, const std::string& ns_separator);

  std::string key() const;
  bool hasNamespace() const;
  std::string nspace() const;
  std::string joinNamespace(size_t pos) const;
  size_t nspaceSize() const;

 private:
  splited_namespaces_t splited_namespaces_and_key_;
  std::string ns_separator_;
};

class NKey {
 public:
  explicit NKey(const std::string& key, ttl_t ttl_sec = NO_TTL);
  KeyInfo info(const std::string& ns_separator) const;

  std::string key() const;

  ttl_t ttl() const;
  void setTTL(ttl_t ttl);

 private:
  std::string key_;
  ttl_t ttl_;
};

typedef common::ValueSPtr NValue;

class NDbKValue {
 public:
  NDbKValue(const NKey& key, NValue value);

  NKey key() const;
  NValue value() const;
  common::Value::Type type() const;

  void setTTL(ttl_t ttl);
  void setValue(NValue value);

  std::string keyString() const;

 private:
  NKey key_;
  NValue value_;
};

}  // namespace core
}  // namespace fastonosql
