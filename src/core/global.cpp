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

#include "core/global.h"

#include <stddef.h>  // for size_t

#include <iterator>       // for prev, next
#include <map>            // for _Rb_tree_const_iterator, etc
#include <unordered_map>  // for _Node_iterator, operator!=, etc

#include <common/convert2string.h>
#include <common/string_util.h>  // for TrimWhitespaceASCII, etc

namespace fastonosql {
namespace core {

FastoObject::IFastoObjectObserver::~IFastoObjectObserver() {}

FastoObject::FastoObject(FastoObject* parent, common::Value* val, const std::string& delimiter)
    : observer_(nullptr), value_(val), parent_(parent), childrens_(), delimiter_(delimiter) {
  DCHECK(value_);
}

FastoObject::~FastoObject() {
  Clear();
}

common::Value::Type FastoObject::Type() const {
  if (!value_) {
    return common::Value::TYPE_NULL;
  }

  return value_->GetType();
}

std::string FastoObject::ToString() const {
  return ConvertToString(value_.get(), Delimiter());
}

FastoObject* FastoObject::CreateRoot(const std::string& text, IFastoObjectObserver* observer) {
  FastoObject* root = new FastoObject(nullptr, common::Value::CreateStringValue(text), std::string());
  root->observer_ = observer;
  return root;
}

FastoObject::childs_t FastoObject::Childrens() const {
  return childrens_;
}

void FastoObject::AddChildren(child_t child) {
  if (!child) {
    return;
  }

  CHECK(child->parent_ == this);
  childrens_.push_back(child);
  if (observer_) {
    observer_->ChildrenAdded(child);
    child->observer_ = observer_;
  }
}

FastoObject* FastoObject::Parent() const {
  return parent_;
}

void FastoObject::Clear() {
  childrens_.clear();
}

std::string FastoObject::Delimiter() const {
  return delimiter_;
}

FastoObject::value_t FastoObject::Value() const {
  return value_;
}

void FastoObject::SetValue(value_t val) {
  value_ = val;
  if (observer_) {
    observer_->Updated(this, val);
  }
}

FastoObjectCommand::FastoObjectCommand(FastoObject* parent,
                                       common::StringValue* cmd,
                                       CmdLoggingType ct,
                                       const std::string& delimiter,
                                       core::connectionTypes type)
    : FastoObject(parent, cmd, delimiter), type_(type), ct_(ct) {}

FastoObjectCommand::~FastoObjectCommand() {}

std::string FastoObjectCommand::ToString() const {
  return std::string();
}

std::string FastoObjectCommand::InputCmd() const {
  std::string input_cmd;
  if (value_->GetAsString(&input_cmd)) {
    std::pair<std::string, std::string> kv = GetKeyValueFromLine(input_cmd);
    return kv.first;
  }

  return std::string();
}

std::string FastoObjectCommand::InputArgs() const {
  std::string input_cmd;
  if (value_->GetAsString(&input_cmd)) {
    std::pair<std::string, std::string> kv = GetKeyValueFromLine(input_cmd);
    return kv.second;
  }

  return std::string();
}

core::connectionTypes FastoObjectCommand::ConnectionType() const {
  return type_;
}

std::string FastoObjectCommand::InputCommand() const {
  std::string input_cmd;
  if (value_->GetAsString(&input_cmd)) {
    return input_cmd;
  }

  return std::string();
}

CmdLoggingType FastoObjectCommand::CommandLoggingType() const {
  return ct_;
}

std::pair<std::string, std::string> GetKeyValueFromLine(const std::string& input) {
  if (input.empty()) {
    return std::pair<std::string, std::string>();
  }

  size_t pos = input.find_first_of(' ');
  std::string key = input;
  std::string value;
  if (pos != std::string::npos) {
    key = input.substr(0, pos);
    value = input.substr(pos + 1);
  }

  std::string trimed;
  common::TrimWhitespaceASCII(value, common::TRIM_ALL, &trimed);
  return std::make_pair(key, trimed);
}

FastoObjectArray::FastoObjectArray(FastoObject* parent, common::ArrayValue* ar, const std::string& delimiter)
    : FastoObject(parent, ar, delimiter) {}

void FastoObjectArray::Append(common::Value* in_value) {
  common::ArrayValue* ar = static_cast<common::ArrayValue*>(value_.get());
  ar->Append(in_value);
}

std::string FastoObjectArray::ToString() const {
  common::ArrayValue* ar = Array();
  return ConvertToString(ar, Delimiter());
}

common::ArrayValue* FastoObjectArray::Array() const {
  return static_cast<common::ArrayValue*>(value_.get());
}

}  // namespace core
}  // namespace fastonosql

