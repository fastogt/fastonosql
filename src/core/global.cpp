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

#include <common/string_util.h>  // for TrimWhitespaceASCII, etc

namespace fastonosql {
namespace core {

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

  return value_->type();
}

std::string FastoObject::ToString() const {
  return ConvertToString(value_.get(), Delimiter());
}

FastoObject* FastoObject::CreateRoot(const std::string& text, IFastoObjectObserver* observer) {
  FastoObject* root =
      new FastoObject(nullptr, common::Value::createStringValue(text), std::string());
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
                                       common::CommandValue* cmd,
                                       const std::string& delimiter,
                                       core::connectionTypes type)
    : FastoObject(parent, cmd, delimiter), type_(type) {}

FastoObjectCommand::~FastoObjectCommand() {}

common::CommandValue* FastoObjectCommand::Cmd() const {
  return static_cast<common::CommandValue*>(value_.get());
}

std::string FastoObjectCommand::ToString() const {
  return std::string();
}

std::string FastoObjectCommand::InputCmd() const {
  common::CommandValue* command = Cmd();
  if (command) {
    std::pair<std::string, std::string> kv = GetKeyValueFromLine(command->inputCommand());
    return kv.first;
  }

  return std::string();
}

std::string FastoObjectCommand::InputArgs() const {
  common::CommandValue* command = Cmd();
  if (command) {
    std::pair<std::string, std::string> kv = GetKeyValueFromLine(command->inputCommand());
    return kv.second;
  }

  return std::string();
}

core::connectionTypes FastoObjectCommand::ConnectionType() const {
  return type_;
}

std::string FastoObjectCommand::InputCommand() const {
  common::CommandValue* command = Cmd();
  if (command) {
    return command->inputCommand();
  }

  return std::string();
}

common::Value::CommandLoggingType FastoObjectCommand::CommandLoggingType() const {
  common::CommandValue* command = Cmd();
  if (command) {
    return command->commandLoggingType();
  }

  return common::Value::C_UNKNOWN;
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

std::string GetFirstWordFromLine(const std::string& input) {
  if (input.empty()) {
    return std::string();
  }

  size_t pos = input.find_first_of(' ');
  if (pos != std::string::npos) {
    return input.substr(0, pos);
  }

  return input;
}

FastoObjectArray::FastoObjectArray(FastoObject* parent,
                                   common::ArrayValue* ar,
                                   const std::string& delimiter)
    : FastoObject(parent, ar, delimiter) {}

void FastoObjectArray::Append(common::Value* in_value) {
  common::ArrayValue* ar = static_cast<common::ArrayValue*>(value_.get());
  ar->append(in_value);
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

  common::Value::Type t = value->type();
  if (t == common::Value::TYPE_ARRAY) {
    return ConvertToString(static_cast<ArrayValue*>(value), delimiter);
  } else if (t == common::Value::TYPE_SET) {
    return ConvertToString(static_cast<SetValue*>(value), delimiter);
  } else if (t == common::Value::TYPE_ZSET) {
    return ConvertToString(static_cast<ZSetValue*>(value), delimiter);
  } else if (t == common::Value::TYPE_HASH) {
    return ConvertToString(static_cast<HashValue*>(value), delimiter);
  } else {
    return value->toString();
  }
}

std::string ConvertToString(common::ArrayValue* array, const std::string& delimiter) {
  if (!array) {
    return std::string();
  }

  if (array->size() == 0) {
    return "(empty list)";
  }

  std::string result;
  auto lastIt = std::prev(array->end());
  for (auto it = array->begin(); it != array->end(); ++it) {
    std::string val = (*it)->toString();
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

  if (set->size() == 0) {
    return "(empty set)";
  }

  std::string result;
  auto lastIt = std::prev(set->end());
  for (auto it = set->begin(); it != set->end(); ++it) {
    std::string val = (*it)->toString();
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

  if (zset->size() == 0) {
    return "(empty zset)";
  }

  std::string result;
  auto lastIt = std::prev(zset->end());
  for (auto it = zset->begin(); it != zset->end(); ++it) {
    auto v = *it;
    std::string key = (v.first)->toString();
    std::string val = (v.second)->toString();
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

  if (hash->size() == 0) {
    return "(empty hash)";
  }

  std::string result;
  for (auto it = hash->begin(); it != hash->end(); ++it) {
    auto v = *it;
    std::string key = (v.first)->toString();
    std::string val = (v.second)->toString();
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

}  // namespace common
