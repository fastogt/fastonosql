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

#include "core/db/rocksdb/internal/commands_api.h"

#include <common/convert2string.h>

#include "core/db_key.h"

#include "core/db/rocksdb/db_connection.h"

#include "global/global.h"

namespace fastonosql {
namespace core {
namespace rocksdb {

common::Error info(internal::CommandHandler* handler,
                   int argc,
                   const char** argv,
                   FastoObject* out) {
  DBConnection* rocks = static_cast<DBConnection*>(handler);
  ServerInfo::Stats statsout;
  common::Error er = rocks->Info(argc == 1 ? argv[0] : nullptr, &statsout);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue(ServerInfo(statsout).ToString());
    FastoObject* child = new FastoObject(out, val, rocks->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error select(internal::CommandHandler* handler,
                     int argc,
                     const char** argv,
                     FastoObject* out) {
  UNUSED(argc);

  DBConnection* rocks = static_cast<DBConnection*>(handler);
  common::Error err = rocks->Select(argv[0], nullptr);
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, rocks->Delimiter());
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

  DBConnection* rocks = static_cast<DBConnection*>(handler);
  NDbKValue key_added;
  common::Error err = rocks->Set(kv, &key_added);
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, rocks->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error get(internal::CommandHandler* handler,
                  int argc,
                  const char** argv,
                  FastoObject* out) {
  UNUSED(argc);

  NKey key(argv[0]);
  DBConnection* rocks = static_cast<DBConnection*>(handler);
  NDbKValue key_loaded;
  common::Error err = rocks->Get(key, &key_loaded);
  if (err && err->isError()) {
    return err;
  }

  NValue val = key_loaded.Value();
  common::Value* copy = val->deepCopy();
  FastoObject* child = new FastoObject(out, copy, rocks->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error mget(internal::CommandHandler* handler,
                   int argc,
                   const char** argv,
                   FastoObject* out) {
  DBConnection* rocks = static_cast<DBConnection*>(handler);
  std::vector< ::rocksdb::Slice> keysget;
  for (int i = 0; i < argc; ++i) {
    keysget.push_back(argv[i]);
  }

  std::vector<std::string> keysout;
  common::Error er = rocks->Mget(keysget, &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, rocks->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error merge(internal::CommandHandler* handler,
                    int argc,
                    const char** argv,
                    FastoObject* out) {
  UNUSED(argc);

  DBConnection* rocks = static_cast<DBConnection*>(handler);
  common::Error er = rocks->Merge(argv[0], argv[1]);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, rocks->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error del(internal::CommandHandler* handler,
                  int argc,
                  const char** argv,
                  FastoObject* out) {
  NKeys keysdel;
  for (int i = 0; i < argc; ++i) {
    keysdel.push_back(NKey(argv[i]));
  }

  DBConnection* rocks = static_cast<DBConnection*>(handler);
  NKeys keys_deleted;
  common::Error err = rocks->Delete(keysdel, &keys_deleted);
  if (err && err->isError()) {
    return err;
  }

  common::FundamentalValue* val = common::Value::createUIntegerValue(keys_deleted.size());
  FastoObject* child = new FastoObject(out, val, rocks->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error rename(internal::CommandHandler* handler,
                     int argc,
                     const char** argv,
                     FastoObject* out) {
  UNUSED(argc);

  NKey key(argv[0]);
  DBConnection* rocks = static_cast<DBConnection*>(handler);
  common::Error err = rocks->Rename(key, argv[1]);
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, rocks->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error set_ttl(internal::CommandHandler* handler,
                      int argc,
                      const char** argv,
                      FastoObject* out) {
  UNUSED(out);
  UNUSED(argc);

  DBConnection* rocks = static_cast<DBConnection*>(handler);
  NKey key(argv[0]);
  ttl_t ttl = common::ConvertFromString<ttl_t>(argv[1]);
  common::Error err = rocks->SetTTL(key, ttl);
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, rocks->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error keys(internal::CommandHandler* handler,
                   int argc,
                   const char** argv,
                   FastoObject* out) {
  UNUSED(argc);

  DBConnection* rocks = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysout;
  common::Error er =
      rocks->Keys(argv[0], argv[1], common::ConvertFromString<uint64_t>(argv[2]), &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, rocks->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error dbkcount(internal::CommandHandler* handler,
                       int argc,
                       const char** argv,
                       FastoObject* out) {
  UNUSED(argc);
  UNUSED(argv);

  DBConnection* rocks = static_cast<DBConnection*>(handler);
  size_t dbkcount = 0;
  common::Error er = rocks->DBkcount(&dbkcount);
  if (!er) {
    common::FundamentalValue* val = common::Value::createUIntegerValue(dbkcount);
    FastoObject* child = new FastoObject(out, val, rocks->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error help(internal::CommandHandler* handler,
                   int argc,
                   const char** argv,
                   FastoObject* out) {
  DBConnection* rocks = static_cast<DBConnection*>(handler);
  std::string answer;
  common::Error err = rocks->Help(argc, argv, &answer);
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue(answer);
  FastoObject* child = new FastoObject(out, val, rocks->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error flushdb(internal::CommandHandler* handler,
                      int argc,
                      const char** argv,
                      FastoObject* out) {
  UNUSED(argc);
  UNUSED(argv);

  DBConnection* rocks = static_cast<DBConnection*>(handler);
  common::Error err = rocks->FlushDB();
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, rocks->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error quit(internal::CommandHandler* handler,
                   int argc,
                   const char** argv,
                   FastoObject* out) {
  UNUSED(argc);
  UNUSED(argv);

  DBConnection* rocks = static_cast<DBConnection*>(handler);
  common::Error err = rocks->Quit();
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, rocks->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

}  // namespace rocksdb
}  // namespace core
}  // namespace fastonosql
