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

#include "common/value.h"
#include "common/convert2string.h"

namespace fastonosql {

class IFastoObjectObserver;
class FastoObject
  : public common::intrusive_ptr_base<FastoObject> {
 public:
  typedef std::vector<FastoObject*> child_container_type;

  FastoObject(FastoObject* parent, common::Value* val, const std::string& delemitr = std::string());
  virtual ~FastoObject();

  common::Value::Type type() const;
  virtual std::string toString() const;

  static FastoObject* createRoot(const std::string& text, IFastoObjectObserver* observer = nullptr);

  child_container_type childrens() const;
  void addChildren(FastoObject* child);
  FastoObject* parent() const;
  void clear();
  std::string delemitr() const;

  common::Value* value() const;
  void setValue(common::Value* val);

 protected:
  IFastoObjectObserver* observer_;
  common::scoped_ptr<common::Value> value_;

 private:
  DISALLOW_COPY_AND_ASSIGN(FastoObject);

  FastoObject* const parent_;
  child_container_type childrens_;
  const std::string delemitr_;
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

  std::string inputCommand() const;
  common::Value::CommandLoggingType commandLoggingType() const;

 protected:
  FastoObjectCommand(FastoObject* parent, common::CommandValue* cmd, const std::string &delemitr);
};

std::string stableCommand(std::string command);
std::pair<std::string, std::string> getKeyValueFromLine(const std::string& input);
std::string getFirstWordFromLine(const std::string& input);

class FastoObjectArray
  : public FastoObject {
 public:
  FastoObjectArray(FastoObject* parent, common::ArrayValue* ar, const std::string& delemitr);

  // Appends a Value to the end of the list.
  void append(common::Value* in_value);
  virtual std::string toString() const;

  common::ArrayValue* array() const;
};

class IFastoObjectObserver {
 public:
  virtual void addedChildren(FastoObject* child) = 0;
  virtual void updated(FastoObject* item, common::Value* val) = 0;
};

typedef common::intrusive_ptr<FastoObject> FastoObjectIPtr;
typedef common::intrusive_ptr<FastoObjectCommand> FastoObjectCommandIPtr;

}

namespace common {

std::string convertToString(fastonosql::FastoObject* obj);

std::string convertToString(common::Value* value, const std::string& delemitr);
std::string convertToString(common::ArrayValue* array, const std::string& delemitr);
std::string convertToString(common::SetValue* set, const std::string& delemitr);
std::string convertToString(common::ZSetValue* zset, const std::string& delemitr);
std::string convertToString(common::HashValue* hash, const std::string& delemitr);

}
