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

#include "core/db/upscaledb/db_connection.h"

#include <stdlib.h>  // for NULL, free, calloc
#include <string.h>  // for memset
#include <string>    // for string
#include <memory>    // for __shared_ptr

#include <ups/upscaledb.h>

#include <common/value.h>  // for StringValue (ptr only)
#include <common/utils.h>  // for c_strornull
#include <common/convert2string.h>
#include <common/file_system.h>  // for get_dir_path, is_directory, etc
#include <common/sprintf.h>      // for MemSPrintf
#include <common/string_util.h>  // for MatchPattern
#include <common/types.h>        // for tribool, tribool::SUCCESS

#include "core/command_holder.h"       // for CommandHolder
#include "core/internal/connection.h"  // for Connection<>::handle_t, etc
#include "core/db/upscaledb/config.h"  // for Config
#include "core/db/upscaledb/command_translator.h"
#include "core/db/upscaledb/database_info.h"
#include "core/db/upscaledb/internal/commands_api.h"

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
  bool need_to_create = false;
  if (create_if_missing) {
    bool exist = common::file_system::is_file_exist(std::string(dbpath));
    if (!exist) {
      need_to_create = true;
    }
  }

  ups_status_t st = need_to_create ? ups_env_create(&lcontext->env, dbpath, 0, 0664, 0)
                                   : ups_env_open(&lcontext->env, dbpath, 0, 0);
  if (st != UPS_SUCCESS) {
    free(lcontext);
    return st;
  }

  st = need_to_create ? ups_env_create_db(lcontext->env, &lcontext->db, db, 0, NULL)
                      : ups_env_open_db(lcontext->env, &lcontext->db, db, 0, NULL);
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

  ups_status_t st = ups_db_close(lcontext->db, 0);
  DCHECK(st == UPS_SUCCESS);
  st = ups_env_close(lcontext->env, 0);
  DCHECK(st == UPS_SUCCESS);
  free(lcontext);
  *context = NULL;
}
}
}
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
const char* CDBConnection<upscaledb::NativeConnection, upscaledb::Config, UPSCALEDB>::BasedOn() {
  return "libupscaledb";
}

template <>
const char* CDBConnection<upscaledb::NativeConnection, upscaledb::Config, UPSCALEDB>::VersionApi() {
  return STRINGIZE(UPS_VERSION_MAJ) "." STRINGIZE(UPS_VERSION_MIN) "." STRINGIZE(UPS_VERSION_REV);
}

