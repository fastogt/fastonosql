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

#include "global/global.h"

#include <stddef.h>                     // for size_t, NULL
#include <iterator>                     // for prev, next
#include <map>                          // for _Rb_tree_const_iterator, etc
#include <unordered_map>                // for _Node_iterator, operator!=, etc
#include "common/string_util.h"         // for TrimWhitespaceASCII, etc

namespace fastonosql {

FastoObject::FastoObject(FastoObject* parent, common::Value *val,
                         const std::string& delemitr, const std::string& ns_separator)
  : observer_(nullptr), value_(val), parent_(parent), childrens_(),
    delemitr_(delemitr), ns_separator_(ns_separator) {
  DCHECK(value_);
}

FastoObject::~FastoObject() {
  for (size_t i = 0; i < childrens_.size(); ++i) {
    FastoObject* item = childrens_[i];
    delete item;
  }
  childrens_.clear();
}

common::Value::Type FastoObject::type() const {
  if (!value_) {
    return common::Value::TYPE_NULL;
  }

  return value_->type();
}

std::string FastoObject::toString() const {
  return ConvertToString(value_.get(), delemitr());
}

FastoObject* FastoObject::createRoot(const std::string& text, IFastoObjectObserver* observer) {
  FastoObject* root =  new FastoObject(NULL, common::Value::createStringValue(text),
                                       std::string(), std::string());
  root->observer_ = observer;
  return root;
}

FastoObject::childs_t FastoObject::childrens() const {
  return childrens_;
}

void FastoObject::addChildren(FastoObject* child) {
  if (!child) {
    return;
  }

  CHECK(child->parent_ == this);
  childrens_.push_back(child);
  if (observer_) {
    observer_->addedChildren(child);
    child->observer_ = observer_;
  }
}

FastoObject* FastoObject::parent() const {
  return parent_;
}

void FastoObject::clear() {
  for (auto it = childrens_.begin(); it != childrens_.end(); ++it) {
    FastoObject* child = (*it);
    delete child;
  }
  childrens_.clear();
}

std::string FastoObject::delemitr() const {
  return delemitr_;
}

std::string FastoObject::nsSeparator() const {
  return ns_separator_;
}

FastoObject::value_t FastoObject::value() const {
  return value_;
}

void FastoObject::setValue(value_t val) {
  value_ = val;
  if (observer_) {
    observer_->updated(this, val);
  }
}

FastoObjectCommand::FastoObjectCommand(FastoObject* parent, common::CommandValue* cmd,
                                       const std::string& delemitr, const std::string& ns_separator)
  : FastoObject(parent, cmd, delemitr, ns_separator) {
}

FastoObjectCommand::~FastoObjectCommand() {
}

common::CommandValue* FastoObjectCommand::cmd() const {
  return static_cast<common::CommandValue*>(value_.get());
}

std::string FastoObjectCommand::toString() const {
  return std::string();
}

std::string FastoObjectCommand::inputCmd() const {
  common::CommandValue* command = cmd();
  if (command) {
    std::pair<std::string, std::string> kv = GetKeyValueFromLine(command->inputCommand());
    return kv.first;
  }

  return std::string();
}

std::string FastoObjectCommand::inputArgs() const {
  common::CommandValue* command = cmd();
  if (command) {
    std::pair<std::string, std::string> kv = GetKeyValueFromLine(command->inputCommand());
    return kv.second;
  }

  return std::string();
}

std::string FastoObjectCommand::inputCommand() const {
  common::CommandValue* command = cmd();
  if (command) {
    return command->inputCommand();
  }

  return std::string();
}

common::Value::CommandLoggingType FastoObjectCommand::commandLoggingType() const {
  common::CommandValue* command = cmd();
  if (command) {
    return command->commandLoggingType();
  }

  return common::Value::C_UNKNOWN;
}

std::string StableCommand(std::string command) {
  if (command.empty()) {
    return std::string();
  }

  if(command[command.size() - 1] == '\r'){
    command.pop_back();
  }

  return command;
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

FastoObjectArray::FastoObjectArray(FastoObject* parent, common::ArrayValue* ar,
                                   const std::string& delemitr, const std::string& ns_separator)
  : FastoObject(parent, ar, delemitr, ns_separator) {
}

void FastoObjectArray::append(common::Value* in_value) {
  common::ArrayValue* ar = static_cast<common::ArrayValue*>(value_.get());
  ar->append(in_value);
}

std::string FastoObjectArray::toString() const {
  common::ArrayValue* ar = array();
  return ConvertToString(ar, delemitr());
}

common::ArrayValue* FastoObjectArray::array() const {
  return static_cast<common::ArrayValue*>(value_.get());
}

}  // namespace fastonosql

namespace common {

std::string ConvertToString(fastonosql::FastoObject* obj) {
  if (!obj) {
    return std::string();
  }

  std::string result;
  std::string str = obj->toString();
  if (!str.empty()) {
    result += str + obj->delemitr();
  }

  auto childrens = obj->childrens();
  for(auto it = childrens.begin(); it != childrens.end(); ++it ){
    result += ConvertToString(*it);
  }

  return result;
}

std::string ConvertToString(common::Value* value, const std::string& delemitr) {
  if (!value) {
    return std::string();
  }

  common::Value::Type t = value->type();
  if (t == common::Value::TYPE_ARRAY) {
    return ConvertToString(static_cast<ArrayValue*>(value), delemitr);
  } else if(t == common::Value::TYPE_SET) {
    return ConvertToString(static_cast<SetValue*>(value), delemitr);
  } else if(t == common::Value::TYPE_ZSET) {
    return ConvertToString(static_cast<ZSetValue*>(value), delemitr);
  } else if(t == common::Value::TYPE_HASH) {
    return ConvertToString(static_cast<HashValue*>(value), delemitr);
  } else {
    return value->toString();
  }
}

std::string ConvertToString(common::ArrayValue* array, const std::string& delemitr) {
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
    if(val.empty()){
      continue;
    }

    result += val;
    if(lastIt != it){
      result += delemitr;
    }
  }

  return result;
}

std::string ConvertToString(common::SetValue* set, const std::string& delemitr) {
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
    if(val.empty()){
      continue;
    }

    result += val;
    if (lastIt != it) {
      result += delemitr;
    }
  }

  return result;
}

std::string ConvertToString(common::ZSetValue* zset, const std::string& delemitr) {
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
    if(val.empty() || key.empty()){
      continue;
    }

    result += key + " " + val;
    if(lastIt != it){
      result += delemitr;
    }
  }
  return result;
}

std::string ConvertToString(common::HashValue* hash, const std::string& delemitr) {
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
      result += delemitr;
    }
  }
  return result;
}

}  // namespace common
