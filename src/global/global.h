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

#include <memory>                       // for shared_ptr
#include <string>                       // for string
#include <utility>                      // for pair
#include <vector>                       // for vector

#include "common/intrusive_ptr.h"       // for intrusive_ptr, etc
#include "common/macros.h"              // for DISALLOW_COPY_AND_ASSIGN
#include "common/value.h"               // for ArrayValue (ptr only), etc

#include "core/connection_types.h"

namespace fastonosql {

class FastoObject;
class FastoObjectCommand;

template<typename T, typename... Args>
inline common::intrusive_ptr<T> make_fasto_object(Args&&... args) {
  return common::intrusive_ptr<T>(new T(std::forward<Args>(args)...));
}

typedef common::intrusive_ptr<FastoObject> FastoObjectIPtr;
typedef common::intrusive_ptr<FastoObjectCommand> FastoObjectCommandIPtr;

class FastoObject
  : public common::intrusive_ptr_base<FastoObject> {
 public:
  typedef FastoObjectIPtr child_t;
  typedef std::vector<child_t> childs_t;
  typedef common::shared_ptr<common::Value> value_t;

  class IFastoObjectObserver {
   public:
    virtual void addedChildren(FastoObject* child) = 0;
    virtual void updated(FastoObject* item, value_t val) = 0;
  };

  FastoObject(FastoObject* parent, common::Value* val, const std::string& delimiter);  // val take ownerships
  virtual ~FastoObject();

  common::Value::Type type() const;
  virtual std::string toString() const;

  static FastoObject* createRoot(const std::string& text, IFastoObjectObserver* observer = nullptr);

  childs_t childrens() const;
  void addChildren(child_t child);
  FastoObject* parent() const;
  void clear();
  std::string delimiter() const;

  value_t value() const;
  void setValue(value_t val);

 protected:
  IFastoObjectObserver* observer_;
  value_t value_;

 private:
  DISALLOW_COPY_AND_ASSIGN(FastoObject);

  FastoObject* const parent_;
  childs_t childrens_;
  const std::string delimiter_;
};

class FastoObjectCommand
  : public FastoObject {
 public:
  virtual ~FastoObjectCommand();
  common::CommandValue* cmd() const;
  virtual std::string toString() const;

  virtual std::string inputCmd() const;
  virtual std::string inputArgs() const;

  virtual bool isReadOnly() const = 0;

  core::connectionTypes connectionType() const;

  std::string inputCommand() const;
  common::Value::CommandLoggingType commandLoggingType() const;

 protected:
  FastoObjectCommand(FastoObject* parent, common::CommandValue* cmd, const std::string& delimiter, core::connectionTypes type);

 private:
  const core::connectionTypes type_;
};

std::pair<std::string, std::string> GetKeyValueFromLine(const std::string& input);
std::string GetFirstWordFromLine(const std::string& input);

class FastoObjectArray
  : public FastoObject {
 public:
  FastoObjectArray(FastoObject* parent, common::ArrayValue* ar, const std::string& delimiter);

  // Appends a Value to the end of the list.
  void append(common::Value* in_value);
  virtual std::string toString() const;

  common::ArrayValue* array() const;
};

}  // namespace fastonosql

namespace common {

std::string ConvertToString(fastonosql::FastoObject* obj);

std::string ConvertToString(common::Value* value, const std::string& delimiter);
std::string ConvertToString(common::ArrayValue* array, const std::string& delimiter);
std::string ConvertToString(common::SetValue* set, const std::string& delimiter);
std::string ConvertToString(common::ZSetValue* zset, const std::string& delimiter);
std::string ConvertToString(common::HashValue* hash, const std::string& delimiter);

}  // namespace common