template <>
std::vector<CommandHolder>
CDBConnection<upscaledb::NativeConnection, upscaledb::Config, UPSCALEDB>::Commands() {
  return upscaledb::g_commands;
}
}  // namespace internal
namespace upscaledb {
common::Error CreateConnection(const Config& config, NativeConnection** context) {
  if (!context) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  DCHECK(*context == NULL);
  struct upscaledb* lcontext = NULL;
  std::string db_path = config.dbname;  // start point must be folder
  std::string folder = common::file_system::get_dir_path(db_path);
  common::tribool is_dir = common::file_system::is_directory(folder);
  if (is_dir != common::SUCCESS) {
    return common::make_error_value(common::MemSPrintf("Invalid input path(%s)", db_path),
                                    common::ErrorValue::E_ERROR);
  }

  const char* dbname = common::utils::c_strornull(db_path);
  int st = upscaledb_open(&lcontext, dbname, config.dbnum, config.create_if_missing);
  if (st != UPS_SUCCESS) {
    std::string buff = common::MemSPrintf("Fail open database: %s", ups_strerror(st));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  *context = lcontext;
  return common::Error();
}

common::Error TestConnection(const Config& config) {
  struct upscaledb* scaledb = NULL;
  common::Error er = CreateConnection(config, &scaledb);
  if (er && er->isError()) {
    return er;
  }

  upscaledb_close(&scaledb);
  return common::Error();
}

DBConnection::DBConnection(CDBConnectionClient* client)
    : base_class(client, new CommandTranslator(base_class::Commands())) {}

std::string DBConnection::CurrentDBName() const {
  if (connection_.handle_) {
    return common::ConvertToString(connection_.handle_->cur_db);
  }

  DNOTREACHED();
  return base_class::CurrentDBName();
}

common::Error DBConnection::Connect(const config_t& config) {
  common::Error err = base_class::Connect(config);
  if (err && err->isError()) {
    return err;
  }

  err = Select(common::ConvertToString(connection_.config_.dbnum), NULL);
  if (err && err->isError()) {
    return err;
  }

  return common::Error();
}

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
  linfo.db_path = conf.dbname;

  *statsout = linfo;
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
    std::string buff = common::MemSPrintf("SET function error: %s", ups_strerror(st));
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
  memset(&rec, 0, sizeof(rec));

  ups_status_t st = ups_db_find(connection_.handle_->db, NULL, &dkey, &rec, 0);
  if (st != UPS_SUCCESS) {
    std::string buff = common::MemSPrintf("GET function error: %s", ups_strerror(st));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  *ret_val = std::string(reinterpret_cast<const char*>(rec.data), rec.size);
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
    std::string buff = common::MemSPrintf("DEL function error: %s", ups_strerror(st));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error DBConnection::ScanImpl(uint64_t cursor_in,
                                     std::string pattern,
                                     uint64_t count_keys,
                                     std::vector<std::string>* keys_out,
                                     uint64_t* cursor_out) {
  ups_cursor_t* cursor; /* upscaledb cursor object */
  ups_key_t key;
  ups_record_t rec;

  memset(&key, 0, sizeof(key));
  memset(&rec, 0, sizeof(rec));

  /* create a new cursor */
  ups_status_t st = ups_cursor_create(&cursor, connection_.handle_->db, 0, 0);
  if (st != UPS_SUCCESS) {
    std::string buff = common::MemSPrintf("KEYS function error: %s", ups_strerror(st));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  uint64_t offset_pos = cursor_in;
  uint64_t lcursor_out = 0;
  std::vector<std::string> lkeys_out;
  while (st == UPS_SUCCESS) {
    if (lkeys_out.size() < count_keys) {
      /* fetch the next item, and repeat till we've reached the end
       * of the database */
      st = ups_cursor_move(cursor, &key, &rec, UPS_CURSOR_NEXT | UPS_SKIP_DUPLICATES);
      if (st == UPS_SUCCESS) {
        std::string skey(reinterpret_cast<const char*>(key.data), key.size);
        if (common::MatchPattern(skey, pattern)) {
          if (offset_pos == 0) {
            lkeys_out.push_back(skey);
          } else {
            offset_pos--;
          }
        }
      } else if (st && st != UPS_KEY_NOT_FOUND) {
        ups_cursor_close(cursor);
        std::string buff = common::MemSPrintf("SCAN function error: %s", ups_strerror(st));
        return common::make_error_value(buff, common::ErrorValue::E_ERROR);
      }
    } else {
      lcursor_out = cursor_in + count_keys;
      break;
    }
  }

  ups_cursor_close(cursor);
  *keys_out = lkeys_out;
  *cursor_out = lcursor_out;
  return common::Error();
}

common::Error DBConnection::KeysImpl(const std::string& key_start,
                                     const std::string& key_end,
                                     uint64_t limit,
                                     std::vector<std::string>* ret) {
  ups_cursor_t* cursor; /* upscaledb cursor object */
  ups_key_t key;
  ups_record_t rec;

  memset(&key, 0, sizeof(key));
  memset(&rec, 0, sizeof(rec));

  /* create a new cursor */
  ups_status_t st = ups_cursor_create(&cursor, connection_.handle_->db, 0, 0);
  if (st != UPS_SUCCESS) {
    std::string buff = common::MemSPrintf("KEYS function error: %s", ups_strerror(st));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  do {
    st = ups_cursor_move(cursor, &key, &rec, UPS_CURSOR_NEXT | UPS_SKIP_DUPLICATES);
    if (st == UPS_SUCCESS) {
      std::string skey(reinterpret_cast<const char*>(key.data), key.size);
      if (key_start < skey && key_end > skey) {
        ret->push_back(skey);
      }
    } else if (st && st != UPS_KEY_NOT_FOUND) {
      ups_cursor_close(cursor);
      std::string buff = common::MemSPrintf("SCAN function error: %s", ups_strerror(st));
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }
  } while (st == UPS_SUCCESS && limit > ret->size());

  ups_cursor_close(cursor);
  return common::Error();
}

common::Error DBConnection::DBkcountImpl(size_t* size) {
  uint64_t sz = 0;
  ups_status_t st = ups_db_count(connection_.handle_->db, NULL, UPS_SKIP_DUPLICATES, &sz);
  if (st != UPS_SUCCESS) {
    std::string buff = common::MemSPrintf("DBKCOUNT function error: %s", ups_strerror(st));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  *size = sz;
  return common::Error();
}

common::Error DBConnection::FlushDBImpl() {
  ups_cursor_t* cursor; /* upscaledb cursor object */
  ups_key_t key;
  ups_record_t rec;

  memset(&key, 0, sizeof(key));
  memset(&rec, 0, sizeof(rec));

  /* create a new cursor */
  ups_status_t st = ups_cursor_create(&cursor, connection_.handle_->db, 0, 0);
  if (st != UPS_SUCCESS) {
    std::string buff = common::MemSPrintf("FLUSHDB function error: %s", ups_strerror(st));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  do {
    /* fetch the next item, and repeat till we've reached the end
     * of the database */
    st = ups_cursor_move(cursor, &key, &rec, UPS_CURSOR_NEXT);
    if (st == UPS_SUCCESS) {
      ups_db_erase(connection_.handle_->db, 0, &key, 0);
    } else if (st && st != UPS_KEY_NOT_FOUND) {
      ups_cursor_close(cursor);
      std::string buff = common::MemSPrintf("FLUSHDB function error: %s", ups_strerror(st));
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }
  } while (st == UPS_SUCCESS);

  ups_cursor_close(cursor);
  return common::Error();
}

common::Error DBConnection::SelectImpl(const std::string& name, IDataBaseInfo** info) {
  uint16_t num = common::ConvertFromString<uint16_t>(name);
  ups_db_t* db = NULL;
  ups_status_t st = ups_env_open_db(connection_.handle_->env, &db, num, 0, NULL);
  if (st != UPS_SUCCESS && st != UPS_DATABASE_ALREADY_OPEN) {
    std::string buff = common::MemSPrintf("SELECT function error: %s", ups_strerror(st));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  if (st == UPS_SUCCESS) {  // if ready to change
    st = ups_db_close(connection_.handle_->db, 0);
    DCHECK(st == UPS_SUCCESS);
    connection_.handle_->db = db;
    connection_.config_.dbnum = num;
    connection_.handle_->cur_db = num;
  }

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

common::Error DBConnection::GetTTLImpl(const NKey& key, ttl_t* ttl) {
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
}  // namespace core
}  // namespace fastonosql
