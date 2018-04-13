/*  Copyright (C) 2014-2018 FastoGT. All right reserved.

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

#include <common/convert2string.h>
#include <common/file_system/path.h>

#include "core/global.h"

#include "core/internal/cdb_connection.h"

namespace fastonosql {
namespace core {
namespace internal {

// static functions, because linked to commands in static application time
template <class CDBConnection>
struct ApiTraits {
  typedef CDBConnection cdb_connection_t;
  static common::Error Help(CommandHandler* handler, commands_args_t argv, FastoObject* out);

  static common::Error Scan(CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Keys(CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error DBkcount(CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error CreateDatabase(CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error RemoveDatabase(CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error FlushDB(CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Select(CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Set(CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Get(CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Rename(CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Delete(CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error SetTTL(CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error GetTTL(CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ModuleLoad(CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ModuleUnLoad(CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error Quit(CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error ConfigGet(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out);
  static common::Error JsonDump(CommandHandler* handler, commands_args_t argv, FastoObject* out);
};

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::Help(internal::CommandHandler* handler,
                                             commands_args_t argv,
                                             FastoObject* out) {
  CDBConnection* cdb = static_cast<CDBConnection*>(handler);
  std::string answer;
  common::Error err = cdb->Help(argv, &answer);
  if (err) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue(answer);
  FastoObject* child = new FastoObject(out, val, cdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::Scan(internal::CommandHandler* handler,
                                             commands_args_t argv,
                                             FastoObject* out) {
  cursor_t cursor_in;
  if (!common::ConvertFromString(argv[0], &cursor_in)) {
    return common::make_error_inval();
  }

  const size_t argc = argv.size();
  std::string pattern = argc >= 3 ? argv[2] : ALL_KEYS_PATTERNS;
  cursor_t count_keys = NO_KEYS_LIMIT;
  if (argc >= 5 && !common::ConvertFromString(argv[4], &count_keys)) {
    return common::make_error_inval();
  }

  cursor_t cursor_out = 0;
  std::vector<std::string> keys_out;
  CDBConnection* cdb = static_cast<CDBConnection*>(handler);

  common::Error err = cdb->Scan(cursor_in, pattern, count_keys, &keys_out, &cursor_out);
  if (err) {
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
  mar->Append(ar);

  FastoObject* child = new FastoObject(out, mar, cdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::Keys(internal::CommandHandler* handler,
                                             commands_args_t argv,
                                             FastoObject* out) {
  CDBConnection* cdb = static_cast<CDBConnection*>(handler);

  cursor_t limit;
  if (!common::ConvertFromString(argv[2], &limit)) {
    return common::make_error_inval();
  }

  std::vector<std::string> keysout;
  common::Error err = cdb->Keys(argv[0], argv[1], limit, &keysout);
  if (err) {
    return err;
  }

  common::ArrayValue* ar = common::Value::CreateArrayValue();
  for (size_t i = 0; i < keysout.size(); ++i) {
    common::StringValue* val = common::Value::CreateStringValue(keysout[i]);
    ar->Append(val);
  }
  FastoObject* child = new FastoObject(out, ar, cdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::DBkcount(internal::CommandHandler* handler,
                                                 commands_args_t argv,
                                                 FastoObject* out) {
  UNUSED(argv);

  CDBConnection* cdb = static_cast<CDBConnection*>(handler);

  size_t dbkcount = 0;
  common::Error err = cdb->DBkcount(&dbkcount);
  if (err) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateUIntegerValue(dbkcount);
  FastoObject* child = new FastoObject(out, val, cdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::CreateDatabase(CommandHandler* handler,
                                                       commands_args_t argv,
                                                       FastoObject* out) {
  CDBConnection* cdb = static_cast<CDBConnection*>(handler);
  common::Error err = cdb->CreateDB(argv[0]);
  if (err) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, cdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::RemoveDatabase(CommandHandler* handler,
                                                       commands_args_t argv,
                                                       FastoObject* out) {
  CDBConnection* cdb = static_cast<CDBConnection*>(handler);
  common::Error err = cdb->RemoveDB(argv[0]);
  if (err) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, cdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::FlushDB(internal::CommandHandler* handler,
                                                commands_args_t argv,
                                                FastoObject* out) {
  UNUSED(argv);

  CDBConnection* cdb = static_cast<CDBConnection*>(handler);
  common::Error err = cdb->FlushDB();
  if (err) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, cdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::Select(CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  CDBConnection* cdb = static_cast<CDBConnection*>(handler);
  common::Error err = cdb->Select(argv[0], nullptr);
  if (err) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, cdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::Set(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  key_t raw_key(argv[0]);
  NKey key(raw_key);

  NValue string_val(common::Value::CreateStringValue(common::ConvertToString(argv[1])));
  NDbKValue kv(key, string_val);

  CDBConnection* cdb = static_cast<CDBConnection*>(handler);
  NDbKValue key_added;
  common::Error err = cdb->Set(kv, &key_added);
  if (err) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, cdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::Get(internal::CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  key_t raw_key(argv[0]);
  NKey key(raw_key);

  CDBConnection* cdb = static_cast<CDBConnection*>(handler);
  NDbKValue key_loaded;
  common::Error err = cdb->Get(key, &key_loaded);
  if (err) {
    return err;
  }

  NValue val = key_loaded.GetValue();
  common::Value* copy = val->DeepCopy();
  FastoObject* child = new FastoObject(out, copy, cdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::Delete(internal::CommandHandler* handler,
                                               commands_args_t argv,
                                               FastoObject* out) {
  NKeys keysdel;
  for (size_t i = 0; i < argv.size(); ++i) {
    key_t raw_key(argv[i]);
    NKey key(raw_key);
    keysdel.push_back(key);
  }

  CDBConnection* cdb = static_cast<CDBConnection*>(handler);
  NKeys keys_deleted;
  common::Error err = cdb->Delete(keysdel, &keys_deleted);
  if (err) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateUIntegerValue(keys_deleted.size());
  FastoObject* child = new FastoObject(out, val, cdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::Rename(internal::CommandHandler* handler,
                                               commands_args_t argv,
                                               FastoObject* out) {
  key_t raw_key(argv[0]);
  NKey key(raw_key);

  CDBConnection* cdb = static_cast<CDBConnection*>(handler);
  common::Error err = cdb->Rename(key, argv[1]);
  if (err) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, cdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::SetTTL(internal::CommandHandler* handler,
                                               commands_args_t argv,
                                               FastoObject* out) {
  CDBConnection* cdb = static_cast<CDBConnection*>(handler);
  key_t raw_key(argv[0]);
  NKey key(raw_key);

  ttl_t ttl;
  if (!common::ConvertFromString(argv[1], &ttl)) {
    return common::make_error_inval();
  }

  common::Error err = cdb->SetTTL(key, ttl);
  if (err) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, cdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::GetTTL(internal::CommandHandler* handler,
                                               commands_args_t argv,
                                               FastoObject* out) {
  CDBConnection* cdb = static_cast<CDBConnection*>(handler);
  key_t raw_key(argv[0]);
  NKey key(raw_key);

  ttl_t ttl;
  common::Error err = cdb->GetTTL(key, &ttl);
  if (err) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateLongLongIntegerValue(ttl);
  FastoObject* child = new FastoObject(out, val, cdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::ModuleLoad(CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  CDBConnection* cdb = static_cast<CDBConnection*>(handler);
  ModuleInfo mod;
  mod.name = argv[0];

  common::Error err = cdb->ModuleLoad(mod);
  if (err) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, cdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::ModuleUnLoad(CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  CDBConnection* cdb = static_cast<CDBConnection*>(handler);
  ModuleInfo mod;
  mod.name = argv[0];

  common::Error err = cdb->ModuleUnLoad(mod);
  if (err) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, cdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::Quit(internal::CommandHandler* handler,
                                             commands_args_t argv,
                                             FastoObject* out) {
  UNUSED(argv);

  CDBConnection* cdb = static_cast<CDBConnection*>(handler);
  common::Error err = cdb->Quit();
  if (err) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, cdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::ConfigGet(internal::CommandHandler* handler,
                                                  commands_args_t argv,
                                                  FastoObject* out) {
  if (!common::EqualsASCII(argv[0], "databases", false)) {
    return common::make_error_inval();
  }

  std::vector<std::string> dbs;
  CDBConnection* cdb = static_cast<CDBConnection*>(handler);
  common::Error err = cdb->ConfigGetDatabases(&dbs);
  if (err) {
    return err;
  }

  common::ArrayValue* arr = new common::ArrayValue;
  arr->AppendStrings(dbs);
  FastoObject* child = new FastoObject(out, arr, cdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

template <class CDBConnection>
common::Error ApiTraits<CDBConnection>::JsonDump(CommandHandler* handler, commands_args_t argv, FastoObject* out) {
  cursor_t cursor_in;
  const size_t argc = argv.size();
  if (argc < 3 || !common::ConvertFromString(argv[0], &cursor_in)) {
    return common::make_error_inval();
  }

  if (common::file_system::is_relative_path(argv[2])) {
    return common::make_error("Please use absolute path!");
  }

  common::file_system::ascii_file_string_path path(argv[2]);

  std::string pattern = argc >= 5 ? argv[4] : ALL_KEYS_PATTERNS;
  cursor_t count_keys = NO_KEYS_LIMIT;
  if (argc == 7 && !common::ConvertFromString(argv[6], &count_keys)) {
    return common::make_error_inval();
  }

  cursor_t cursor_out = 0;
  CDBConnection* cdb = static_cast<CDBConnection*>(handler);
  common::Error err = cdb->JsonDump(cursor_in, pattern, count_keys, path, &cursor_out);
  if (err) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateIntegerValue(cursor_out);
  FastoObject* child = new FastoObject(out, val, cdb->GetDelimiter());
  out->AddChildren(child);
  return common::Error();
}

}  // namespace internal
}  // namespace core
}  // namespace fastonosql
