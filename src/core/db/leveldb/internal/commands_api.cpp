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

#include "core/db/leveldb/internal/commands_api.h"

#include <time.h>

#include <common/convert2string.h>

#include "core/db_key.h"

#include "core/db/leveldb/db_connection.h"

#include "global/global.h"

namespace fastonosql {
namespace core {
namespace leveldb {

common::Error dbkcount(internal::CommandHandler* handler,
                       int argc,
                       const char** argv,
                       FastoObject* out) {
  UNUSED(argc);
  UNUSED(argv);

  DBConnection* level = static_cast<DBConnection*>(handler);

  size_t dbkcount = 0;
  common::Error er = level->DBkcount(&dbkcount);
  if (!er) {
    common::FundamentalValue* val = common::Value::createUIntegerValue(dbkcount);
    FastoObject* child = new FastoObject(out, val, level->Delimiter());
    out->AddChildren(child);
  }
  return er;
}

common::Error scan(internal::CommandHandler* handler,
                   int argc,
                   const char** argv,
                   FastoObject* out) {
  uint32_t cursor_in = common::ConvertFromString<uint32_t>(argv[0]);
  std::string pattern = argc >= 3 ? argv[2] : ALL_KEYS_PATTERNS;
  uint32_t count_keys = argc >= 5 ? common::ConvertFromString<uint32_t>(argv[4]) : NO_KEYS_LIMIT;
  uint64_t cursor_out = 0;
  std::vector<std::string> keys_out;
  DBConnection* level = static_cast<DBConnection*>(handler);

  common::Error err = level->Scan(cursor_in, pattern, count_keys, &keys_out, &cursor_out);
  if (err && err->isError()) {
    return err;
  }

  common::ArrayValue* ar = common::Value::createArrayValue();
  for (size_t i = 0; i < keys_out.size(); ++i) {
    common::StringValue* val = common::Value::createStringValue(keys_out[i]);
    ar->append(val);
  }

  common::ArrayValue* mar = common::Value::createArrayValue();
  std::string rep_out = common::ConvertToString(cursor_out);  // string representing
  common::StringValue* val = common::Value::createStringValue(rep_out);
  mar->append(val);
  FastoObjectArray* child = new FastoObjectArray(out, mar, level->Delimiter());
  FastoObjectArray* keys_arr = new FastoObjectArray(child, ar, level->Delimiter());
  child->AddChildren(keys_arr);
  out->AddChildren(child);
  return common::Error();
}

common::Error info(internal::CommandHandler* handler,
                   int argc,
                   const char** argv,
                   FastoObject* out) {
  DBConnection* level = static_cast<DBConnection*>(handler);

  ServerInfo::Stats statsout;
  common::Error er = level->Info(argc == 1 ? argv[0] : nullptr, &statsout);
  if (!er) {
    ServerInfo linf(statsout);
    common::StringValue* val = common::Value::createStringValue(linf.ToString());
    FastoObject* child = new FastoObject(out, val, level->Delimiter());
    out->AddChildren(child);
  }
  return er;
}

common::Error select(internal::CommandHandler* handler,
                     int argc,
                     const char** argv,
                     FastoObject* out) {
  UNUSED(argc);

  DBConnection* level = static_cast<DBConnection*>(handler);
  common::Error err = level->Select(argv[0], nullptr);
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, level->Delimiter());
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

  DBConnection* level = static_cast<DBConnection*>(handler);
  NDbKValue key_added;
  common::Error err = level->Set(kv, &key_added);
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, level->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error get(internal::CommandHandler* handler,
                  int argc,
                  const char** argv,
                  FastoObject* out) {
  UNUSED(argc);

  NKey key(argv[0]);
  DBConnection* level = static_cast<DBConnection*>(handler);
  NDbKValue key_loaded;
  common::Error err = level->Get(key, &key_loaded);
  if (err && err->isError()) {
    return err;
  }

  NValue val = key_loaded.Value();
  common::Value* copy = val->deepCopy();
  FastoObject* child = new FastoObject(out, copy, level->Delimiter());
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

  DBConnection* level = static_cast<DBConnection*>(handler);
  NKeys keys_deleted;
  common::Error err = level->Delete(keysdel, &keys_deleted);
  if (err && err->isError()) {
    return err;
  }

  common::FundamentalValue* val = common::Value::createUIntegerValue(keys_deleted.size());
  FastoObject* child = new FastoObject(out, val, level->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error rename(internal::CommandHandler* handler,
                     int argc,
                     const char** argv,
                     FastoObject* out) {
  UNUSED(argc);

  NKey key(argv[0]);
  DBConnection* level = static_cast<DBConnection*>(handler);
  common::Error err = level->Rename(key, argv[1]);
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, level->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error set_ttl(internal::CommandHandler* handler,
                      int argc,
                      const char** argv,
                      FastoObject* out) {
  UNUSED(out);
  UNUSED(argc);

  DBConnection* level = static_cast<DBConnection*>(handler);
  NKey key(argv[0]);
  time_t ttl = common::ConvertFromString<time_t>(argv[1]);
  common::Error err = level->SetTTL(key, ttl);
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, level->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error keys(internal::CommandHandler* handler,
                   int argc,
                   const char** argv,
                   FastoObject* out) {
  UNUSED(argc);

  DBConnection* level = static_cast<DBConnection*>(handler);

  std::vector<std::string> keysout;
  common::Error er =
      level->Keys(argv[0], argv[1], common::ConvertFromString<uint64_t>(argv[2]), &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, level->Delimiter());
    out->AddChildren(child);
  }
  return er;
}

common::Error help(internal::CommandHandler* handler,
                   int argc,
                   const char** argv,
                   FastoObject* out) {
  DBConnection* level = static_cast<DBConnection*>(handler);
  std::string answer;
  common::Error err = level->Help(argc, argv, &answer);
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue(answer);
  FastoObject* child = new FastoObject(out, val, level->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error flushdb(internal::CommandHandler* handler,
                      int argc,
                      const char** argv,
                      FastoObject* out) {
  UNUSED(argc);
  UNUSED(argv);

  DBConnection* level = static_cast<DBConnection*>(handler);
  common::Error err = level->FlushDB();
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, level->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error quit(internal::CommandHandler* handler,
                   int argc,
                   const char** argv,
                   FastoObject* out) {
  UNUSED(argc);
  UNUSED(argv);

  DBConnection* level = static_cast<DBConnection*>(handler);
  common::Error err = level->Quit();
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, level->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

}  // namespace leveldb
}  // namespace core
}  // namespace fastonosql
