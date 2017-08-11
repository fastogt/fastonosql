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

#include "core/db/rocksdb/internal/commands_api.h"

#include <stddef.h>  // for size_t
#include <memory>    // for __shared_ptr
#include <string>    // for string

#include <common/macros.h>  // for UNUSED
#include <common/value.h>   // for Value, ErrorValue, etc

#include "core/db/rocksdb/db_connection.h"
#include "core/db/rocksdb/server_info.h"  // for ServerInfo, etc

#include "core/global.h"  // for FastoObject, etc

namespace fastonosql {
namespace core {
namespace rocksdb {

common::Error CommandsApi::Info(internal::CommandHandler* handler,
                                commands_args_t argv,
                                FastoObject* out) {
  DBConnection* rocks = static_cast<DBConnection*>(handler);
  ServerInfo::Stats statsout;
  common::Error err = rocks->Info(argv.size() == 1 ? argv[0] : std::string(), &statsout);
  if (err && err->IsError()) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue(ServerInfo(statsout).ToString());
  FastoObject* child = new FastoObject(out, val, rocks->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Mget(internal::CommandHandler* handler,
                                commands_args_t argv,
                                FastoObject* out) {
  DBConnection* rocks = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysget;
  for (size_t i = 0; i < argv.size(); ++i) {
    keysget.push_back(argv[i]);
  }

  std::vector<std::string> keysout;
  common::Error err = rocks->Mget(keysget, &keysout);
  if (err && err->IsError()) {
    return err;
  }

  common::ArrayValue* ar = common::Value::CreateArrayValue();
  for (size_t i = 0; i < keysout.size(); ++i) {
    common::StringValue* val = common::Value::CreateStringValue(keysout[i]);
    ar->Append(val);
  }
  FastoObjectArray* child = new FastoObjectArray(out, ar, rocks->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Merge(internal::CommandHandler* handler,
                                 commands_args_t argv,
                                 FastoObject* out) {
  DBConnection* rocks = static_cast<DBConnection*>(handler);
  common::Error err = rocks->Merge(argv[0], argv[1]);
  if (err && err->IsError()) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, rocks->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

}  // namespace rocksdb
}  // namespace core
}  // namespace fastonosql
