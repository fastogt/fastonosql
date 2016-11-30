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

#include "core/db_key.h"

#include "core/db/memcached/db_connection.h"

#include "global/global.h"

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
  if (err && err->isError()) {
    return err;
  }

  ServerInfo minf(statsout);
  common::StringValue* val = common::Value::createStringValue(minf.ToString());
  FastoObject* child = new FastoObject(out, val, mem->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Add(internal::CommandHandler* handler,
                               int argc,
                               const char** argv,
                               FastoObject* out) {
  UNUSED(argc);

  DBConnection* mem = static_cast<DBConnection*>(handler);
  common::Error err =
      mem->AddIfNotExist(argv[0], argv[3], common::ConvertFromString<time_t>(argv[2]),
                         common::ConvertFromString<uint32_t>(argv[1]));
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, mem->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Replace(internal::CommandHandler* handler,
                                   int argc,
                                   const char** argv,
                                   FastoObject* out) {
  UNUSED(argc);

  DBConnection* mem = static_cast<DBConnection*>(handler);
  common::Error err = mem->Replace(argv[0], argv[3], common::ConvertFromString<time_t>(argv[2]),
                                   common::ConvertFromString<uint32_t>(argv[1]));
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, mem->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Append(internal::CommandHandler* handler,
                                  int argc,
                                  const char** argv,
                                  FastoObject* out) {
  UNUSED(argc);

  DBConnection* mem = static_cast<DBConnection*>(handler);
  common::Error err = mem->Append(argv[0], argv[3], common::ConvertFromString<time_t>(argv[2]),
                                  common::ConvertFromString<uint32_t>(argv[1]));
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, mem->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Prepend(internal::CommandHandler* handler,
                                   int argc,
                                   const char** argv,
                                   FastoObject* out) {
  UNUSED(argc);

  DBConnection* mem = static_cast<DBConnection*>(handler);
  common::Error err = mem->Prepend(argv[0], argv[3], common::ConvertFromString<time_t>(argv[2]),
                                   common::ConvertFromString<uint32_t>(argv[1]));
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, mem->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Incr(internal::CommandHandler* handler,
                                int argc,
                                const char** argv,
                                FastoObject* out) {
  UNUSED(argc);

  DBConnection* mem = static_cast<DBConnection*>(handler);
  common::Error err = mem->Incr(argv[0], common::ConvertFromString<uint64_t>(argv[1]));
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, mem->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Decr(internal::CommandHandler* handler,
                                int argc,
                                const char** argv,
                                FastoObject* out) {
  UNUSED(argc);

  DBConnection* mem = static_cast<DBConnection*>(handler);
  common::Error err = mem->Decr(argv[0], common::ConvertFromString<uint64_t>(argv[1]));
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, mem->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

}  // namespace memcached
}  // namespace core
}  // namespace fastonosql
