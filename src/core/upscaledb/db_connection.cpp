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

#include "core/upscaledb/db_connection.h"

#include <ups/upscaledb.hpp>
#include <stdlib.h>  // for NULL, free, calloc
#include <time.h>    // for time_t
#include <string>    // for string

#include <common/value.h>  // for StringValue (ptr only)
#include <common/utils.h>  // for c_strornull
#include <common/convert2string.h>

#include "core/upscaledb/config.h"               // for Config
#include "core/upscaledb/connection_settings.h"  // for ConnectionSettings
#include "core/upscaledb/command_translator.h"
#include "core/upscaledb/database.h"
#include "core/upscaledb/internal/commands_api.h"

#include "global/global.h"  // for FastoObject, etc

namespace fastonosql {
class FastoObjectArray;
}

namespace fastonosql {
namespace core {
namespace upscaledb {

struct upscaledb {
  ups_env_t* env;
  ups_db_t* db;
  uint16_t cur_db;
};

namespace {

ups_status_t upscaledb_open(upscaledb** context,
                            const char* dbpath,
                            uint16_t db,
                            bool create_if_missing) {

  upscaledb* lcontext = reinterpret_cast<upscaledb*>(calloc(1, sizeof(upscaledb)));
  ups_status_t st = create_if_missing ? ups_env_create(&lcontext->env, dbpath, 0, 0664, 0)
                                      : ups_env_open(&lcontext->env, dbpath, 0, 0);
  if (st != UPS_SUCCESS) {
    free(lcontext);
    return st;
  }

  st = ups_env_create_db(lcontext->env, &lcontext->db, db, 0, NULL);
  if (st != UPS_SUCCESS) {
    free(lcontext);
    return st;
  }

  lcontext->cur_db = db;
  *context = lcontext;
  return UPS_SUCCESS;
}

void upscaledb_close(upscaledb** context) {
  if (!context) {
    return;
  }

  upscaledb* lcontext = *context;
  if (!lcontext) {
    return;
  }

  ups_db_close(lcontext->db, 0);
  ups_env_close(lcontext->env, 0);
  free(lcontext);
  *context = NULL;
}

}  // namespace

common::Error CreateConnection(const Config& config, NativeConnection** context) {
  if (!context) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  DCHECK(*context == NULL);
  struct upscaledb* lcontext = NULL;
  std::string path = config.dbname;  // start point must be folder
  const char* dbname = common::utils::c_strornull(path);
  int st = upscaledb_open(&lcontext, dbname, 1, config.create_if_missing);
  if (st != UPS_SUCCESS) {
    std::string buff = common::MemSPrintf("Fail open database: %s", ups_strerror(st));
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

  struct upscaledb* scaledb = NULL;
  common::Error er = CreateConnection(settings, &scaledb);
  if (er && er->isError()) {
    return er;
  }

  upscaledb_close(&scaledb);
  return common::Error();
}

DBConnection::DBConnection(CDBConnectionClient* client)
    : base_class(client, new CommandTranslator) {}

uint16_t DBConnection::CurDb() const {
  if (connection_.handle_) {
    return connection_.handle_->cur_db;
  }

  return 0;
}

common::Error DBConnection::Info(const char* args, ServerInfo::Stats* statsout) {
  UNUSED(args);
  if (!statsout) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument for command: INFO",
                                    common::ErrorValue::E_ERROR);
  }

  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  ServerInfo::Stats linfo;
  Config conf = config();
  linfo.db_path = conf.dbname;

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

  size_t sz = 0;
  ups_status_t st = ups_db_count(connection_.handle_->db, NULL, 0, &sz);
  if (st != UPS_SUCCESS) {
    std::string buff = common::MemSPrintf("dbkcount function error: %s", ups_strerror(st));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  *size = sz;
  return common::Error();
}

common::Error DBConnection::SetInner(const std::string& key, const std::string& value) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  ups_key_t dkey;
  memset(&dkey, 0, sizeof(dkey));
  dkey.size = key.size();
  dkey.data = const_cast<char*>(key.c_str());

  ups_record_t rec;
  memset(&rec, 0, sizeof(rec));
  rec.data = const_cast<char*>(value.c_str());
  rec.size = value.size();

