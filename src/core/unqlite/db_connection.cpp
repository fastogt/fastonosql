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

#include <stdlib.h>                     // for atoll

#include <memory>                       // for __shared_ptr
#include <string>                       // for string, operator<, etc
#include <vector>                       // for vector

extern "C" {
  #include <unqlite.h>
}

#include "common/sprintf.h"             // for MemSPrintf
#include "common/utils.h"               // for c_strornull
#include "common/value.h"               // for Value::ErrorsType::E_ERROR, etc
#include "core/unqlite/config.h"        // for Config
#include "core/unqlite/connection_settings.h"  // for ConnectionSettings

#include "global/global.h"              // for FastoObject, etc

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
    return "Method not implemented by the underlying Key/Value storage engine";
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
template<>
common::Error ConnectionAllocatorTraits<unqlite::NativeConnection, unqlite::Config>::connect(const unqlite::Config& config, unqlite::NativeConnection** hout) {
  unqlite::NativeConnection* context = NULL;
  common::Error er = unqlite::createConnection(config, &context);
  if (er && er->isError()) {
    return er;
  }

  *hout = context;
  return common::Error();
}
template<>
common::Error ConnectionAllocatorTraits<unqlite::NativeConnection, unqlite::Config>::disconnect(unqlite::NativeConnection** handle) {
  unqlite_close(*handle);
  *handle = NULL;
  return common::Error();
}
template<>
bool ConnectionAllocatorTraits<unqlite::NativeConnection, unqlite::Config>::isConnected(unqlite::NativeConnection* handle) {
  if (!handle) {
    return false;
  }

  return true;
}
namespace unqlite {

common::Error createConnection(const Config& config, NativeConnection** context) {
  if (!context) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  DCHECK(*context == NULL);
  struct unqlite* lcontext = NULL;
  const char* dbname = common::utils::c_strornull(config.dbname);
  int st = unqlite_open(&lcontext, dbname, config.create_if_missing ?
                          UNQLITE_OPEN_CREATE : UNQLITE_OPEN_READWRITE);
  if (st != UNQLITE_OK) {
    std::string buff = common::MemSPrintf("Fail open database: %s!", unqlite_strerror(st));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  *context = lcontext;
  return common::Error();
}

common::Error createConnection(ConnectionSettings* settings, NativeConnection **context) {
  if (!settings) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  Config config = settings->info();
  return createConnection(config, context);
}

common::Error testConnection(ConnectionSettings* settings) {
  if (!settings) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  struct unqlite* ldb = nullptr;
  common::Error er = createConnection(settings, &ldb);
  if (er && er->isError()) {
    return er;
  }

  unqlite_close(ldb);
  return common::Error();
}

DBConnection::DBConnection()
  : base_class(), CommandHandler(unqliteCommands) {
}

common::Error DBConnection::info(const char* args, ServerInfo::Stats* statsout) {
  UNUSED(args);

  if (!isConnected()) {
    DNOTREACHED();
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  if (!statsout) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  ServerInfo::Stats linfo;
  Config conf = config();
  linfo.file_name = conf.dbname;
  *statsout = linfo;
  return common::Error();
}

common::Error DBConnection::dbkcount(size_t* size) {
  if (!isConnected()) {
    DNOTREACHED();
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  if (!size) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
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

const char* DBConnection::versionApi() {
  return UNQLITE_VERSION;
}

common::Error DBConnection::set(const std::string& key, const std::string& value) {
  if (!isConnected()) {
    DNOTREACHED();
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  int rc = unqlite_kv_store(connection_.handle_, key.c_str(), key.size(), value.c_str(), value.length());
  if (rc != UNQLITE_OK) {
    std::string buff = common::MemSPrintf("set function error: %s", unqlite_strerror(rc));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error DBConnection::get(const std::string& key, std::string* ret_val) {
  if (!isConnected()) {
    DNOTREACHED();
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  int rc = unqlite_kv_fetch_callback(connection_.handle_, key.c_str(), key.size(), unqlite_data_callback, ret_val);
  if (rc != UNQLITE_OK) {
    std::string buff = common::MemSPrintf("get function error: %s", unqlite_strerror(rc));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error DBConnection::del(const std::string& key) {
  if (!isConnected()) {
    DNOTREACHED();
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  int rc = unqlite_kv_delete(connection_.handle_, key.c_str(), key.size());
  if (rc != UNQLITE_OK) {
    std::string buff = common::MemSPrintf("delete function error: %s", unqlite_strerror(rc));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error DBConnection::keys(const std::string& key_start, const std::string& key_end,
                   uint64_t limit, std::vector<std::string> *ret) {
  if (!isConnected()) {
    DNOTREACHED();
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

common::Error DBConnection::help(int argc, char** argv) {
  UNUSED(argc);
  UNUSED(argv);

  return notSupported("HELP");
}

common::Error DBConnection::flushdb() {
  if (!isConnected()) {
    DNOTREACHED();
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
    common::Error err = del(key);
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

common::Error set(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* unq = static_cast<DBConnection*>(handler);
  common::Error er = unq->set(argv[0], argv[1]);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, unq->delimiter());
    out->addChildren(child);
  }

  return er;
}

common::Error get(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* unq = static_cast<DBConnection*>(handler);
  std::string ret;
  common::Error er = unq->get(argv[0], &ret);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue(ret);
    FastoObject* child = new FastoObject(out, val, unq->delimiter());
    out->addChildren(child);
  }

  return er;
}

common::Error del(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* unq = static_cast<DBConnection*>(handler);
  common::Error er = unq->del(argv[0]);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, unq->delimiter());
    out->addChildren(child);
  }

  return er;
}

common::Error keys(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* unq = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysout;
  common::Error er = unq->keys(argv[0], argv[1], atoll(argv[2]), &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, unq->delimiter());
    out->addChildren(child);
  }

  return er;
}

common::Error info(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  DBConnection* unq = static_cast<DBConnection*>(handler);
  ServerInfo::Stats statsout;
  common::Error er = unq->info(argc == 1 ? argv[0] : nullptr, &statsout);
  if (!er) {
    ServerInfo uinf(statsout);
    common::StringValue* val = common::Value::createStringValue(uinf.toString());
    FastoObject* child = new FastoObject(out, val, unq->delimiter());
    out->addChildren(child);
  }

  return er;
}

common::Error dbkcount(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  UNUSED(argc);
  UNUSED(argv);

  DBConnection* unq = static_cast<DBConnection*>(handler);
  size_t dbkcount = 0;
  common::Error er = unq->dbkcount(&dbkcount);
  if (!er) {
    common::FundamentalValue* val = common::Value::createUIntegerValue(dbkcount);
    FastoObject* child = new FastoObject(out, val, unq->delimiter());
    out->addChildren(child);
  }

  return er;
}

common::Error help(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  UNUSED(out);

  DBConnection* unq = static_cast<DBConnection*>(handler);
  return unq->help(argc - 1, argv + 1);
}

common::Error flushdb(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  UNUSED(argc);
  UNUSED(argv);
  UNUSED(out);

  DBConnection* unq = static_cast<DBConnection*>(handler);
  return unq->flushdb();
}

}  // namespace unqlite
}  // namespace core
}  // namespace fastonosql
