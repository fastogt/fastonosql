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

#include "core/db/memcached/internal/commands_api.h"

#include <common/convert2string.h>

#include "core/db/memcached/db_connection.h"
#include "core/db_key.h"
#include "core/global.h"

namespace fastonosql {
namespace core {
namespace memcached {

common::Error CommandsApi::Version(internal::CommandHandler* handler,
                                   int argc,
                                   const char** argv,
                                   FastoObject* out) {
  UNUSED(argc);
  UNUSED(argv);
  UNUSED(out);

  DBConnection* mem = static_cast<DBConnection*>(handler);
  return mem->VersionServer();
}

common::Error CommandsApi::Info(internal::CommandHandler* handler,
                                int argc,
                                const char** argv,
                                FastoObject* out) {
  DBConnection* mem = static_cast<DBConnection*>(handler);
  const char* args = argc == 1 ? argv[0] : nullptr;
  if (args && strcasecmp(args, "items") == 0) {
    const char* largv[3] = {"a", "z", "100"};
    return Keys(handler, SIZEOFMASS(argv), largv, out);
  }

  ServerInfo::Stats statsout;
  common::Error err = mem->Info(args, &statsout);
  if (err && err->IsError()) {
    return err;
  }

  ServerInfo minf(statsout);
  common::StringValue* val = common::Value::CreateStringValue(minf.ToString());
  FastoObject* child = new FastoObject(out, val, mem->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Add(internal::CommandHandler* handler,
                               int argc,
                               const char** argv,
                               FastoObject* out) {
  UNUSED(argc);

  NKey key(argv[0]);
  DBConnection* mem = static_cast<DBConnection*>(handler);
  time_t expiration;
  if (!common::ConvertFromString(argv[2], &expiration)) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  uint32_t flags;
  if (!common::ConvertFromString(argv[1], &flags)) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  common::Error err = mem->AddIfNotExist(key, argv[3], expiration, flags);
  if (err && err->IsError()) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, mem->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Replace(internal::CommandHandler* handler,
                                   int argc,
                                   const char** argv,
                                   FastoObject* out) {
  UNUSED(argc);

  NKey key(argv[0]);
  DBConnection* mem = static_cast<DBConnection*>(handler);
  time_t expiration;
  if (!common::ConvertFromString(argv[2], &expiration)) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  uint32_t flags;
  if (!common::ConvertFromString(argv[1], &flags)) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  common::Error err = mem->Replace(key, argv[3], expiration, flags);
  if (err && err->IsError()) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, mem->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Append(internal::CommandHandler* handler,
                                  int argc,
                                  const char** argv,
                                  FastoObject* out) {
  UNUSED(argc);

  NKey key(argv[0]);
  DBConnection* mem = static_cast<DBConnection*>(handler);
  time_t expiration;
  if (!common::ConvertFromString(argv[2], &expiration)) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  uint32_t flags;
  if (!common::ConvertFromString(argv[1], &flags)) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }
  common::Error err = mem->Append(key, argv[3], expiration, flags);
  if (err && err->IsError()) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, mem->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Prepend(internal::CommandHandler* handler,
                                   int argc,
                                   const char** argv,
                                   FastoObject* out) {
  UNUSED(argc);

  NKey key(argv[0]);
  DBConnection* mem = static_cast<DBConnection*>(handler);
  time_t expiration;
  if (!common::ConvertFromString(argv[2], &expiration)) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  uint32_t flags;
  if (!common::ConvertFromString(argv[1], &flags)) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }
  common::Error err = mem->Prepend(key, argv[3], expiration, flags);
  if (err && err->IsError()) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, mem->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Incr(internal::CommandHandler* handler,
                                int argc,
                                const char** argv,
                                FastoObject* out) {
  UNUSED(argc);

  NKey key(argv[0]);
  DBConnection* mem = static_cast<DBConnection*>(handler);
  uint32_t value;
  if (!common::ConvertFromString(argv[1], &value)) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }
  uint64_t result = 0;
  common::Error err = mem->Incr(key, value, &result);
  if (err && err->IsError()) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateULongLongIntegerValue(result);
  FastoObject* child = new FastoObject(out, val, mem->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Decr(internal::CommandHandler* handler,
                                int argc,
                                const char** argv,
                                FastoObject* out) {
  UNUSED(argc);

  NKey key(argv[0]);
  DBConnection* mem = static_cast<DBConnection*>(handler);
  uint32_t value;
  if (!common::ConvertFromString(argv[1], &value)) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }
  uint64_t result = 0;
  common::Error err = mem->Decr(key, value, &result);
  if (err && err->IsError()) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateULongLongIntegerValue(result);
  FastoObject* child = new FastoObject(out, val, mem->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

}  // namespace memcached
}  // namespace core
}  // namespace fastonosql
