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

common::Error keys(internal::CommandHandler* handler,
                   int argc,
                   const char** argv,
                   FastoObject* out) {
  UNUSED(argc);

  DBConnection* mem = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysout;
  common::Error er =
      mem->Keys(argv[0], argv[1], common::ConvertFromString<uint64_t>(argv[2]), &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, mem->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error stats(internal::CommandHandler* handler,
                    int argc,
                    const char** argv,
                    FastoObject* out) {
  DBConnection* mem = static_cast<DBConnection*>(handler);
  const char* args = argc == 1 ? argv[0] : nullptr;
  if (args && strcasecmp(args, "items") == 0) {
    const char* largv[3] = {"a", "z", "100"};
    return keys(handler, SIZEOFMASS(argv), largv, out);
  }

  ServerInfo::Stats statsout;
  common::Error er = mem->Info(args, &statsout);
  if (!er) {
    ServerInfo minf(statsout);
    common::StringValue* val = common::Value::createStringValue(minf.ToString());
    FastoObject* child = new FastoObject(out, val, mem->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error get(internal::CommandHandler* handler,
                  int argc,
                  const char** argv,
                  FastoObject* out) {
  UNUSED(argc);

  NKey key(argv[0]);
  DBConnection* unqlite = static_cast<DBConnection*>(handler);
  NDbKValue key_loaded;
  common::Error err = unqlite->Get(key, &key_loaded);
  if (err && err->isError()) {
    return err;
  }

  NValue val = key_loaded.Value();
  common::Value* copy = val->deepCopy();
  FastoObject* child = new FastoObject(out, copy, unqlite->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error add(internal::CommandHandler* handler,
                  int argc,
                  const char** argv,
                  FastoObject* out) {
  UNUSED(argc);

  DBConnection* mem = static_cast<DBConnection*>(handler);
  common::Error er =
      mem->AddIfNotExist(argv[0], argv[3], common::ConvertFromString<time_t>(argv[2]),
                         common::ConvertFromString<uint32_t>(argv[1]));
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, mem->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error replace(internal::CommandHandler* handler,
                      int argc,
                      const char** argv,
                      FastoObject* out) {
  UNUSED(argc);

  DBConnection* mem = static_cast<DBConnection*>(handler);
  common::Error er = mem->Replace(argv[0], argv[3], common::ConvertFromString<time_t>(argv[2]),
                                  common::ConvertFromString<uint32_t>(argv[1]));
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, mem->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error append(internal::CommandHandler* handler,
                     int argc,
                     const char** argv,
                     FastoObject* out) {
  UNUSED(argc);

  DBConnection* mem = static_cast<DBConnection*>(handler);
  common::Error er = mem->Append(argv[0], argv[3], common::ConvertFromString<time_t>(argv[2]),
                                 common::ConvertFromString<uint32_t>(argv[1]));
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, mem->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error prepend(internal::CommandHandler* handler,
                      int argc,
                      const char** argv,
                      FastoObject* out) {
  UNUSED(argc);

  DBConnection* mem = static_cast<DBConnection*>(handler);
  common::Error er = mem->Prepend(argv[0], argv[3], common::ConvertFromString<time_t>(argv[2]),
                                  common::ConvertFromString<uint32_t>(argv[1]));
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, mem->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error incr(internal::CommandHandler* handler,
                   int argc,
                   const char** argv,
                   FastoObject* out) {
  UNUSED(argc);

  DBConnection* mem = static_cast<DBConnection*>(handler);
  common::Error er = mem->Incr(argv[0], common::ConvertFromString<uint64_t>(argv[1]));
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, mem->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error decr(internal::CommandHandler* handler,
                   int argc,
                   const char** argv,
                   FastoObject* out) {
  UNUSED(argc);

  DBConnection* mem = static_cast<DBConnection*>(handler);
  common::Error er = mem->Decr(argv[0], common::ConvertFromString<uint64_t>(argv[1]));
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, mem->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error select(internal::CommandHandler* handler,
                     int argc,
                     const char** argv,
                     FastoObject* out) {
  UNUSED(argc);

  DBConnection* mem = static_cast<DBConnection*>(handler);
  common::Error err = mem->Select(argv[0], nullptr);
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, mem->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error set(internal::CommandHandler* handler,
                  int argc,
                  const char** argv,
                  FastoObject* out) {
  UNUSED(argc);

  NKey key(argv[0]);
  NValue string_val(common::Value::createStringValue(argv[1]));
  NDbKValue kv(key, string_val);

  DBConnection* red = static_cast<DBConnection*>(handler);
  NDbKValue key_added;
  common::Error err = red->Set(kv, &key_added);
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, red->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error rename(internal::CommandHandler* handler,
                     int argc,
                     const char** argv,
                     FastoObject* out) {
  UNUSED(argc);

  NKey key(argv[0]);
  DBConnection* red = static_cast<DBConnection*>(handler);
  common::Error err = red->Rename(key, argv[1]);
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, red->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error set_ttl(internal::CommandHandler* handler,
                      int argc,
                      const char** argv,
                      FastoObject* out) {
  UNUSED(out);
  UNUSED(argc);

  DBConnection* mem = static_cast<DBConnection*>(handler);
  NKey key(argv[0]);
  time_t ttl = common::ConvertFromString<time_t>(argv[1]);
  common::Error err = mem->SetTTL(key, ttl);
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, mem->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error del(internal::CommandHandler* handler,
                  int argc,
                  const char** argv,
                  FastoObject* out) {
  NKeys keysdel;
  for (int i = 0; i < argc; ++i) {
    keysdel.push_back(NKey(argv[i]));
  }

  DBConnection* mem = static_cast<DBConnection*>(handler);
  NKeys keys_deleted;
  common::Error err = mem->Delete(keysdel, &keys_deleted);
  if (err && err->isError()) {
    return err;
  }

  common::FundamentalValue* val = common::Value::createUIntegerValue(keys_deleted.size());
  FastoObject* child = new FastoObject(out, val, mem->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error flushdb(internal::CommandHandler* handler,
                      int argc,
                      const char** argv,
                      FastoObject* out) {
  DBConnection* mem = static_cast<DBConnection*>(handler);
  common::Error er = mem->Flushdb(argc == 1 ? common::ConvertFromString<time_t>(argv[0]) : 0);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, mem->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error version_server(internal::CommandHandler* handler,
                             int argc,
                             const char** argv,
                             FastoObject* out) {
  UNUSED(argc);
  UNUSED(argv);
  UNUSED(out);

  DBConnection* mem = static_cast<DBConnection*>(handler);
  return mem->VersionServer();
}

common::Error dbkcount(internal::CommandHandler* handler,
                       int argc,
                       const char** argv,
                       FastoObject* out) {
  UNUSED(argc);
  UNUSED(argv);

  DBConnection* mem = static_cast<DBConnection*>(handler);
  size_t dbkcount = 0;
  common::Error er = mem->DBkcount(&dbkcount);
  if (!er) {
    common::FundamentalValue* val = common::Value::createUIntegerValue(dbkcount);
    FastoObject* child = new FastoObject(out, val, mem->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error help(internal::CommandHandler* handler,
                   int argc,
                   const char** argv,
                   FastoObject* out) {
  UNUSED(out);

  DBConnection* mem = static_cast<DBConnection*>(handler);
  return mem->Help(argc - 1, argv + 1);
}

common::Error expire(internal::CommandHandler* handler,
                     int argc,
                     const char** argv,
                     FastoObject* out) {
  return set_ttl(handler, argc, argv, out);
}

common::Error quit(internal::CommandHandler* handler,
                   int argc,
                   const char** argv,
                   FastoObject* out) {
  UNUSED(argc);
  UNUSED(argv);

  DBConnection* mem = static_cast<DBConnection*>(handler);
  common::Error err = mem->Quit();
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
