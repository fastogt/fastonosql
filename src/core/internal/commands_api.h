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

#include <stddef.h>  // for size_t
#include <stdint.h>  // for uint32_t, uint64_t
#include <memory>    // for __shared_ptr
#include <string>    // for string
#include <vector>    // for vector

#include <common/convert2string.h>
#include <common/error.h>
#include <common/macros.h>  // for UNUSED
#include <common/value.h>   // for Value, ErrorValue, etc

#include "core/db_key.h"  // for NKey, NDbKValue, NKeys, etc
#include "core/global.h"
#include "core/internal/cdb_connection.h"

namespace fastonosql {
namespace core {
namespace internal {
class CommandHandler;
}
}  // namespace core
}  // namespace fastonosql

namespace fastonosql {
namespace core {
namespace internal {

template <class CDBConnection>
struct ApiTraits {
  typedef CDBConnection cdb_connection_t;
  static common::Error Help(CommandHandler* handler, std::vector<std::string> argv, FastoObject* out);

  static common::Error Scan(CommandHandler* handler, std::vector<std::string> argv, FastoObject* out);
  static common::Error Keys(CommandHandler* handler, std::vector<std::string> argv, FastoObject* out);
  static common::Error DBkcount(CommandHandler* handler, std::vector<std::string> argv, FastoObject* out);
  static common::Error FlushDB(CommandHandler* handler, std::vector<std::string> argv, FastoObject* out);
  static common::Error Select(CommandHandler* handler, std::vector<std::string> argv, FastoObject* out);
  static common::Error Set(CommandHandler* handler, std::vector<std::string> argv, FastoObject* out);
  static common::Error Get(CommandHandler* handler, std::vector<std::string> argv, FastoObject* out);
  static common::Error Rename(CommandHandler* handler, std::vector<std::string> argv, FastoObject* out);
  static common::Error Delete(CommandHandler* handler, std::vector<std::string> argv, FastoObject* out);
  static common::Error SetTTL(CommandHandler* handler, std::vector<std::string> argv, FastoObject* out);
  static common::Error GetTTL(CommandHandler* handler, std::vector<std::string> argv, FastoObject* out);
  static common::Error Quit(CommandHandler* handler, std::vector<std::string> argv, FastoObject* out);
};

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::Help(internal::CommandHandler* handler,
                                             std::vector<std::string> argv,
                                             FastoObject* out) {
  CDBConnection* cdb = static_cast<CDBConnection*>(handler);
  std::string answer;
  common::Error err = cdb->Help(argv, &answer);
  if (err && err->IsError()) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue(answer);
  FastoObject* child = new FastoObject(out, val, cdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::Scan(internal::CommandHandler* handler,
                                             std::vector<std::string> argv,
                                             FastoObject* out) {
  uint32_t cursor_in;
  if (!common::ConvertFromString(argv[0], &cursor_in)) {
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  const size_t argc = argv.size();
  std::string pattern = argc >= 3 ? argv[2] : ALL_KEYS_PATTERNS;
  uint64_t count_keys = NO_KEYS_LIMIT;
  if (argc >= 5 && !common::ConvertFromString(argv[4], &count_keys)) {
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  uint64_t cursor_out = 0;
  std::vector<std::string> keys_out;
  CDBConnection* cdb = static_cast<CDBConnection*>(handler);

  common::Error err = cdb->Scan(cursor_in, pattern, count_keys, &keys_out, &cursor_out);
  if (err && err->IsError()) {
    return err;
  }

  common::ArrayValue* ar = common::Value::CreateArrayValue();
  for (size_t i = 0; i < keys_out.size(); ++i) {
    common::StringValue* val = common::Value::CreateStringValue(keys_out[i]);
    ar->Append(val);
  }

  common::ArrayValue* mar = common::Value::CreateArrayValue();
  std::string rep_out = common::ConvertToString(cursor_out);  // string representing
  common::StringValue* val = common::Value::CreateStringValue(rep_out);
  mar->Append(val);
  FastoObjectArray* child = new FastoObjectArray(out, mar, cdb->Delimiter());
  out->AddChildren(child);
  FastoObjectArray* keys_arr = new FastoObjectArray(child, ar, cdb->Delimiter());
  child->AddChildren(keys_arr);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::Keys(internal::CommandHandler* handler,
                                             std::vector<std::string> argv,
                                             FastoObject* out) {
  CDBConnection* cdb = static_cast<CDBConnection*>(handler);

  uint64_t limit;
  if (!common::ConvertFromString(argv[2], &limit)) {
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  std::vector<std::string> keysout;
  common::Error err = cdb->Keys(argv[0], argv[1], limit, &keysout);
  if (err && err->IsError()) {
    return err;
  }

  common::ArrayValue* ar = common::Value::CreateArrayValue();
  for (size_t i = 0; i < keysout.size(); ++i) {
    common::StringValue* val = common::Value::CreateStringValue(keysout[i]);
    ar->Append(val);
  }
  FastoObjectArray* child = new FastoObjectArray(out, ar, cdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::DBkcount(internal::CommandHandler* handler,
                                                 std::vector<std::string> argv,
                                                 FastoObject* out) {
  UNUSED(argv);

  CDBConnection* cdb = static_cast<CDBConnection*>(handler);

  size_t dbkcount = 0;
  common::Error err = cdb->DBkcount(&dbkcount);
  if (err && err->IsError()) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateUIntegerValue(dbkcount);
  FastoObject* child = new FastoObject(out, val, cdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::FlushDB(internal::CommandHandler* handler,
                                                std::vector<std::string> argv,
                                                FastoObject* out) {
  UNUSED(argv);

  CDBConnection* cdb = static_cast<CDBConnection*>(handler);
  common::Error err = cdb->FlushDB();
  if (err && err->IsError()) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, cdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::Select(CommandHandler* handler,
                                               std::vector<std::string> argv,
                                               FastoObject* out) {
  CDBConnection* cdb = static_cast<CDBConnection*>(handler);
  common::Error err = cdb->Select(argv[0], nullptr);
  if (err && err->IsError()) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, cdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::Set(internal::CommandHandler* handler,
                                            std::vector<std::string> argv,
                                            FastoObject* out) {
  command_buffer_writer_t wr;
  wr << argv[0];
  key_t raw_key(wr.GetBuffer());
  NKey key(raw_key);

  NValue string_val(common::Value::CreateStringValue(common::ConvertToString(argv[1])));
  NDbKValue kv(key, string_val);

  CDBConnection* cdb = static_cast<CDBConnection*>(handler);
  NDbKValue key_added;
  common::Error err = cdb->Set(kv, &key_added);
  if (err && err->IsError()) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, cdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::Get(internal::CommandHandler* handler,
                                            std::vector<std::string> argv,
                                            FastoObject* out) {
  command_buffer_writer_t wr;
  wr << argv[0];
  key_t raw_key(wr.GetBuffer());
  NKey key(raw_key);

  CDBConnection* cdb = static_cast<CDBConnection*>(handler);
  NDbKValue key_loaded;
  common::Error err = cdb->Get(key, &key_loaded);
  if (err && err->IsError()) {
    return err;
  }

  NValue val = key_loaded.GetValue();
  common::Value* copy = val->DeepCopy();
  FastoObject* child = new FastoObject(out, copy, cdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::Delete(internal::CommandHandler* handler,
                                               std::vector<std::string> argv,
                                               FastoObject* out) {
  NKeys keysdel;
  for (size_t i = 0; i < argv.size(); ++i) {
    command_buffer_writer_t wr;
    wr << argv[i];
    key_t raw_key(wr.GetBuffer());
    NKey key(raw_key);

    keysdel.push_back(key);
  }

  CDBConnection* cdb = static_cast<CDBConnection*>(handler);
  NKeys keys_deleted;
  common::Error err = cdb->Delete(keysdel, &keys_deleted);
  if (err && err->IsError()) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateUIntegerValue(keys_deleted.size());
  FastoObject* child = new FastoObject(out, val, cdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::Rename(internal::CommandHandler* handler,
                                               std::vector<std::string> argv,
                                               FastoObject* out) {
  command_buffer_writer_t wr;
  wr << argv[0];
  key_t raw_key(wr.GetBuffer());
  NKey key(raw_key);

  CDBConnection* cdb = static_cast<CDBConnection*>(handler);
  wr.clear();
  wr << argv[1];
  common::Error err = cdb->Rename(key, wr.GetBuffer());
  if (err && err->IsError()) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, cdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::SetTTL(internal::CommandHandler* handler,
                                               std::vector<std::string> argv,
                                               FastoObject* out) {
  UNUSED(out);

  CDBConnection* cdb = static_cast<CDBConnection*>(handler);
  command_buffer_writer_t wr;
  wr << argv[0];
  key_t raw_key(wr.GetBuffer());
  NKey key(raw_key);

  ttl_t ttl;
  if (!common::ConvertFromString(argv[1], &ttl)) {
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  common::Error err = cdb->SetTTL(key, ttl);
  if (err && err->IsError()) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, cdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::GetTTL(internal::CommandHandler* handler,
                                               std::vector<std::string> argv,
                                               FastoObject* out) {
  UNUSED(out);

  CDBConnection* cdb = static_cast<CDBConnection*>(handler);
  command_buffer_writer_t wr;
  wr << argv[0];
  key_t raw_key(wr.GetBuffer());
  NKey key(raw_key);

  ttl_t ttl;
  common::Error err = cdb->GetTTL(key, &ttl);
  if (err && err->IsError()) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateIntegerValue(ttl);
  FastoObject* child = new FastoObject(out, val, cdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::Quit(internal::CommandHandler* handler,
                                             std::vector<std::string> argv,
                                             FastoObject* out) {
  UNUSED(argv);

  CDBConnection* cdb = static_cast<CDBConnection*>(handler);
  common::Error err = cdb->Quit();
  if (err && err->IsError()) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, cdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

}  // namespace internal
}  // namespace core
}  // namespace fastonosql
