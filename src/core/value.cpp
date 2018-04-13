/*  Copyright (C) 2014-2018 FastoGT. All right reserved.

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

#include <json-c/json_tokener.h>

#include <common/convert2string.h>

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

StreamValue::StreamValue() : Value(TYPE_STREAM), streams_() {}

StreamValue::~StreamValue() {}

StreamValue* StreamValue::DeepCopy() const {
  StreamValue* str = new StreamValue();
  str->SetStreams(streams_);
  return str;
}

bool StreamValue::Equals(const Value* other) const {
  if (other->GetType() != GetType()) {
    return false;
  }

  std::string lhs, rhs;
  return GetAsString(&lhs) && other->GetAsString(&rhs) && lhs == rhs;
}

StreamValue::streams_t StreamValue::GetStreams() const {
  return streams_;
}

void StreamValue::SetStreams(const streams_t& streams) {
  streams_ = streams;
}

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

bool JsonValue::IsValidJson(const std::string& json) {
  if (json.empty()) {
    return false;
  }

  json_object* obj = json_tokener_parse(json.c_str());
  if (!obj) {
    return false;
  }

  json_object_put(obj);
  return true;
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

BloomValue::BloomValue() : Value(TYPE_BLOOM) {}

BloomValue::~BloomValue() {}

BloomValue* BloomValue::DeepCopy() const {
  return new BloomValue;
}

bool BloomValue::Equals(const Value* other) const {
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
    case common::Value::TYPE_NULL: {
      return common::Value::CreateNullValue();
    }
    case common::Value::TYPE_BOOLEAN: {
      return common::Value::CreateBooleanValue(false);
    }
    case common::Value::TYPE_INTEGER: {
      return common::Value::CreateIntegerValue(0);
    }
    case common::Value::TYPE_UINTEGER: {
      return common::Value::CreateUIntegerValue(0);
    }
    case common::Value::TYPE_LONG_INTEGER: {
      return common::Value::CreateLongIntegerValue(0);
    }
    case common::Value::TYPE_ULONG_INTEGER: {
      return common::Value::CreateULongIntegerValue(0);
    }
    case common::Value::TYPE_LONG_LONG_INTEGER: {
      return common::Value::CreateLongLongIntegerValue(0);
    }
    case common::Value::TYPE_ULONG_LONG_INTEGER: {
      return common::Value::CreateULongLongIntegerValue(0);
    }
    case common::Value::TYPE_DOUBLE: {
      return common::Value::CreateDoubleValue(0);
    }
    case common::Value::TYPE_STRING: {
      return common::Value::CreateStringValue(std::string());
    }
    case common::Value::TYPE_ARRAY: {
      return common::Value::CreateArrayValue();
    }
    case common::Value::TYPE_BYTE_ARRAY: {
      return common::Value::CreateByteArrayValue(common::byte_array_t());
    }
    case common::Value::TYPE_SET: {
      return common::Value::CreateSetValue();
    }
    case common::Value::TYPE_ZSET: {
      return common::Value::CreateZSetValue();
    }
    case common::Value::TYPE_HASH: {
      return common::Value::CreateHashValue();
    }
    // extended
    case StreamValue::TYPE_STREAM: {
      return new StreamValue;
    }
    case JsonValue::TYPE_JSON: {
      return new JsonValue(std::string());
    }
    case GraphValue::TYPE_GRAPH: {
      return new GraphValue;
    }
    case BloomValue::TYPE_BLOOM: {
      return new BloomValue;
    }
    case SearchValue::TYPE_FT_INDEX: {
      return SearchValue::CreateSearchIndex();
    }
    case SearchValue::TYPE_FT_TERM: {
      return SearchValue::CreateSearchDocument();
    }
  }

  return nullptr;
}

const char* GetTypeName(common::Value::Type value_type) {
  if (value_type <= common::Value::TYPE_HASH) {
    return string_types[value_type];
  } else if (value_type == StreamValue::TYPE_STREAM) {
    return "TYPE_STREAM";
  } else if (value_type == JsonValue::TYPE_JSON) {
    return "TYPE_JSON";
  } else if (value_type == GraphValue::TYPE_GRAPH) {
    return "TYPE_GRAPH";
  } else if (value_type == BloomValue::TYPE_BLOOM) {
    return "TYPE_BLOOM";
  } else if (value_type == SearchValue::TYPE_FT_INDEX) {
    return "TYPE_FT_INDEX";
  } else if (value_type == SearchValue::TYPE_FT_TERM) {
    return "TYPE_FT_TERM";
  }

  DNOTREACHED();
  return "UNKNOWN";
}

std::string ConvertValue(common::Value* value, const std::string& delimiter) {
  if (!value) {
    return std::string();
  }

  common::Value::Type t = value->GetType();
  if (t == common::Value::TYPE_NULL) {
    return "(nil)";
  } else if (t == common::Value::TYPE_BOOLEAN) {
    return ConvertValue(static_cast<common::FundamentalValue*>(value), delimiter);
  } else if (t == common::Value::TYPE_INTEGER) {
    return ConvertValue(static_cast<common::FundamentalValue*>(value), delimiter);
  } else if (t == common::Value::TYPE_UINTEGER) {
    return ConvertValue(static_cast<common::FundamentalValue*>(value), delimiter);
  } else if (t == common::Value::TYPE_LONG_INTEGER) {
    return ConvertValue(static_cast<common::FundamentalValue*>(value), delimiter);
  } else if (t == common::Value::TYPE_ULONG_INTEGER) {
    return ConvertValue(static_cast<common::FundamentalValue*>(value), delimiter);
  } else if (t == common::Value::TYPE_LONG_LONG_INTEGER) {
    return ConvertValue(static_cast<common::FundamentalValue*>(value), delimiter);
  } else if (t == common::Value::TYPE_ULONG_LONG_INTEGER) {
    return ConvertValue(static_cast<common::FundamentalValue*>(value), delimiter);
  } else if (t == common::Value::TYPE_DOUBLE) {
    return ConvertValue(static_cast<common::FundamentalValue*>(value), delimiter);

  } else if (t == common::Value::TYPE_STRING) {
    return ConvertValue(static_cast<common::StringValue*>(value), delimiter);

  } else if (t == common::Value::TYPE_ARRAY) {
    return ConvertValue(static_cast<common::ArrayValue*>(value), delimiter);
  } else if (t == common::Value::TYPE_BYTE_ARRAY) {
    return ConvertValue(static_cast<common::ByteArrayValue*>(value), delimiter);

  } else if (t == common::Value::TYPE_SET) {
    return ConvertValue(static_cast<common::SetValue*>(value), delimiter);
  } else if (t == common::Value::TYPE_ZSET) {
    return ConvertValue(static_cast<common::ZSetValue*>(value), delimiter);
  } else if (t == common::Value::TYPE_HASH) {
    return ConvertValue(static_cast<common::HashValue*>(value), delimiter);
  } else if (t == StreamValue::TYPE_STREAM) {
    return ConvertValue(static_cast<StreamValue*>(value), delimiter);
    // extended
  } else if (t == JsonValue::TYPE_JSON) {
    return ConvertValue(static_cast<JsonValue*>(value), delimiter);
  } else if (t == GraphValue::TYPE_GRAPH) {
    return ConvertValue(static_cast<GraphValue*>(value), delimiter);
  } else if (t == BloomValue::TYPE_BLOOM) {
    return ConvertValue(static_cast<BloomValue*>(value), delimiter);
  } else if (t == SearchValue::TYPE_FT_INDEX) {
    return ConvertValue(static_cast<SearchValue*>(value), delimiter);
  } else if (t == SearchValue::TYPE_FT_TERM) {
    return ConvertValue(static_cast<SearchValue*>(value), delimiter);
  }

  DNOTREACHED();
  return std::string();
}

std::string ConvertValue(common::ArrayValue* array, const std::string& delimiter) {
  if (!array) {
    return std::string();
  }

  std::string result;
  auto lastIt = std::prev(array->end());
  for (auto it = array->begin(); it != array->end(); ++it) {
    common::Value* cur_val = *it;
    std::string val_str = ConvertValue(cur_val, delimiter);
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

std::string ConvertValue(common::SetValue* set, const std::string& delimiter) {
  if (!set) {
    return std::string();
  }

  std::string result;
  auto lastIt = std::prev(set->end());
  for (auto it = set->begin(); it != set->end(); ++it) {
    std::string val = ConvertValue((*it), delimiter);
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

std::string ConvertValue(common::ZSetValue* zset, const std::string& delimiter) {
  if (!zset) {
    return std::string();
  }

  std::string result;
  auto lastIt = std::prev(zset->end());
  for (auto it = zset->begin(); it != zset->end(); ++it) {
    auto v = *it;
    std::string key = ConvertValue((v.first), delimiter);
    std::string val = ConvertValue((v.second), delimiter);
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

std::string ConvertValue(common::HashValue* hash, const std::string& delimiter) {
  if (!hash) {
    return std::string();
  }

  std::string result;
  for (auto it = hash->begin(); it != hash->end(); ++it) {
    auto v = *it;
    std::string key = ConvertValue((v.first), delimiter);
    std::string val = ConvertValue((v.second), delimiter);
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

std::string ConvertValue(common::FundamentalValue* value, const std::string& delimiter) {
  UNUSED(delimiter);
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

std::string ConvertValue(common::StringValue* value, const std::string& delimiter) {
  UNUSED(delimiter);
  if (!value) {
    return std::string();
  }

  std::string res;
  if (!value->GetAsString(&res)) {
    return std::string();
  }

  return res;
}

std::string ConvertValue(common::ByteArrayValue* value, const std::string& delimiter) {
  UNUSED(delimiter);
  if (!value) {
    return std::string();
  }

  common::byte_array_t res;
  if (!value->GetAsByteArray(&res)) {
    return std::string();
  }

  return common::ConvertToString(res);
}

std::string ConvertValue(StreamValue* value, const std::string& delimiter) {
  if (!value) {
    return std::string();
  }

  std::ostringstream wr;
  StreamValue::streams_t streams = value->GetStreams();
  for (size_t i = 0; i < streams.size(); ++i) {
    StreamValue::Stream cur_str = streams[i];
    if (i != 0) {
      wr << delimiter;
    }

    wr << streams[i].id_;
    for (size_t j = 0; j < cur_str.entries_.size(); ++j) {
      wr << " " << cur_str.entries_[i].name << " " << cur_str.entries_[i].value;
    }
  }

  return wr.str();
}

std::string ConvertValue(JsonValue* value, const std::string& delimiter) {
  UNUSED(delimiter);
  if (!value) {
    return std::string();
  }

  std::string res;
  if (!value->GetAsString(&res)) {
    return std::string();
  }

  return res;
}

std::string ConvertValue(GraphValue* value, const std::string& delimiter) {
  UNUSED(value);
  UNUSED(delimiter);
  return std::string();
}

std::string ConvertValue(BloomValue* value, const std::string& delimiter) {
  UNUSED(value);
  UNUSED(delimiter);
  return std::string();
}

std::string ConvertValue(SearchValue* value, const std::string& delimiter) {
  UNUSED(value);
  UNUSED(delimiter);
  return std::string();
}

}  // namespace core
}  // namespace fastonosql
