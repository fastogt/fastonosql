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

#include "core/unqlite/db_connection.h"

#include <memory>  // for __shared_ptr
#include <string>  // for string, operator<, etc
#include <vector>  // for vector

extern "C" {
#include <unqlite.h>
}

#include <common/sprintf.h>  // for MemSPrintf
#include <common/utils.h>    // for c_strornull
#include <common/value.h>    // for Value::ErrorsType::E_ERROR, etc
#include <common/convert2string.h>

#include "core/unqlite/config.h"               // for Config
#include "core/unqlite/connection_settings.h"  // for ConnectionSettings
#include "core/unqlite/database.h"
#include "core/unqlite/command_translator.h"

#include "global/global.h"  // for FastoObject, etc

namespace {

std::string unqlite_constext_strerror(unqlite* context) {
  const char* zErr = NULL;
  int iLen = 0;
  unqlite_config(context, UNQLITE_CONFIG_ERR_LOG, &zErr, &iLen);
  return std::string(zErr, iLen);
}

std::string unqlite_strerror(int unqlite_error) {
  if (unqlite_error == UNQLITE_OK) {
    return std::string();
  } else if (unqlite_error == UNQLITE_NOMEM) {
    return "Out of memory";
  } else if (unqlite_error == UNQLITE_ABORT) {
    return "Another thread have released this instance";
  } else if (unqlite_error == UNQLITE_IOERR) {
    return "IO error";
  } else if (unqlite_error == UNQLITE_CORRUPT) {
    return "Corrupt pointer";
  } else if (unqlite_error == UNQLITE_LOCKED) {
    return "Forbidden Operation";
  } else if (unqlite_error == UNQLITE_BUSY) {
    return "The database file is locked";
  } else if (unqlite_error == UNQLITE_DONE) {
    return "Operation done";
  } else if (unqlite_error == UNQLITE_PERM) {
    return "Permission error";
  } else if (unqlite_error == UNQLITE_NOTIMPLEMENTED) {
    return "Method not implemented by the underlying "
           "Key/Value storage engine";
  } else if (unqlite_error == UNQLITE_NOTFOUND) {
    return "No such record";
  } else if (unqlite_error == UNQLITE_NOOP) {
    return "No such method";
  } else if (unqlite_error == UNQLITE_INVALID) {
    return "Invalid parameter";
  } else if (unqlite_error == UNQLITE_EOF) {
    return "End Of Input";
  } else if (unqlite_error == UNQLITE_UNKNOWN) {
    return "Unknown configuration option";
  } else if (unqlite_error == UNQLITE_LIMIT) {
    return "Database limit reached";
  } else if (unqlite_error == UNQLITE_EXISTS) {
    return "Record exists";
  } else if (unqlite_error == UNQLITE_EMPTY) {
    return "Empty record";
  } else if (unqlite_error == UNQLITE_COMPILE_ERR) {
    return "Compilation error";
  } else if (unqlite_error == UNQLITE_VM_ERR) {
    return "Virtual machine error";
  } else if (unqlite_error == UNQLITE_FULL) {
    return "Full database (unlikely)";
  } else if (unqlite_error == UNQLITE_CANTOPEN) {
    return "Unable to open the database file";
  } else if (unqlite_error == UNQLITE_READ_ONLY) {
    return "Read only Key/Value storage engine";
  } else if (unqlite_error == UNQLITE_LOCKERR) {
    return "Locking protocol error";
  } else {
    return common::MemSPrintf("Unknown error %d", unqlite_error);
  }
}

int unqlite_data_callback(const void* pData, unsigned int nDatalen, void* str) {
  std::string* out = static_cast<std::string*>(str);
  out->assign(reinterpret_cast<const char*>(pData), nDatalen);
  return UNQLITE_OK;
}

}  // namespace