  ups_status_t st = ups_db_insert(connection_.handle_->db, 0, &dkey, &rec, UPS_OVERWRITE);
  if (st != UPS_SUCCESS) {
    std::string buff = common::MemSPrintf("set function error: %s", ups_strerror(st));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::GetInner(const std::string& key, std::string* ret_val) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  ups_key_t dkey;
  memset(&dkey, 0, sizeof(dkey));
  dkey.size = key.size();
  dkey.data = const_cast<char*>(key.c_str());

  ups_record_t rec;

  ups_status_t st = ups_db_find(connection_.handle_->db, NULL, &dkey, &rec, 0);
  if (st != UPS_SUCCESS) {
    std::string buff = common::MemSPrintf("get function error: %s", ups_strerror(st));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  ret_val->assign(reinterpret_cast<const char*>(rec.data), rec.size);
  return common::Error();
}

common::Error DBConnection::DelInner(const std::string& key) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  ups_key_t dkey;
  memset(&dkey, 0, sizeof(dkey));

  dkey.size = key.size();
  dkey.data = const_cast<char*>(key.c_str());

  ups_status_t st = ups_db_erase(connection_.handle_->db, 0, &dkey, 0);
  if (st != UPS_SUCCESS) {
    std::string buff = common::MemSPrintf("delete function error: %s", ups_strerror(st));
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

  ups_cursor_t* cursor; /* upscaledb cursor object */
  ups_key_t key;
  ups_record_t rec;

  memset(&key, 0, sizeof(key));
  memset(&rec, 0, sizeof(rec));

  /* create a new cursor */
  ups_status_t st = ups_cursor_create(&cursor, connection_.handle_->db, 0, 0);
  if (st) {
    std::string buff = common::MemSPrintf("Keys function error: %s", ups_strerror(st));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  /* get a cursor to the source database */
  st = ups_cursor_move(cursor, &key, &rec, UPS_CURSOR_FIRST);
  if (st == UPS_KEY_NOT_FOUND) {
    ups_cursor_close(cursor);
    return common::Error();
  } else if (st) {
    std::string buff = common::MemSPrintf("Keys function error: %s", ups_strerror(st));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  do {
    /* fetch the next item, and repeat till we've reached the end
     * of the database */
    st = ups_cursor_move(cursor, &key, &rec, UPS_CURSOR_NEXT);
    if (st && st != UPS_KEY_NOT_FOUND) {
      ups_cursor_close(cursor);
      std::string buff = common::MemSPrintf("Keys function error: %s", ups_strerror(st));
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }

    std::string skey(reinterpret_cast<const char*>(key.data), key.size);
    if (key_start < skey && key_end > skey) {
      ret->push_back(skey);
    }
  } while (st == UPS_SUCCESS && limit > ret->size());

  ups_cursor_close(cursor);
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

  ups_cursor_t* cursor; /* upscaledb cursor object */
  ups_key_t key;
  ups_record_t rec;

  memset(&key, 0, sizeof(key));
  memset(&rec, 0, sizeof(rec));

  /* create a new cursor */
  ups_status_t st = ups_cursor_create(&cursor, connection_.handle_->db, 0, 0);
  if (st) {
    std::string buff = common::MemSPrintf("Keys function error: %s", ups_strerror(st));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  /* get a cursor to the source database */
  st = ups_cursor_move(cursor, &key, &rec, UPS_CURSOR_FIRST);
  if (st == UPS_KEY_NOT_FOUND) {
    ups_cursor_close(cursor);
    return common::Error();
  } else if (st) {
    std::string buff = common::MemSPrintf("flushdb function error: %s", ups_strerror(st));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  do {
    /* fetch the next item, and repeat till we've reached the end
     * of the database */
    st = ups_cursor_move(cursor, &key, &rec, UPS_CURSOR_NEXT);
    if (st && st != UPS_KEY_NOT_FOUND) {
      ups_cursor_close(cursor);
      std::string buff = common::MemSPrintf("flushdb function error: %s", ups_strerror(st));
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }

    ups_db_erase(connection_.handle_->db, 0, &key, 0);
  } while (st == UPS_SUCCESS);

  ups_cursor_close(cursor);
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
                                  " for UPSCALEDB not supported TTL commands.",
                                  common::ErrorValue::E_ERROR);
}

common::Error DBConnection::QuitImpl() {
  common::Error err = Disconnect();
  if (err && err->isError()) {
    return err;
  }

  return common::Error();
}

}  // namespace upscaledb
namespace internal {
template <>
common::Error ConnectionAllocatorTraits<upscaledb::NativeConnection, upscaledb::Config>::Connect(
    const upscaledb::Config& config,
    upscaledb::NativeConnection** hout) {
  upscaledb::NativeConnection* context = nullptr;
  common::Error er = upscaledb::CreateConnection(config, &context);
  if (er && er->isError()) {
    return er;
  }

  *hout = context;
  return common::Error();
}

template <>
common::Error ConnectionAllocatorTraits<upscaledb::NativeConnection, upscaledb::Config>::Disconnect(
    upscaledb::NativeConnection** handle) {
  upscaledb::upscaledb_close(handle);
  *handle = nullptr;
  return common::Error();
}

template <>
bool ConnectionAllocatorTraits<upscaledb::NativeConnection, upscaledb::Config>::IsConnected(
    upscaledb::NativeConnection* handle) {
  if (!handle) {
    return false;
  }

  return true;
}

template <>
const char* CDBConnection<upscaledb::NativeConnection, upscaledb::Config, UPSCALEDB>::VersionApi() {
  return STRINGIZE(UPS_VERSION_MAJ) "." STRINGIZE(UPS_VERSION_MIN) "." STRINGIZE(UPS_VERSION_REV);
}

template <>
std::vector<CommandHolder>
CDBConnection<upscaledb::NativeConnection, upscaledb::Config, UPSCALEDB>::Commands() {
  return upscaledb::upscaledbCommands;
}
}
}  // namespace core
}  // namespace fastonosql
