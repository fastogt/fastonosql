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

#include <limits>

#include <common/value.h>  // for Value, Value::Type, etc

#include "core/types.h"

#define NO_TTL -1
#define EXPIRED_TTL -2

namespace fastonosql {
namespace core {

typedef long long ttl_t;
COMPILE_ASSERT(std::numeric_limits<ttl_t>::max() >= NO_TTL && NO_TTL >= std::numeric_limits<ttl_t>::min(),
               "NO_TTL define must be in ttl type range");
COMPILE_ASSERT(std::numeric_limits<ttl_t>::max() >= EXPIRED_TTL && EXPIRED_TTL >= std::numeric_limits<ttl_t>::min(),
               "EXPIRED_TTL define must be in ttl type range");

typedef command_buffer_t string_key_t;

bool IsBinaryKey(const command_buffer_t& key);

class KeyString {
 public:
  enum KeyType { TEXT_KEY = 0, BINARY_KEY };

  KeyString();
  explicit KeyString(const string_key_t& key_data);

  KeyType GetType() const;

  std::string ToBytes() const;      // for direct bytes call
  std::string ToString() const;     // for diplaying
  string_key_t GetKeyData() const;  // escape if hex, or not changed if text
  void SetKeyData(const string_key_t& key_data);

  bool Equals(const KeyString& other) const;

 private:
  string_key_t key_;
  KeyType type_;
};

inline bool operator==(const KeyString& r, const KeyString& l) {
  return r.Equals(l);
}

inline bool operator!=(const KeyString& r, const KeyString& l) {
  return !r.Equals(l);
}

typedef KeyString key_t;

class NKey {
 public:
  NKey();
  explicit NKey(key_t key, ttl_t ttl_sec = NO_TTL);

  key_t GetKey() const;
  void SetKey(key_t key);

  ttl_t GetTTL() const;
  void SetTTL(ttl_t ttl);

  bool Equals(const NKey& other) const;

 private:
  key_t key_;
  ttl_t ttl_;
};

inline bool operator==(const NKey& r, const NKey& l) {
  return r.Equals(l);
}

typedef std::vector<NKey> NKeys;
typedef common::ValueSPtr NValue;

class NDbKValue {
 public:
  NDbKValue();
  NDbKValue(const NKey& key, NValue value);

  NKey GetKey() const;
  NValue GetValue() const;
  common::Value::Type GetType() const;

  void SetKey(const NKey& key);
  void SetValue(NValue value);

  std::string ValueString() const;

  bool Equals(const NDbKValue& other) const;

 private:
  NKey key_;
  NValue value_;
};

inline bool operator==(const NDbKValue& r, const NDbKValue& l) {
  return r.Equals(l);
}

typedef std::vector<NDbKValue> NDbKValues;

}  // namespace core
}  // namespace fastonosql