namespace fastonosql {
namespace core {
namespace internal {
template <>
common::Error ConnectionAllocatorTraits<unqlite::NativeConnection, unqlite::Config>::Connect(
    const unqlite::Config& config,
    unqlite::NativeConnection** hout) {
  unqlite::NativeConnection* context = NULL;
  common::Error er = unqlite::CreateConnection(config, &context);
  if (er && er->isError()) {
    return er;
  }

  *hout = context;
  return common::Error();
}
template <>
common::Error ConnectionAllocatorTraits<unqlite::NativeConnection, unqlite::Config>::Disconnect(
    unqlite::NativeConnection** handle) {
  unqlite_close(*handle);
  *handle = NULL;
  return common::Error();
}
template <>
bool ConnectionAllocatorTraits<unqlite::NativeConnection, unqlite::Config>::IsConnected(
    unqlite::NativeConnection* handle) {
  if (!handle) {
    return false;
  }

  return true;
}
}
namespace unqlite {

common::Error CreateConnection(const Config& config, NativeConnection** context) {
  if (!context) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  DCHECK(*context == NULL);
  struct unqlite* lcontext = NULL;
  const char* dbname = common::utils::c_strornull(config.dbname);
  int st = unqlite_open(&lcontext, dbname,
                        config.create_if_missing ? UNQLITE_OPEN_CREATE : UNQLITE_OPEN_READWRITE);
  if (st != UNQLITE_OK) {
    std::string buff = common::MemSPrintf("Fail open database: %s!", unqlite_strerror(st));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  *context = lcontext;
  return common::Error();
}

common::Error CreateConnection(ConnectionSettings* settings, NativeConnection** context) {
  if (!settings) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  Config config = settings->Info();
  return CreateConnection(config, context);
}

common::Error TestConnection(ConnectionSettings* settings) {
  if (!settings) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  struct unqlite* ldb = nullptr;
  common::Error er = CreateConnection(settings, &ldb);
  if (er && er->isError()) {
    return er;
  }

  unqlite_close(ldb);
  return common::Error();
}

DBConnection::DBConnection(CDBConnectionClient* client)
    : base_class(unqliteCommands, client, new CommandTranslator) {}

common::Error DBConnection::Info(const char* args, ServerInfo::Stats* statsout) {
  UNUSED(args);
  if (!statsout) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  ServerInfo::Stats linfo;
  Config conf = config();
  linfo.file_name = conf.dbname;
  *statsout = linfo;
  return common::Error();
}

common::Error DBConnection::DBkcount(size_t* size) {
  if (!size) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  /* Allocate a new cursor instance */
  unqlite_kv_cursor* pCur; /* Cursor handle */
  int rc = unqlite_kv_cursor_init(connection_.handle_, &pCur);
  if (rc != UNQLITE_OK) {
    std::string buff = common::MemSPrintf("DBKCOUNT function error: %s", unqlite_strerror(rc));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  /* Point to the first record */
  unqlite_kv_cursor_first_entry(pCur);

  size_t sz = 0;
  /* Iterate over the entries */
  while (unqlite_kv_cursor_valid_entry(pCur)) {
    sz++;
    /* Point to the next entry */
    unqlite_kv_cursor_next_entry(pCur);
  }
  /* Finally, Release our cursor */
  unqlite_kv_cursor_release(connection_.handle_, pCur);

  *size = sz;
  return common::Error();
}

const char* DBConnection::VersionApi() {
  return UNQLITE_VERSION;
}

common::Error DBConnection::SetInner(const std::string& key, const std::string& value) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  int rc =
      unqlite_kv_store(connection_.handle_, key.c_str(), key.size(), value.c_str(), value.length());
  if (rc != UNQLITE_OK) {
    std::string buff = common::MemSPrintf("set function error: %s", unqlite_strerror(rc));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error DBConnection::DelInner(const std::string& key) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  int rc = unqlite_kv_delete(connection_.handle_, key.c_str(), key.size());
  if (rc != UNQLITE_OK) {
    std::string buff = common::MemSPrintf("delete function error: %s", unqlite_strerror(rc));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error DBConnection::GetInner(const std::string& key, std::string* ret_val) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  int rc = unqlite_kv_fetch_callback(connection_.handle_, key.c_str(), key.size(),
                                     unqlite_data_callback, ret_val);
  if (rc != UNQLITE_OK) {
    std::string buff = common::MemSPrintf("get function error: %s", unqlite_strerror(rc));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error DBConnection::Keys(const std::string& key_start,
                                 const std::string& key_end,
                                 uint64_t limit,
                                 std::vector<std::string>* ret) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  /* Allocate a new cursor instance */
  unqlite_kv_cursor* pCur; /* Cursor handle */
  int rc = unqlite_kv_cursor_init(connection_.handle_, &pCur);
  if (rc != UNQLITE_OK) {
    std::string buff = common::MemSPrintf("Keys function error: %s", unqlite_strerror(rc));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  /* Point to the first record */
  unqlite_kv_cursor_first_entry(pCur);

  /* Iterate over the entries */
  while (unqlite_kv_cursor_valid_entry(pCur) && limit > ret->size()) {
    std::string key;
    unqlite_kv_cursor_key_callback(pCur, unqlite_data_callback, &key);
    if (key_start < key && key_end > key) {
      ret->push_back(key);
    }

    /* Point to the next entry */
    unqlite_kv_cursor_next_entry(pCur);
  }
  /* Finally, Release our cursor */
  unqlite_kv_cursor_release(connection_.handle_, pCur);

  return common::Error();
}

common::Error DBConnection::Help(int argc, const char** argv) {
  UNUSED(argc);
  UNUSED(argv);

  return NotSupported("HELP");
}

common::Error DBConnection::Flushdb() {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  unqlite_kv_cursor* pCur; /* Cursor handle */
  int rc = unqlite_kv_cursor_init(connection_.handle_, &pCur);
  if (rc != UNQLITE_OK) {
    std::string buff = common::MemSPrintf("FlushDB function error: %s", unqlite_strerror(rc));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  /* Point to the first record */
  unqlite_kv_cursor_first_entry(pCur);

  /* Iterate over the entries */
  while (unqlite_kv_cursor_valid_entry(pCur)) {
    std::string key;
    unqlite_kv_cursor_key_callback(pCur, unqlite_data_callback, &key);
    common::Error err = DelInner(key);
    if (err && err->isError()) {
      return err;
    }
    /* Point to the next entry */
    unqlite_kv_cursor_next_entry(pCur);
  }

  /* Finally, Release our cursor */
  unqlite_kv_cursor_release(connection_.handle_, pCur);
  return common::Error();
}

common::Error DBConnection::SelectImpl(const std::string& name, IDataBaseInfo** info) {
  size_t kcount = 0;
  common::Error err = DBkcount(&kcount);
  DCHECK(!err);
  *info = new DataBaseInfo(name, true, kcount);
  return common::Error();
}

common::Error DBConnection::SetImpl(const NDbKValue& key, NDbKValue* added_key) {
  std::string key_str = key.KeyString();
  std::string value_str = key.ValueString();
  common::Error err = SetInner(key_str, value_str);
  if (err && err->isError()) {
    return err;
  }

  *added_key = key;
  return common::Error();
}

common::Error DBConnection::GetImpl(const NKey& key, NDbKValue* loaded_key) {
  std::string key_str = key.Key();
  std::string value_str;
  common::Error err = GetInner(key_str, &value_str);
  if (err && err->isError()) {
    return err;
  }

  NValue val(common::Value::createStringValue(value_str));
  *loaded_key = NDbKValue(key, val);
  return common::Error();
}

common::Error DBConnection::RenameImpl(const NKey& key, const std::string& new_key) {
  std::string key_str = key.Key();
  std::string value_str;
  common::Error err = GetInner(key_str, &value_str);
  if (err && err->isError()) {
    return err;
  }

  err = DelInner(key_str);
  if (err && err->isError()) {
    return err;
  }

  err = SetInner(new_key, value_str);
  if (err && err->isError()) {
    return err;
  }

  return common::Error();
}

common::Error DBConnection::SetTTLImpl(const NKey& key, ttl_t ttl) {
  UNUSED(key);
  UNUSED(ttl);
  return common::make_error_value("Sorry, but now " PROJECT_NAME_TITLE
                                  " for UnqLite not supported TTL commands.",
                                  common::ErrorValue::E_ERROR);
}

common::Error DBConnection::DeleteImpl(const NKeys& keys, NKeys* deleted_keys) {
  for (size_t i = 0; i < keys.size(); ++i) {
    NKey key = keys[i];
    std::string key_str = key.Key();
    common::Error err = DelInner(key_str);
    if (err && err->isError()) {
      continue;
    }

    deleted_keys->push_back(key);
  }

  return common::Error();
}

common::Error select(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* unqlite = static_cast<DBConnection*>(handler);
  common::Error err = unqlite->Select(argv[0], nullptr);
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, unqlite->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error set(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  NKey key(argv[0]);
  NValue string_val(common::Value::createStringValue(argv[1]));
  NDbKValue kv(key, string_val);

  DBConnection* unqlite = static_cast<DBConnection*>(handler);
  NDbKValue key_added;
  common::Error err = unqlite->Set(kv, &key_added);
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, unqlite->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error get(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
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

common::Error del(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  NKeys keysdel;
  for (int i = 0; i < argc; ++i) {
    keysdel.push_back(NKey(argv[i]));
  }

  DBConnection* unq = static_cast<DBConnection*>(handler);
  NKeys keys_deleted;
  common::Error err = unq->Delete(keysdel, &keys_deleted);
  if (err && err->isError()) {
    return err;
  }

  common::FundamentalValue* val = common::Value::createUIntegerValue(keys_deleted.size());
  FastoObject* child = new FastoObject(out, val, unq->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error rename(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
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

common::Error set_ttl(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(out);
  UNUSED(argc);

  DBConnection* unq = static_cast<DBConnection*>(handler);
  NKey key(argv[0]);
  ttl_t ttl = common::ConvertFromString<ttl_t>(argv[1]);
  common::Error er = unq->SetTTL(key, ttl);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, unq->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error keys(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* unq = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysout;
  common::Error er =
      unq->Keys(argv[0], argv[1], common::ConvertFromString<uint64_t>(argv[2]), &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, unq->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error info(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  DBConnection* unq = static_cast<DBConnection*>(handler);
  ServerInfo::Stats statsout;
  common::Error er = unq->Info(argc == 1 ? argv[0] : nullptr, &statsout);
  if (!er) {
    ServerInfo uinf(statsout);
    common::StringValue* val = common::Value::createStringValue(uinf.ToString());
    FastoObject* child = new FastoObject(out, val, unq->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error dbkcount(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);
  UNUSED(argv);

  DBConnection* unq = static_cast<DBConnection*>(handler);
  size_t dbkcount = 0;
  common::Error er = unq->DBkcount(&dbkcount);
  if (!er) {
    common::FundamentalValue* val = common::Value::createUIntegerValue(dbkcount);
    FastoObject* child = new FastoObject(out, val, unq->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error help(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(out);

  DBConnection* unq = static_cast<DBConnection*>(handler);
  return unq->Help(argc - 1, argv + 1);
}

common::Error flushdb(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);
  UNUSED(argv);
  UNUSED(out);

  DBConnection* unq = static_cast<DBConnection*>(handler);
  return unq->Flushdb();
}

}  // namespace unqlite
}  // namespace core
}  // namespace fastonosql