namespace common {

std::string ConvertToString(fastonosql::core::FastoObject* obj) {
  if (!obj) {
    return std::string();
  }

  std::string result;
  std::string str = obj->ToString();
  if (!str.empty()) {
    result += str + obj->Delimiter();
  }

  auto childrens = obj->Childrens();
  for (auto it = childrens.begin(); it != childrens.end(); ++it) {
    result += ConvertToString((*it).get());
  }

  return result;
}

std::string ConvertToString(common::Value* value, const std::string& delimiter) {
  if (!value) {
    return std::string();
  }

  common::Value::Type t = value->GetType();
  if (t == common::Value::TYPE_NULL) {
    return "(nil)";
  } else if (t == common::Value::TYPE_BOOLEAN) {
    return ConvertToString(static_cast<FundamentalValue*>(value), delimiter);
  } else if (t == common::Value::TYPE_INTEGER) {
    return ConvertToString(static_cast<FundamentalValue*>(value), delimiter);
  } else if (t == common::Value::TYPE_UINTEGER) {
    return ConvertToString(static_cast<FundamentalValue*>(value), delimiter);
  } else if (t == common::Value::TYPE_LONG_INTEGER) {
    return ConvertToString(static_cast<FundamentalValue*>(value), delimiter);
  } else if (t == common::Value::TYPE_ULONG_INTEGER) {
    return ConvertToString(static_cast<FundamentalValue*>(value), delimiter);
  } else if (t == common::Value::TYPE_LONG_LONG_INTEGER) {
    return ConvertToString(static_cast<FundamentalValue*>(value), delimiter);
  } else if (t == common::Value::TYPE_ULONG_LONG_INTEGER) {
    return ConvertToString(static_cast<FundamentalValue*>(value), delimiter);
  } else if (t == common::Value::TYPE_DOUBLE) {
    return ConvertToString(static_cast<FundamentalValue*>(value), delimiter);

  } else if (t == common::Value::TYPE_STRING) {
    return ConvertToString(static_cast<StringValue*>(value), delimiter);

  } else if (t == common::Value::TYPE_ARRAY) {
    return ConvertToString(static_cast<ArrayValue*>(value), delimiter);
  } else if (t == common::Value::TYPE_BYTE_ARRAY) {
    return ConvertToString(static_cast<ByteArrayValue*>(value), delimiter);

  } else if (t == common::Value::TYPE_SET) {
    return ConvertToString(static_cast<SetValue*>(value), delimiter);
  } else if (t == common::Value::TYPE_ZSET) {
    return ConvertToString(static_cast<ZSetValue*>(value), delimiter);
  } else if (t == common::Value::TYPE_HASH) {
    return ConvertToString(static_cast<HashValue*>(value), delimiter);
  } else {
    DNOTREACHED();
    return std::string();
  }
}

std::string ConvertToString(common::ArrayValue* array, const std::string& delimiter) {
  if (!array) {
    return std::string();
  }

  if (array->GetSize() == 0) {
    return "(empty list)";
  }

  std::string result;
  auto lastIt = std::prev(array->end());
  for (auto it = array->begin(); it != array->end(); ++it) {
    std::string val = ConvertToString((*it), delimiter);
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

std::string ConvertToString(common::SetValue* set, const std::string& delimiter) {
  if (!set) {
    return std::string();
  }

  if (set->GetSize() == 0) {
    return "(empty set)";
  }

  std::string result;
  auto lastIt = std::prev(set->end());
  for (auto it = set->begin(); it != set->end(); ++it) {
    std::string val = ConvertToString((*it), delimiter);
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

std::string ConvertToString(common::ZSetValue* zset, const std::string& delimiter) {
  if (!zset) {
    return std::string();
  }

  if (zset->GetSize() == 0) {
    return "(empty zset)";
  }

  std::string result;
  auto lastIt = std::prev(zset->end());
  for (auto it = zset->begin(); it != zset->end(); ++it) {
    auto v = *it;
    std::string key = ConvertToString((v.first), delimiter);
    std::string val = ConvertToString((v.second), delimiter);
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

std::string ConvertToString(common::HashValue* hash, const std::string& delimiter) {
  if (!hash) {
    return std::string();
  }

  if (hash->GetSize() == 0) {
    return "(empty hash)";
  }

  std::string result;
  for (auto it = hash->begin(); it != hash->end(); ++it) {
    auto v = *it;
    std::string key = ConvertToString((v.first), delimiter);
    std::string val = ConvertToString((v.second), delimiter);
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

std::string ConvertToString(FundamentalValue* value, const std::string& delimiter) {
  UNUSED(delimiter);
  const Value::Type t = value->GetType();
  if (t == Value::TYPE_BOOLEAN) {
    bool res;
    if (!value->GetAsBoolean(&res)) {
      return std::string();
    }
    return common::ConvertToString(res);
  } else if (t == Value::TYPE_INTEGER) {
    int res;
    if (!value->GetAsInteger(&res)) {
      return std::string();
    }
    return common::ConvertToString(res);
  } else if (t == Value::TYPE_UINTEGER) {
    unsigned int res;
    if (!value->GetAsUInteger(&res)) {
      return std::string();
    }
    return common::ConvertToString(res);
  } else if (t == Value::TYPE_LONG_INTEGER) {
    long res;
    if (!value->GetAsLongInteger(&res)) {
      return std::string();
    }
    return common::ConvertToString(res);
  } else if (t == Value::TYPE_ULONG_INTEGER) {
    unsigned long res;
    if (!value->GetAsULongInteger(&res)) {
      return std::string();
    }
    return common::ConvertToString(res);
  } else if (t == Value::TYPE_LONG_LONG_INTEGER) {
    long long res;
    if (!value->GetAsLongLongInteger(&res)) {
      return std::string();
    }
    return common::ConvertToString(res);
  } else if (t == Value::TYPE_ULONG_LONG_INTEGER) {
    unsigned long long res;
    if (!value->GetAsULongLongInteger(&res)) {
      return std::string();
    }
    return common::ConvertToString(res);
  } else if (t == Value::TYPE_DOUBLE) {
    double res;
    if (!value->GetAsDouble(&res)) {
      return std::string();
    }
    return common::ConvertToString(res);
  }
  return std::string();
}

std::string ConvertToString(StringValue* value, const std::string& delimiter) {
  UNUSED(delimiter);
  std::string res;
  if (!value->GetAsString(&res)) {
    return std::string();
  }

  return res;
}

std::string ConvertToString(ByteArrayValue* value, const std::string& delimiter) {
  UNUSED(delimiter);
  byte_array_t res;
  if (!value->GetAsByteArray(&res)) {
    return std::string();
  }

  return common::ConvertToString(res);
}

}  // namespace common
