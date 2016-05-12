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

#include "common/string_util.h"

namespace fastonosql {

FastoObject::FastoObject(FastoObject* parent, common::Value* val,
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
  return convertToString(value_.get(), delemitr());
}

FastoObject* FastoObject::createRoot(const std::string& text, IFastoObjectObserver* observer) {
  FastoObject* root =  new FastoObject(NULL, common::Value::createStringValue(text),
                                       std::string(), std::string());
  root->observer_ = observer;
  return root;
}

FastoObject::child_container_t FastoObject::childrens() const {
  return childrens_;
}

void FastoObject::addChildren(FastoObject* child) {
  if (child) {
    DCHECK(child->parent_ == this);
    childrens_.push_back(child);
    if (observer_) {
      observer_->addedChildren(child);
      child->observer_ = observer_;
    }
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

common::Value* FastoObject::value() const {
  return value_.get();
}

void FastoObject::setValue(common::Value* val) {
  value_.reset(val);
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
    std::pair<std::string, std::string> kv = getKeyValueFromLine(command->inputCommand());
    return kv.first;
  }

  return std::string();
}

std::string FastoObjectCommand::inputArgs() const {
  common::CommandValue* command = cmd();
  if (command) {
    std::pair<std::string, std::string> kv = getKeyValueFromLine(command->inputCommand());
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

std::string stableCommand(std::string command) {
  if (command.empty()) {
    return std::string();
  }

  if(command[command.size() - 1] == '\r'){
    command.pop_back();
  }

  return command;
}

std::pair<std::string, std::string> getKeyValueFromLine(const std::string& input) {
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
  return std::make_pair(key, value);
}

std::string getFirstWordFromLine(const std::string& input) {
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
  return convertToString(ar, delemitr());
}

common::ArrayValue* FastoObjectArray::array() const {
  return static_cast<common::ArrayValue*>(value_.get());
}

}

namespace common {

std::string convertToString(fastonosql::FastoObject* obj) {
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
    result += convertToString(*it);
  }

  return result;
}

std::string convertToString(common::Value* value, const std::string& delemitr) {
  if (!value) {
    return std::string();
  }

  common::Value::Type t = value->type();
  if (t == common::Value::TYPE_ARRAY) {
    return convertToString(static_cast<ArrayValue*>(value), delemitr);
  } else if(t == common::Value::TYPE_SET) {
    return convertToString(static_cast<SetValue*>(value), delemitr);
  } else if(t == common::Value::TYPE_ZSET) {
    return convertToString(static_cast<ZSetValue*>(value), delemitr);
  } else if(t == common::Value::TYPE_HASH) {
    return convertToString(static_cast<HashValue*>(value), delemitr);
  } else {
    return value->toString();
  }
}

std::string convertToString(common::ArrayValue* array, const std::string& delemitr) {
  if (!array) {
    return std::string();
  }

  std::string result;
  common::ArrayValue::const_iterator lastIt = std::prev(array->end());
  for (common::ArrayValue::const_iterator it = array->begin(); it != array->end(); ++it) {
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

std::string convertToString(common::SetValue* set, const std::string& delemitr) {
  if (!set) {
    return std::string();
  }

  std::string result;
  const common::SetValue::const_iterator lastIt = std::prev(set->end());
  for (common::SetValue::const_iterator it = set->begin(); it != set->end(); ++it) {
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

std::string convertToString(common::ZSetValue* zset, const std::string& delemitr) {
  if (!zset) {
    return std::string();
  }

  std::string result;
  const common::ZSetValue::const_iterator lastIt = std::prev(zset->end());
  for (common::ZSetValue::const_iterator it = zset->begin(); it != zset->end(); ++it) {
    common::ZSetValue::value_type v = *it;
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

std::string convertToString(common::HashValue* hash, const std::string& delemitr) {
  if (!hash) {
    return std::string();
  }

  std::string result;
  for (common::HashValue::const_iterator it = hash->begin(); it != hash->end(); ++it) {
    common::HashValue::value_type v = *it;
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

}
