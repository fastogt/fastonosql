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

#include <common/error.h>
#include <common/convert2string.h>

#include "core/internal/cdb_connection.h"

#include "global/global.h"

namespace fastonosql {
namespace core {
namespace internal {

template <class CDBConnection>
struct ApiTraits {
  typedef CDBConnection cdb_connection_t;
  static common::Error Help(CommandHandler* handler, int argc, const char** argv, FastoObject* out);

  static common::Error Scan(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error Keys(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error DBkcount(CommandHandler* handler,
                                int argc,
                                const char** argv,
                                FastoObject* out);
  static common::Error FlushDB(CommandHandler* handler,
                               int argc,
                               const char** argv,
                               FastoObject* out);
  static common::Error Select(CommandHandler* handler,
                              int argc,
                              const char** argv,
                              FastoObject* out);
  static common::Error Set(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error Get(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
  static common::Error Rename(CommandHandler* handler,
                              int argc,
                              const char** argv,
                              FastoObject* out);
  static common::Error Delete(CommandHandler* handler,
                              int argc,
                              const char** argv,
                              FastoObject* out);
  static common::Error SetTTL(CommandHandler* handler,
                              int argc,
                              const char** argv,
                              FastoObject* out);
  static common::Error Quit(CommandHandler* handler, int argc, const char** argv, FastoObject* out);
};

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::Help(internal::CommandHandler* handler,
                                             int argc,
                                             const char** argv,
                                             FastoObject* out) {
  CDBConnection* cdb = static_cast<CDBConnection*>(handler);
  std::string answer;
  common::Error err = cdb->Help(argc, argv, &answer);
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue(answer);
  FastoObject* child = new FastoObject(out, val, cdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::Scan(internal::CommandHandler* handler,
                                             int argc,
                                             const char** argv,
                                             FastoObject* out) {
  uint32_t cursor_in = common::ConvertFromString<uint32_t>(argv[0]);
  std::string pattern = argc >= 3 ? argv[2] : ALL_KEYS_PATTERNS;
  uint32_t count_keys = argc >= 5 ? common::ConvertFromString<uint32_t>(argv[4]) : NO_KEYS_LIMIT;
  uint64_t cursor_out = 0;
  std::vector<std::string> keys_out;
  CDBConnection* cdb = static_cast<CDBConnection*>(handler);

  common::Error err = cdb->Scan(cursor_in, pattern, count_keys, &keys_out, &cursor_out);
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
  FastoObjectArray* child = new FastoObjectArray(out, mar, cdb->Delimiter());
  FastoObjectArray* keys_arr = new FastoObjectArray(child, ar, cdb->Delimiter());
  child->AddChildren(keys_arr);
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::Keys(internal::CommandHandler* handler,
                                             int argc,
                                             const char** argv,
                                             FastoObject* out) {
  UNUSED(argc);

  CDBConnection* cdb = static_cast<CDBConnection*>(handler);

  std::vector<std::string> keysout;
  common::Error err =
      cdb->Keys(argv[0], argv[1], common::ConvertFromString<uint64_t>(argv[2]), &keysout);
  if (err && err->isError()) {
    return err;
  }

  common::ArrayValue* ar = common::Value::createArrayValue();
  for (size_t i = 0; i < keysout.size(); ++i) {
    common::StringValue* val = common::Value::createStringValue(keysout[i]);
    ar->append(val);
  }
  FastoObjectArray* child = new FastoObjectArray(out, ar, cdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::DBkcount(internal::CommandHandler* handler,
                                                 int argc,
                                                 const char** argv,
                                                 FastoObject* out) {
  UNUSED(argc);
  UNUSED(argv);

  CDBConnection* cdb = static_cast<CDBConnection*>(handler);

  size_t dbkcount = 0;
  common::Error err = cdb->DBkcount(&dbkcount);
  if (err && err->isError()) {
    return err;
  }

  common::FundamentalValue* val = common::Value::createUIntegerValue(dbkcount);
  FastoObject* child = new FastoObject(out, val, cdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::FlushDB(internal::CommandHandler* handler,
                                                int argc,
                                                const char** argv,
                                                FastoObject* out) {
  UNUSED(argc);
  UNUSED(argv);

  CDBConnection* cdb = static_cast<CDBConnection*>(handler);
  common::Error err = cdb->FlushDB();
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, cdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::Select(CommandHandler* handler,
                                               int argc,
                                               const char** argv,
                                               FastoObject* out) {
  UNUSED(argc);

  CDBConnection* cdb = static_cast<CDBConnection*>(handler);
  common::Error err = cdb->Select(argv[0], nullptr);
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, cdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::Set(internal::CommandHandler* handler,
                                            int argc,
                                            const char** argv,
                                            FastoObject* out) {
  UNUSED(argc);

  NKey key(argv[0]);
  NValue string_val(common::Value::createStringValue(argv[1]));
  NDbKValue kv(key, string_val);

  CDBConnection* cdb = static_cast<CDBConnection*>(handler);
  NDbKValue key_added;
  common::Error err = cdb->Set(kv, &key_added);
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, cdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::Get(internal::CommandHandler* handler,
                                            int argc,
                                            const char** argv,
                                            FastoObject* out) {
  UNUSED(argc);

  NKey key(argv[0]);
  CDBConnection* cdb = static_cast<CDBConnection*>(handler);
  NDbKValue key_loaded;
  common::Error err = cdb->Get(key, &key_loaded);
  if (err && err->isError()) {
    return err;
  }

  NValue val = key_loaded.Value();
  common::Value* copy = val->deepCopy();
  FastoObject* child = new FastoObject(out, copy, cdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::Delete(internal::CommandHandler* handler,
                                               int argc,
                                               const char** argv,
                                               FastoObject* out) {
  NKeys keysdel;
  for (int i = 0; i < argc; ++i) {
    keysdel.push_back(NKey(argv[i]));
  }

  CDBConnection* cdb = static_cast<CDBConnection*>(handler);
  NKeys keys_deleted;
  common::Error err = cdb->Delete(keysdel, &keys_deleted);
  if (err && err->isError()) {
    return err;
  }

  common::FundamentalValue* val = common::Value::createUIntegerValue(keys_deleted.size());
  FastoObject* child = new FastoObject(out, val, cdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::Rename(internal::CommandHandler* handler,
                                               int argc,
                                               const char** argv,
                                               FastoObject* out) {
  UNUSED(argc);

  NKey key(argv[0]);
  CDBConnection* cdb = static_cast<CDBConnection*>(handler);
  common::Error err = cdb->Rename(key, argv[1]);
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, cdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::SetTTL(internal::CommandHandler* handler,
                                               int argc,
                                               const char** argv,
                                               FastoObject* out) {
  UNUSED(out);
  UNUSED(argc);

  CDBConnection* level = static_cast<CDBConnection*>(handler);
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

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::Quit(internal::CommandHandler* handler,
                                             int argc,
                                             const char** argv,
                                             FastoObject* out) {
  UNUSED(argc);
  UNUSED(argv);

  CDBConnection* cdb = static_cast<CDBConnection*>(handler);
  common::Error err = cdb->Quit();
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, cdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

}  // namespace leveldb
}  // namespace core
}  // namespace fastonosql
