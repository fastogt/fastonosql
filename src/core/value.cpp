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

#include "core/value.h"

#include <algorithm>

#include <common/convert2string.h>
#include <common/utils.h>

namespace {

template <typename T>
std::string string_from_hex_impl(const T& value) {
  size_t len = value.size();
  if (len % 4 != 0) {
    return std::string();
  }

  std::string hex_digits;
  for (size_t i = 0; i < len; i += 4) {
    auto c1 = value[i];
    auto c2 = value[i + 1];
    if (c1 == '\\' && c2 == 'x') {
      hex_digits += value[i + 2];
      hex_digits += value[i + 3];
    } else {
      return std::string();
    }
  }

  return common::utils::hex::decode(hex_digits);
}

template <typename T>
std::string hex_string_impl(const T& value) {
  std::ostringstream wr;
  auto hexed = common::utils::hex::encode(value, true);
  for (size_t i = 0; i < hexed.size(); i += 2) {
    wr << "\\x";
    wr << hexed[i];
    wr << hexed[i + 1];
  }

  return wr.str();
}

}  // namespace

namespace fastonosql {
namespace core {
namespace {
const char* string_types[] = {"TYPE_NULL",
                              "TYPE_BOOLEAN",
                              "TYPE_INTEGER",
                              "TYPE_UINTEGER",
                              "TYPE_LONG_INTEGER",
                              "TYPE_ULONG_INTEGER",
                              "TYPE_LONG_LONG_INTEGER",
                              "TYPE_ULONG_LONG_INTEGER",
                              "TYPE_DOUBLE",
                              "TYPE_STRING",
                              "TYPE_ARRAY",
                              "TYPE_BYTE_ARRAY",
                              "TYPE_SET",
                              "TYPE_ZSET",
                              "TYPE_HASH"};
static_assert(arraysize(string_types) == static_cast<size_t>(common::Value::Type::TYPE_HASH) + 1,
              "string_types Has Wrong Size");
}  // namespace

JsonValue::JsonValue(const std::string& json_value) : Value(TYPE_JSON), value_(json_value) {}

JsonValue::~JsonValue() {}

bool JsonValue::GetAsString(std::string* out_value) const {
  if (out_value) {
    *out_value = value_;
  }

  return true;
}

JsonValue* JsonValue::DeepCopy() const {
  return new JsonValue(value_);
}

bool JsonValue::Equals(const Value* other) const {
  if (other->GetType() != GetType()) {
    return false;
  }

  std::string lhs, rhs;
  return GetAsString(&lhs) && other->GetAsString(&rhs) && lhs == rhs;
}

GraphValue::GraphValue() : Value(TYPE_GRAPH) {}

GraphValue::~GraphValue() {}

GraphValue* GraphValue::DeepCopy() const {
  return new GraphValue;
}

bool GraphValue::Equals(const Value* other) const {
  if (other->GetType() != GetType()) {
    return false;
  }

  return true;
}

SearchValue* SearchValue::CreateSearchIndex() {
  return new SearchValue(TYPE_FT_INDEX);
}

SearchValue* SearchValue::CreateSearchDocument() {
  return new SearchValue(TYPE_FT_TERM);
}

SearchValue::~SearchValue() {}

SearchValue* SearchValue::DeepCopy() const {
  return new SearchValue(GetType());
}

bool SearchValue::Equals(const Value* other) const {
  if (other->GetType() != GetType()) {
    return false;
  }

  return true;
}

SearchValue::SearchValue(common::Value::Type type) : Value(type) {}

common::Value* CreateEmptyValueFromType(common::Value::Type value_type) {
  const uint8_t cvalue_type = value_type;
  switch (cvalue_type) {
    case common::Value::TYPE_NULL:
      return common::Value::CreateNullValue();
    case common::Value::TYPE_BOOLEAN:
      return common::Value::CreateBooleanValue(false);
    case common::Value::TYPE_INTEGER:
      return common::Value::CreateIntegerValue(0);
    case common::Value::TYPE_UINTEGER:
      return common::Value::CreateUIntegerValue(0);
    case common::Value::TYPE_LONG_INTEGER:
      return common::Value::CreateLongIntegerValue(0);
    case common::Value::TYPE_ULONG_INTEGER:
      return common::Value::CreateULongIntegerValue(0);
    case common::Value::TYPE_LONG_LONG_INTEGER:
      return common::Value::CreateLongLongIntegerValue(0);
    case common::Value::TYPE_ULONG_LONG_INTEGER:
      return common::Value::CreateULongLongIntegerValue(0);
    case common::Value::TYPE_DOUBLE:
      return common::Value::CreateDoubleValue(0);
    case common::Value::TYPE_STRING:
      return common::Value::CreateStringValue(std::string());
    case common::Value::TYPE_ARRAY:
      return common::Value::CreateArrayValue();
    case common::Value::TYPE_BYTE_ARRAY:
      return common::Value::CreateByteArrayValue(common::byte_array_t());
    case common::Value::TYPE_SET:
      return common::Value::CreateSetValue();
    case common::Value::TYPE_ZSET:
      return common::Value::CreateZSetValue();
    case common::Value::TYPE_HASH:
      return common::Value::CreateHashValue();
    // extended
    case JsonValue::TYPE_JSON:
      return new JsonValue(std::string());
    case GraphValue::TYPE_GRAPH:
      return new GraphValue;
    case SearchValue::TYPE_FT_INDEX:
      return SearchValue::CreateSearchIndex();
    case SearchValue::TYPE_FT_TERM:
      return SearchValue::CreateSearchDocument();
  }

  return nullptr;
}

const char* GetTypeName(common::Value::Type value_type) {
  if (value_type <= common::Value::TYPE_HASH) {
    return string_types[value_type];
  } else if (value_type == JsonValue::TYPE_JSON) {
    return "TYPE_JSON";
  } else if (value_type == GraphValue::TYPE_GRAPH) {
    return "TYPE_GRAPH";
  } else if (value_type == SearchValue::TYPE_FT_INDEX) {
    return "TYPE_FT_INDEX";
  } else if (value_type == SearchValue::TYPE_FT_TERM) {
    return "TYPE_FT_TERM";
  }

  DNOTREACHED();
  return "UNKNOWN";
}

namespace detail {
bool have_space(const std::string& data) {
  auto it = std::find_if(data.begin(), data.end(), [](char c) { return std::isspace(c); });
  return it != data.end();
}

std::string hex_string(const common::buffer_t& value) {
  return hex_string_impl(value);
}

std::string hex_string(const std::string& value) {
  return hex_string_impl(value);
}

std::string string_from_hex(const common::buffer_t& value) {
  return string_from_hex_impl(value);
}

std::string string_from_hex(const std::string& value) {
  return string_from_hex_impl(value);
}

}  // namespace detail

std::string ConvertValue(common::Value* value, const std::string& delimiter, bool for_cmd) {
  if (!value) {
    return std::string();
  }

  common::Value::Type t = value->GetType();
  if (t == common::Value::TYPE_NULL) {
    return std::string();
  } else if (t == common::Value::TYPE_BOOLEAN) {
    return ConvertValue(static_cast<common::FundamentalValue*>(value), delimiter, for_cmd);
  } else if (t == common::Value::TYPE_INTEGER) {
    return ConvertValue(static_cast<common::FundamentalValue*>(value), delimiter, for_cmd);
  } else if (t == common::Value::TYPE_UINTEGER) {
    return ConvertValue(static_cast<common::FundamentalValue*>(value), delimiter, for_cmd);
  } else if (t == common::Value::TYPE_LONG_INTEGER) {
    return ConvertValue(static_cast<common::FundamentalValue*>(value), delimiter, for_cmd);
  } else if (t == common::Value::TYPE_ULONG_INTEGER) {
    return ConvertValue(static_cast<common::FundamentalValue*>(value), delimiter, for_cmd);
  } else if (t == common::Value::TYPE_LONG_LONG_INTEGER) {
    return ConvertValue(static_cast<common::FundamentalValue*>(value), delimiter, for_cmd);
  } else if (t == common::Value::TYPE_ULONG_LONG_INTEGER) {
    return ConvertValue(static_cast<common::FundamentalValue*>(value), delimiter, for_cmd);
  } else if (t == common::Value::TYPE_DOUBLE) {
    return ConvertValue(static_cast<common::FundamentalValue*>(value), delimiter, for_cmd);

  } else if (t == common::Value::TYPE_STRING) {
    return ConvertValue(static_cast<common::StringValue*>(value), delimiter, for_cmd);

  } else if (t == common::Value::TYPE_ARRAY) {
    return ConvertValue(static_cast<common::ArrayValue*>(value), delimiter, for_cmd);
  } else if (t == common::Value::TYPE_BYTE_ARRAY) {
    return ConvertValue(static_cast<common::ByteArrayValue*>(value), delimiter, for_cmd);

  } else if (t == common::Value::TYPE_SET) {
    return ConvertValue(static_cast<common::SetValue*>(value), delimiter, for_cmd);
  } else if (t == common::Value::TYPE_ZSET) {
    return ConvertValue(static_cast<common::ZSetValue*>(value), delimiter, for_cmd);
  } else if (t == common::Value::TYPE_HASH) {
    return ConvertValue(static_cast<common::HashValue*>(value), delimiter, for_cmd);
    // extended
  } else if (t == JsonValue::TYPE_JSON) {
    return ConvertValue(static_cast<JsonValue*>(value), delimiter, for_cmd);
  } else if (t == GraphValue::TYPE_GRAPH) {
    return std::string();
  } else if (t == SearchValue::TYPE_FT_INDEX) {
    return std::string();
  } else if (t == SearchValue::TYPE_FT_TERM) {
    return std::string();
  }

  DNOTREACHED();
  return std::string();
}

std::string ConvertValue(common::ArrayValue* array, const std::string& delimiter, bool for_cmd) {
  if (!array) {
    return std::string();
  }

  std::string result;
  auto lastIt = std::prev(array->end());
  for (auto it = array->begin(); it != array->end(); ++it) {
    common::Value* cur_val = *it;
    std::string val_str = ConvertValue(cur_val, delimiter, for_cmd);
    if (val_str.empty()) {
      continue;
    }

    result += val_str;
    if (lastIt != it) {
      result += delimiter;
    }
  }

  return result;
}

std::string ConvertValue(common::SetValue* set, const std::string& delimiter, bool for_cmd) {
  if (!set) {
    return std::string();
  }

  std::string result;
  auto lastIt = std::prev(set->end());
  for (auto it = set->begin(); it != set->end(); ++it) {
    std::string val = ConvertValue((*it), delimiter, for_cmd);
    if (val.empty()) {
      continue;
    }

    result += val;
    if (lastIt != it) {
      result += delimiter;
    }
  }

  return result;
}

std::string ConvertValue(common::ZSetValue* zset, const std::string& delimiter, bool for_cmd) {
  if (!zset) {
    return std::string();
  }

  std::string result;
  auto lastIt = std::prev(zset->end());
  for (auto it = zset->begin(); it != zset->end(); ++it) {
    auto v = *it;
    std::string key = ConvertValue((v.first), delimiter, for_cmd);
    std::string val = ConvertValue((v.second), delimiter, for_cmd);
    if (val.empty() || key.empty()) {
      continue;
    }

    result += key + " " + val;
    if (lastIt != it) {
      result += delimiter;
    }
  }
  return result;
}

std::string ConvertValue(common::HashValue* hash, const std::string& delimiter, bool for_cmd) {
  if (!hash) {
    return std::string();
  }

  std::string result;
  for (auto it = hash->begin(); it != hash->end(); ++it) {
    auto v = *it;
    std::string key = ConvertValue((v.first), delimiter, for_cmd);
    std::string val = ConvertValue((v.second), delimiter, for_cmd);
    if (val.empty() || key.empty()) {
      continue;
    }

    result += key + " " + val;
    if (std::next(it) != hash->end()) {
      result += delimiter;
    }
  }
  return result;
}

std::string ConvertValue(common::FundamentalValue* value, const std::string& delimiter, bool for_cmd) {
  UNUSED(delimiter);
  UNUSED(for_cmd);
  if (!value) {
    return std::string();
  }

  const common::Value::Type t = value->GetType();
  if (t == common::Value::TYPE_BOOLEAN) {
    bool res;
    if (!value->GetAsBoolean(&res)) {
      return std::string();
    }
    return common::ConvertToString(res);
  } else if (t == common::Value::TYPE_INTEGER) {
    int res;
    if (!value->GetAsInteger(&res)) {
      return std::string();
    }
    return common::ConvertToString(res);
  } else if (t == common::Value::TYPE_UINTEGER) {
    unsigned int res;
    if (!value->GetAsUInteger(&res)) {
      return std::string();
    }
    return common::ConvertToString(res);
  } else if (t == common::Value::TYPE_LONG_INTEGER) {
    long res;
    if (!value->GetAsLongInteger(&res)) {
      return std::string();
    }
    return common::ConvertToString(res);
  } else if (t == common::Value::TYPE_ULONG_INTEGER) {
    unsigned long res;
    if (!value->GetAsULongInteger(&res)) {
      return std::string();
    }
    return common::ConvertToString(res);
  } else if (t == common::Value::TYPE_LONG_LONG_INTEGER) {
    long long res;
    if (!value->GetAsLongLongInteger(&res)) {
      return std::string();
    }
    return common::ConvertToString(res);
  } else if (t == common::Value::TYPE_ULONG_LONG_INTEGER) {
    unsigned long long res;
    if (!value->GetAsULongLongInteger(&res)) {
      return std::string();
    }
    return common::ConvertToString(res);
  } else if (t == common::Value::TYPE_DOUBLE) {
    double res;
    if (!value->GetAsDouble(&res)) {
      return std::string();
    }
    return common::ConvertToString(res);
  }
  return std::string();
}

std::string ConvertValue(common::StringValue* value, const std::string& delimiter, bool for_cmd) {
  UNUSED(delimiter);
  if (!value) {
    return std::string();
  }

  std::string res;
  if (!value->GetAsString(&res)) {
    return std::string();
  }

  if (!for_cmd) {
    return res;
  }

  if (detail::have_space(res)) {
    return "\"" + res + "\"";
  }

  return res;
}

std::string ConvertValue(common::ByteArrayValue* value, const std::string& delimiter, bool for_cmd) {
  UNUSED(delimiter);
  if (!value) {
    return std::string();
  }

  common::byte_array_t res;
  if (!value->GetAsByteArray(&res)) {
    return std::string();
  }

  if (!for_cmd) {
    return common::ConvertToString(res);
  }

  return detail::hex_string(res);
}

std::string ConvertValue(JsonValue* value, const std::string& delimiter, bool for_cmd) {
  UNUSED(delimiter);
  if (!value) {
    return std::string();
  }

  std::string res;
  if (!value->GetAsString(&res)) {
    return std::string();
  }

  if (!for_cmd) {
    return res;
  }

  if (detail::have_space(res)) {
    return "\"" + res + "\"";
  }

  return res;
}

std::string ConvertToHumanReadable(common::Value* value, const std::string& delimiter) {
  return ConvertValue(value, delimiter, false);
}

}  // namespace core
}  // namespace fastonosql
