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

#include "core/global.h"

#include "core/value.h"

namespace fastonosql {
namespace core {

FastoObject::IFastoObjectObserver::~IFastoObjectObserver() {}

FastoObject::FastoObject(FastoObject* parent, common::Value* val, const std::string& delimiter)
    : observer_(nullptr), value_(val), parent_(parent), childrens_(), delimiter_(delimiter) {
  DCHECK(value_);
  if (parent_) {
    observer_ = parent_->observer_;
  }
}

FastoObject::~FastoObject() {
  Clear();
}

common::Value::Type FastoObject::GetType() const {
  if (!value_) {
    return common::Value::TYPE_NULL;
  }

  return value_->GetType();
}

std::string FastoObject::ToString() const {
  return ConvertValue(value_.get(), GetDelimiter());
}

FastoObject* FastoObject::CreateRoot(const command_buffer_t& text, IFastoObjectObserver* observer) {
  FastoObject* root = new FastoObject(nullptr, common::Value::CreateStringValue(text), std::string());
  root->observer_ = observer;
  return root;
}

FastoObject::childs_t FastoObject::GetChildrens() const {
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
  }
}

FastoObject* FastoObject::GetParent() const {
  return parent_;
}

void FastoObject::Clear() {
  childrens_.clear();
}

std::string FastoObject::GetDelimiter() const {
  return delimiter_;
}

FastoObject::value_t FastoObject::GetValue() const {
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

core::connectionTypes FastoObjectCommand::GetConnectionType() const {
  return type_;
}

command_buffer_t FastoObjectCommand::GetInputCommand() const {
  command_buffer_t input_cmd;
  if (value_->GetAsString(&input_cmd)) {
    return input_cmd;
  }

  return command_buffer_t();
}

CmdLoggingType FastoObjectCommand::GetCommandLoggingType() const {
  return ct_;
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
    result += str + obj->GetDelimiter();
  }

  fastonosql::core::FastoObject::childs_t childrens = obj->GetChildrens();
  for (auto it = childrens.begin(); it != childrens.end(); ++it) {
    fastonosql::core::FastoObjectIPtr val = *it;
    result += ConvertToString(val.get());
  }

  return result;
}
}  // namespace common
