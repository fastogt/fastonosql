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

#include <common/intrusive_ptr.h>  // for intrusive_ptr, etc
#include <common/value.h>

#include "core/connection_types.h"
#include "core/types.h"

namespace fastonosql {
namespace core {

enum CmdLoggingType { C_UNKNOWN, C_USER, C_INNER };

class FastoObject;
class FastoObjectCommand;

template <typename T, typename... Args>
inline common::intrusive_ptr<T> make_fasto_object(Args&&... args) {
  return common::intrusive_ptr<T>(new T(std::forward<Args>(args)...));
}

typedef common::intrusive_ptr<FastoObject> FastoObjectIPtr;
typedef common::intrusive_ptr<FastoObjectCommand> FastoObjectCommandIPtr;

class FastoObject : public common::intrusive_ptr_base<FastoObject> {
 public:
  typedef FastoObjectIPtr child_t;
  typedef std::vector<child_t> childs_t;
  typedef common::ValueSPtr value_t;

  class IFastoObjectObserver {
   public:
    virtual void ChildrenAdded(child_t child) = 0;
    virtual void Updated(FastoObject* item, value_t val) = 0;
    virtual ~IFastoObjectObserver();
  };

  FastoObject(FastoObject* parent, common::Value* val, const std::string& delimiter);  // val take ownerships
  virtual ~FastoObject();

  common::Value::Type GetType() const;
  virtual std::string ToString() const;

  static FastoObject* CreateRoot(const command_buffer_t& text, IFastoObjectObserver* observer = nullptr);

  childs_t GetChildrens() const;
  void AddChildren(child_t child);
  FastoObject* GetParent() const;
  void Clear();
  std::string GetDelimiter() const;

  value_t GetValue() const;
  void SetValue(value_t val);

 protected:
  IFastoObjectObserver* observer_;
  value_t value_;

 private:
  DISALLOW_COPY_AND_ASSIGN(FastoObject);

  FastoObject* const parent_;
  childs_t childrens_;
  const std::string delimiter_;
};

class FastoObjectCommand : public FastoObject {
 public:
  virtual ~FastoObjectCommand();
  virtual std::string ToString() const override;

  core::connectionTypes GetConnectionType() const;

  command_buffer_t GetInputCommand() const;
  CmdLoggingType GetCommandLoggingType() const;

 protected:
  FastoObjectCommand(FastoObject* parent,
                     common::StringValue* cmd,
                     CmdLoggingType ct,
                     const std::string& delimiter,
                     core::connectionTypes type);

 private:
  DISALLOW_COPY_AND_ASSIGN(FastoObjectCommand);

  const core::connectionTypes type_;
  const CmdLoggingType ct_;
};

}  // namespace core
}  // namespace fastonosql

namespace common {

std::string ConvertToString(fastonosql::core::FastoObject* obj);

}  // namespace common
