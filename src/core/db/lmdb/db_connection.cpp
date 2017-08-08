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

#include "core/db/lmdb/db_connection.h"

#include <errno.h>   // for EACCES
#include <lmdb.h>    // for mdb_txn_abort, MDB_val
#include <stdlib.h>  // for NULL, free, calloc
#include <time.h>    // for time_t
#include <string>    // for string

#include <common/convert2string.h>
#include <common/file_system.h>
#include <common/utils.h>  // for c_strornull
#include <common/value.h>  // for StringValue (ptr only)

#include "core/db/lmdb/command_translator.h"
#include "core/db/lmdb/config.h"  // for Config
#include "core/db/lmdb/database_info.h"
#include "core/db/lmdb/internal/commands_api.h"

#include "core/global.h"  // for FastoObject, etc

#define LMDB_OK 0

namespace fastonosql {
namespace core {
namespace {
MDB_val ConvertToLMDBSlice(const string_key_t& key) {
  MDB_val mkey;
  mkey.mv_size = key.size();
  mkey.mv_data = const_cast<char*>(key.data());
  return mkey;
}
}
namespace lmdb {
struct lmdb {
  MDB_env* env;
  MDB_dbi dbir;
};

namespace {

unsigned int lmdb_db_flag_from_env_flags(int env_flags) {
  return (env_flags & MDB_RDONLY) ? MDB_RDONLY : 0;
}

int lmdb_open(lmdb** context, const char* db_path, const char* db_name, int env_flags, unsigned int db_env_flags) {
  lmdb* lcontext = reinterpret_cast<lmdb*>(calloc(1, sizeof(lmdb)));
  int rc = mdb_env_create(&lcontext->env);
  if (rc != LMDB_OK) {
    free(lcontext);
    return rc;
  }

  rc = mdb_env_open(lcontext->env, db_path, env_flags, 0664);
  if (rc != LMDB_OK) {
    free(lcontext);
    return rc;
  }

  MDB_txn* txn = NULL;
  rc = mdb_txn_begin(lcontext->env, NULL, db_env_flags, &txn);
  if (rc != LMDB_OK) {
    free(lcontext);
    return rc;
  }

  rc = mdb_dbi_open(txn, db_name, 0, &lcontext->dbir);
  mdb_txn_abort(txn);
  if (rc != LMDB_OK) {
    free(lcontext);
    return rc;
  }

  *context = lcontext;
  return rc;
}

void lmdb_close(lmdb** context) {
  if (!context) {
    return;
  }

  lmdb* lcontext = *context;
  if (!lcontext) {
    return;
  }

  mdb_dbi_close(lcontext->env, lcontext->dbir);
  mdb_env_close(lcontext->env);
  free(lcontext);
  *context = NULL;
}

}  // namespace
}  // namespace lmdb
namespace internal {
template <>
common::Error ConnectionAllocatorTraits<lmdb::NativeConnection, lmdb::Config>::Connect(const lmdb::Config& config,
                                                                                       lmdb::NativeConnection** hout) {
  lmdb::NativeConnection* context = nullptr;
  common::Error er = lmdb::CreateConnection(config, &context);
  if (er && er->IsError()) {
    return er;
  }

  *hout = context;
  return common::Error();
}

template <>
common::Error ConnectionAllocatorTraits<lmdb::NativeConnection, lmdb::Config>::Disconnect(
    lmdb::NativeConnection** handle) {
  lmdb::lmdb_close(handle);
  *handle = nullptr;
  return common::Error();
}

template <>
bool ConnectionAllocatorTraits<lmdb::NativeConnection, lmdb::Config>::IsConnected(lmdb::NativeConnection* handle) {
  if (!handle) {
    return false;
  }

  return true;
}

template <>
const char* CDBConnection<lmdb::NativeConnection, lmdb::Config, LMDB>::BasedOn() {
  return "liblmdb";
}

template <>
const char* CDBConnection<lmdb::NativeConnection, lmdb::Config, LMDB>::VersionApi() {
  return STRINGIZE(MDB_VERSION_MAJOR) "." STRINGIZE(MDB_VERSION_MINOR) "." STRINGIZE(MDB_VERSION_PATCH);
}

template <>
ConstantCommandsArray CDBConnection<lmdb::NativeConnection, lmdb::Config, LMDB>::Commands() {
  return lmdb::g_commands;
}

}  // namespace internal

namespace lmdb {

common::Error CreateConnection(const Config& config, NativeConnection** context) {
  if (!context) {
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  DCHECK(*context == NULL);
  struct lmdb* lcontext = NULL;
  std::string folder = config.db_path;  // start point must be folder
  common::tribool is_dir = common::file_system::is_directory(folder);
  if (is_dir != common::SUCCESS) {
    return common::make_error_value(common::MemSPrintf("Invalid input path(%s)", folder), common::ErrorValue::E_ERROR);
  }

  const char* db_path = common::utils::c_strornull(folder);
  int env_flags = config.env_flags;
  int st = lmdb_open(&lcontext, db_path, NULL, env_flags, lmdb_db_flag_from_env_flags(env_flags));
  if (st != LMDB_OK) {
    std::string buff = common::MemSPrintf("Fail open database: %s", mdb_strerror(st));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  *context = lcontext;
  return common::Error();
}

common::Error TestConnection(const Config& config) {
  NativeConnection* ldb = nullptr;
  common::Error er = CreateConnection(config, &ldb);
  if (er && er->IsError()) {
    return er;
  }

  lmdb_close(&ldb);
  return common::Error();
}

DBConnection::DBConnection(CDBConnectionClient* client)
    : base_class(client, new CommandTranslator(base_class::Commands())) {}

std::string DBConnection::CurrentDBName() const {
  if (connection_.handle_) {
    return common::ConvertToString(connection_.handle_->dbir);
  }

  DNOTREACHED();
  return base_class::CurrentDBName();
}

common::Error DBConnection::Info(const char* args, ServerInfo::Stats* statsout) {
  UNUSED(args);
  if (!statsout) {
    DNOTREACHED();
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  ServerInfo::Stats linfo;
  Config conf = config();
  linfo.db_path = conf.db_path;

  *statsout = linfo;
  return common::Error();
}

common::Error DBConnection::SetInner(string_key_t key, const std::string& value) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  MDB_val key_slice = ConvertToLMDBSlice(key);
  MDB_val mval;
  mval.mv_size = value.size();
  mval.mv_data = const_cast<char*>(value.c_str());

  MDB_txn* txn = NULL;
  int env_flags = connection_.config_.env_flags;
  int rc = mdb_txn_begin(connection_.handle_->env, NULL, lmdb_db_flag_from_env_flags(env_flags), &txn);
  if (rc == LMDB_OK) {
    rc = mdb_put(txn, connection_.handle_->dbir, &key_slice, &mval, 0);
    if (rc == LMDB_OK) {
      rc = mdb_txn_commit(txn);
    } else {
      mdb_txn_abort(txn);
    }
  }

  if (rc != LMDB_OK) {
    std::string buff = common::MemSPrintf("set function error: %s", mdb_strerror(rc));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error DBConnection::GetInner(string_key_t key, std::string* ret_val) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  MDB_val key_slice = ConvertToLMDBSlice(key);
  MDB_val mval;

  MDB_txn* txn = NULL;
  int rc = mdb_txn_begin(connection_.handle_->env, NULL, MDB_RDONLY, &txn);
  if (rc == LMDB_OK) {
    rc = mdb_get(txn, connection_.handle_->dbir, &key_slice, &mval);
  }
  mdb_txn_abort(txn);

  if (rc != LMDB_OK) {
    std::string buff = common::MemSPrintf("get function error: %s", mdb_strerror(rc));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  ret_val->assign(reinterpret_cast<const char*>(mval.mv_data), mval.mv_size);
  return common::Error();
}

common::Error DBConnection::DelInner(string_key_t key) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  MDB_val key_slice = ConvertToLMDBSlice(key);

  MDB_txn* txn = NULL;
  int env_flags = connection_.config_.env_flags;
  int rc = mdb_txn_begin(connection_.handle_->env, NULL, lmdb_db_flag_from_env_flags(env_flags), &txn);
  if (rc == LMDB_OK) {
    rc = mdb_del(txn, connection_.handle_->dbir, &key_slice, NULL);
    if (rc == LMDB_OK) {
      rc = mdb_txn_commit(txn);
    } else {
      mdb_txn_abort(txn);
    }
  }

  if (rc != LMDB_OK) {
    std::string buff = common::MemSPrintf("delete function error: %s", mdb_strerror(rc));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error DBConnection::ScanImpl(uint64_t cursor_in,
                                     const std::string& pattern,
                                     uint64_t count_keys,
                                     std::vector<std::string>* keys_out,
                                     uint64_t* cursor_out) {
  MDB_cursor* cursor = NULL;
  MDB_txn* txn = NULL;
  int rc = mdb_txn_begin(connection_.handle_->env, NULL, MDB_RDONLY, &txn);
  if (rc == LMDB_OK) {
    rc = mdb_cursor_open(txn, connection_.handle_->dbir, &cursor);
  }

  if (rc != LMDB_OK) {
    mdb_txn_abort(txn);
    std::string buff = common::MemSPrintf("Keys function error: %s", mdb_strerror(rc));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  MDB_val key;
  MDB_val data;
  uint64_t offset_pos = cursor_in;
  uint64_t lcursor_out = 0;
  std::vector<std::string> lkeys_out;
  while ((mdb_cursor_get(cursor, &key, &data, MDB_NEXT) == LMDB_OK)) {
    if (lkeys_out.size() < count_keys) {
      std::string skey(reinterpret_cast<const char*>(key.mv_data), key.mv_size);
      if (common::MatchPattern(skey, pattern)) {
        if (offset_pos == 0) {
          lkeys_out.push_back(skey);
        } else {
          offset_pos--;
        }
      }
    } else {
      lcursor_out = cursor_in + count_keys;
      break;
    }
  }

  *keys_out = lkeys_out;
  *cursor_out = lcursor_out;
  mdb_cursor_close(cursor);
  mdb_txn_abort(txn);
  return common::Error();
}

common::Error DBConnection::KeysImpl(const std::string& key_start,
                                     const std::string& key_end,
                                     uint64_t limit,
                                     std::vector<std::string>* ret) {
  MDB_cursor* cursor = NULL;
  MDB_txn* txn = NULL;
  int rc = mdb_txn_begin(connection_.handle_->env, NULL, MDB_RDONLY, &txn);
  if (rc == LMDB_OK) {
    rc = mdb_cursor_open(txn, connection_.handle_->dbir, &cursor);
  }

  if (rc != LMDB_OK) {
    mdb_txn_abort(txn);
    std::string buff = common::MemSPrintf("Keys function error: %s", mdb_strerror(rc));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  MDB_val key;
  MDB_val data;
  while ((mdb_cursor_get(cursor, &key, &data, MDB_NEXT) == LMDB_OK) && limit > ret->size()) {
    std::string skey(reinterpret_cast<const char*>(key.mv_data), key.mv_size);
    if (key_start < skey && key_end > skey) {
      ret->push_back(skey);
    }
  }

  mdb_cursor_close(cursor);
  mdb_txn_abort(txn);
  return common::Error();
}

common::Error DBConnection::DBkcountImpl(size_t* size) {
  MDB_cursor* cursor = NULL;
  MDB_txn* txn = NULL;
  int rc = mdb_txn_begin(connection_.handle_->env, NULL, MDB_RDONLY, &txn);
  if (rc == LMDB_OK) {
    rc = mdb_cursor_open(txn, connection_.handle_->dbir, &cursor);
  }

  if (rc != LMDB_OK) {
    mdb_txn_abort(txn);
    std::string buff = common::MemSPrintf("DBKCOUNT function error: %s", mdb_strerror(rc));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  MDB_val key;
  MDB_val data;
  size_t sz = 0;
  while (mdb_cursor_get(cursor, &key, &data, MDB_NEXT) == LMDB_OK) {
    sz++;
  }
  mdb_cursor_close(cursor);
  mdb_txn_abort(txn);

  *size = sz;
  return common::Error();
}

common::Error DBConnection::FlushDBImpl() {
  MDB_cursor* cursor = NULL;
  MDB_txn* txn = NULL;
  int env_flags = connection_.config_.env_flags;
  int rc = mdb_txn_begin(connection_.handle_->env, NULL, lmdb_db_flag_from_env_flags(env_flags), &txn);
  if (rc == LMDB_OK) {
    rc = mdb_cursor_open(txn, connection_.handle_->dbir, &cursor);
  }

  if (rc != LMDB_OK) {
    mdb_txn_abort(txn);
    std::string buff = common::MemSPrintf("flushdb function error: %s", mdb_strerror(rc));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  MDB_val key;
  MDB_val data;
  size_t sz = 0;
  while (mdb_cursor_get(cursor, &key, &data, MDB_NEXT) == LMDB_OK) {
    sz++;
    rc = mdb_del(txn, connection_.handle_->dbir, &key, NULL);
    if (rc != LMDB_OK) {
      mdb_cursor_close(cursor);
      mdb_txn_abort(txn);
      std::string buff = common::MemSPrintf("del function error: %s", mdb_strerror(rc));
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }
  }

  mdb_cursor_close(cursor);
  if (sz != 0) {
    rc = mdb_txn_commit(txn);
    if (rc != LMDB_OK) {
      std::string buff = common::MemSPrintf("commit function error: %s", mdb_strerror(rc));
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }
    return common::Error();
  }

  mdb_txn_abort(txn);
  return common::Error();
}

common::Error DBConnection::SelectImpl(const std::string& name, IDataBaseInfo** info) {
  if (name != CurrentDBName()) {
    return ICommandTranslator::InvalidInputArguments("SELECT");
  }

  size_t kcount = 0;
  common::Error err = DBkcount(&kcount);
  DCHECK(!err);
  *info = new DataBaseInfo(name, true, kcount);
  return common::Error();
}

common::Error DBConnection::SetImpl(const NDbKValue& key, NDbKValue* added_key) {
  const NKey cur = key.GetKey();
  string_key_t key_str = cur.GetKey();
  std::string value_str = key.ValueString();
  common::Error err = SetInner(key_str, value_str);
  if (err && err->IsError()) {
    return err;
  }

  *added_key = key;
  return common::Error();
}

common::Error DBConnection::GetImpl(const NKey& key, NDbKValue* loaded_key) {
  string_key_t key_str = key.GetKey();
  std::string value_str;
  common::Error err = GetInner(key_str, &value_str);
  if (err && err->IsError()) {
    return err;
  }

  NValue val(common::Value::CreateStringValue(value_str));
  *loaded_key = NDbKValue(key, val);
  return common::Error();
}

common::Error DBConnection::DeleteImpl(const NKeys& keys, NKeys* deleted_keys) {
  for (size_t i = 0; i < keys.size(); ++i) {
    NKey key = keys[i];
    string_key_t key_str = key.GetKey();
    common::Error err = DelInner(key_str);
    if (err && err->IsError()) {
      continue;
    }

    deleted_keys->push_back(key);
  }

  return common::Error();
}

common::Error DBConnection::RenameImpl(const NKey& key, const std::string& new_key) {
  string_key_t key_str = key.GetKey();
  std::string value_str;
  common::Error err = GetInner(key_str, &value_str);
  if (err && err->IsError()) {
    return err;
  }

  err = DelInner(key_str);
  if (err && err->IsError()) {
    return err;
  }

  err = SetInner(new_key, value_str);
  if (err && err->IsError()) {
    return err;
  }

  return common::Error();
}

common::Error DBConnection::SetTTLImpl(const NKey& key, ttl_t ttl) {
  UNUSED(key);
  UNUSED(ttl);
  return common::make_error_value("Sorry, but now " PROJECT_NAME_TITLE " for LMDB not supported TTL commands.",
                                  common::ErrorValue::E_ERROR);
}

common::Error DBConnection::GetTTLImpl(const NKey& key, ttl_t* ttl) {
  UNUSED(key);
  UNUSED(ttl);
  return common::make_error_value("Sorry, but now " PROJECT_NAME_TITLE " for LMDB not supported TTL commands.",
                                  common::ErrorValue::E_ERROR);
}

common::Error DBConnection::QuitImpl() {
  common::Error err = Disconnect();
  if (err && err->IsError()) {
    return err;
  }

  return common::Error();
}

}  // namespace lmdb
}  // namespace core
}  // namespace fastonosql
