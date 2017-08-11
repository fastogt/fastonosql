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

#include "core/db/forestdb/db_connection.h"

#include <errno.h>   // for EACCES
#include <stdlib.h>  // for NULL, free, calloc
#include <time.h>    // for time_t
#include <string>    // for string

#include <libforestdb/forestdb.h>

#include <common/convert2string.h>
#include <common/file_system.h>
#include <common/utils.h>  // for c_strornull
#include <common/value.h>  // for StringValue (ptr only)

#include "core/db/forestdb/command_translator.h"
#include "core/db/forestdb/config.h"  // for Config
#include "core/db/forestdb/database_info.h"
#include "core/db/forestdb/internal/commands_api.h"

#include "core/global.h"  // for FastoObject, etc

namespace fastonosql {
namespace core {
namespace forestdb {
struct fdb {
  fdb_file_handle* handle;
  fdb_kvs_handle* kvs;
  char* db_name;
};

namespace {

fdb_status forestdb_open(fdb** context, const char* db_path, const char* db_name, fdb_config* fconfig) {
  fdb* lcontext = reinterpret_cast<fdb*>(calloc(1, sizeof(fdb)));
  fdb_status rc = fdb_open(&lcontext->handle, db_path, fconfig);
  if (rc != FDB_RESULT_SUCCESS) {
    free(lcontext);
    return rc;
  }

  fdb_kvs_config kvs_config = fdb_get_default_kvs_config();
  rc = fdb_kvs_open(lcontext->handle, &lcontext->kvs, db_name, &kvs_config);
  if (rc != FDB_RESULT_SUCCESS) {
    free(lcontext);
    return rc;
  }

  lcontext->db_name = common::utils::strdupornull(db_name);
  *context = lcontext;
  return rc;
}

void forestdb_close(fdb** context) {
  if (!context) {
    return;
  }

  fdb* lcontext = *context;
  if (!lcontext) {
    return;
  }

  fdb_commit_opt_t opt = FDB_COMMIT_NORMAL;
  fdb_commit(lcontext->handle, opt);
  common::utils::freeifnotnull(lcontext->db_name);
  fdb_kvs_close(lcontext->kvs);
  fdb_close(lcontext->handle);
  free(lcontext);
  *context = NULL;
}

}  // namespace
}  // namespace forestdb
namespace internal {
template <>
common::Error ConnectionAllocatorTraits<forestdb::NativeConnection, forestdb::Config>::Connect(
    const forestdb::Config& config,
    forestdb::NativeConnection** hout) {
  forestdb::NativeConnection* context = nullptr;
  common::Error er = forestdb::CreateConnection(config, &context);
  if (er && er->IsError()) {
    return er;
  }

  *hout = context;
  return common::Error();
}

template <>
common::Error ConnectionAllocatorTraits<forestdb::NativeConnection, forestdb::Config>::Disconnect(
    forestdb::NativeConnection** handle) {
  forestdb::forestdb_close(handle);
  *handle = nullptr;
  return common::Error();
}

template <>
bool ConnectionAllocatorTraits<forestdb::NativeConnection, forestdb::Config>::IsConnected(
    forestdb::NativeConnection* handle) {
  if (!handle) {
    return false;
  }

  return true;
}

template <>
const char* CDBConnection<forestdb::NativeConnection, forestdb::Config, FORESTDB>::BasedOn() {
  return "libforestdb";
}

template <>
const char* CDBConnection<forestdb::NativeConnection, forestdb::Config, FORESTDB>::VersionApi() {
  return fdb_get_lib_version();
}

template <>
ConstantCommandsArray CDBConnection<forestdb::NativeConnection, forestdb::Config, FORESTDB>::Commands() {
  return forestdb::g_commands;
}

}  // namespace internal

namespace forestdb {

common::Error CreateConnection(const Config& config, NativeConnection** context) {
  if (!context) {
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  DCHECK(*context == NULL);
  NativeConnection* lcontext = NULL;
  fdb_config fconfig = fdb_get_default_config();
  // fconfig.flags = FDB_OPEN_FLAG_CREATE;
  const char* db_path = common::utils::c_strornull(config.db_path);  // start point must be file
  const char* db_name = common::utils::c_strornull(config.db_name);
  fdb_status st = forestdb_open(&lcontext, db_path, db_name, &fconfig);
  if (st != FDB_RESULT_SUCCESS) {
    std::string buff = common::MemSPrintf("Fail open database: %s", fdb_error_msg(st));
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

  forestdb_close(&ldb);
  return common::Error();
}

DBConnection::DBConnection(CDBConnectionClient* client)
    : base_class(client, new CommandTranslator(base_class::Commands())) {}

std::string DBConnection::CurrentDBName() const {
  if (connection_.handle_) {
    return connection_.handle_->db_name;
  }

  DNOTREACHED();
  return base_class::CurrentDBName();
}

common::Error DBConnection::Info(const std::string& args, ServerInfo::Stats* statsout) {
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

common::Error DBConnection::SetInner(key_t key, const std::string& value) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  const string_key_t key_slice = key.ToString();
  fdb_status rc = fdb_set_kv(connection_.handle_->kvs, key_slice.data(), key_slice.size(), value.c_str(), value.size());
  if (rc != FDB_RESULT_SUCCESS) {
    std::string buff = common::MemSPrintf("set function error: %s", fdb_error_msg(rc));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error DBConnection::GetInner(key_t key, std::string* ret_val) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  const string_key_t key_slice = key.ToString();
  void* value_out = NULL;
  size_t valuelen_out = 0;
  fdb_status rc = fdb_get_kv(connection_.handle_->kvs, key_slice.data(), key_slice.size(), &value_out, &valuelen_out);
  if (rc != FDB_RESULT_SUCCESS) {
    std::string buff = common::MemSPrintf("get function error: %s", fdb_error_msg(rc));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  ret_val->assign(reinterpret_cast<const char*>(value_out), valuelen_out);
  return common::Error();
}

common::Error DBConnection::DelInner(key_t key) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  std::string exist_key;
  common::Error err = GetInner(key, &exist_key);
  if (err && err->IsError()) {
    return err;
  }

  const string_key_t key_slice = key.ToString();
  fdb_status rc = fdb_del_kv(connection_.handle_->kvs, key_slice.data(), key_slice.size());
  if (rc != FDB_RESULT_SUCCESS) {
    std::string buff = common::MemSPrintf("delete function error: %s", fdb_error_msg(rc));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error DBConnection::ScanImpl(uint64_t cursor_in,
                                     const std::string& pattern,
                                     uint64_t count_keys,
                                     std::vector<std::string>* keys_out,
                                     uint64_t* cursor_out) {
  fdb_iterator* it = NULL;
  fdb_iterator_opt_t opt = FDB_ITR_NONE;

  fdb_status rc = fdb_iterator_init(connection_.handle_->kvs, &it, NULL, 0, NULL, 0, opt);
  if (rc != FDB_RESULT_SUCCESS) {
    std::string buff = common::MemSPrintf("Keys function error: %s", fdb_error_msg(rc));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  fdb_doc* doc = NULL;
  uint64_t offset_pos = cursor_in;
  uint64_t lcursor_out = 0;
  std::vector<std::string> lkeys_out;
  do {
    rc = fdb_iterator_get(it, &doc);
    if (rc != FDB_RESULT_SUCCESS) {
      break;
    }

    if (lkeys_out.size() < count_keys) {
      std::string skey = std::string(static_cast<const char*>(doc->key), doc->keylen);
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
    fdb_doc_free(doc);
  } while (fdb_iterator_next(it) != FDB_RESULT_ITERATOR_FAIL);
  fdb_iterator_close(it);

  *keys_out = lkeys_out;
  *cursor_out = lcursor_out;
  return common::Error();
}

common::Error DBConnection::KeysImpl(const std::string& key_start,
                                     const std::string& key_end,
                                     uint64_t limit,
                                     std::vector<std::string>* ret) {
  fdb_iterator* it = NULL;
  fdb_iterator_opt_t opt = FDB_ITR_NONE;
  fdb_status rc = fdb_iterator_init(connection_.handle_->kvs, &it, key_start.c_str(), key_start.size(), key_end.c_str(),
                                    key_end.size(), opt);

  if (rc != FDB_RESULT_SUCCESS) {
    std::string buff = common::MemSPrintf("Keys function error: %s", fdb_error_msg(rc));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  fdb_doc* doc = NULL;
  do {
    rc = fdb_iterator_get(it, &doc);
    if (rc != FDB_RESULT_SUCCESS) {
      break;
    }

    std::string key = std::string(static_cast<const char*>(doc->key), doc->keylen);
    if (ret->size() < limit) {
      if (key < key_end) {
        ret->push_back(key);
      }
    } else {
      break;
    }
    fdb_doc_free(doc);
  } while (fdb_iterator_next(it) != FDB_RESULT_ITERATOR_FAIL);
  fdb_iterator_close(it);
  return common::Error();
}

common::Error DBConnection::DBkcountImpl(size_t* size) {
  fdb_iterator* it = NULL;
  fdb_iterator_opt_t opt = FDB_ITR_NONE;

  fdb_status rc = fdb_iterator_init(connection_.handle_->kvs, &it, NULL, 0, NULL, 0, opt);
  if (rc != FDB_RESULT_SUCCESS) {
    std::string buff = common::MemSPrintf("Keys function error: %s", fdb_error_msg(rc));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  size_t sz = 0;
  fdb_doc* doc = NULL;
  do {
    rc = fdb_iterator_get(it, &doc);
    if (rc != FDB_RESULT_SUCCESS) {
      break;
    }

    sz++;
    fdb_doc_free(doc);
  } while (fdb_iterator_next(it) != FDB_RESULT_ITERATOR_FAIL);
  fdb_iterator_close(it);

  *size = sz;
  return common::Error();
}

common::Error DBConnection::FlushDBImpl() {
  fdb_iterator* it = NULL;
  fdb_iterator_opt_t opt = FDB_ITR_NONE;

  fdb_status rc = fdb_iterator_init(connection_.handle_->kvs, &it, NULL, 0, NULL, 0, opt);
  if (rc != FDB_RESULT_SUCCESS) {
    std::string buff = common::MemSPrintf("Keys function error: %s", fdb_error_msg(rc));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  fdb_doc* doc = NULL;
  do {
    rc = fdb_iterator_get(it, &doc);
    if (rc != FDB_RESULT_SUCCESS) {
      break;
    }

    std::string key;
    rc = fdb_del_kv(connection_.handle_->kvs, key.c_str(), key.size());
    if (rc != FDB_RESULT_SUCCESS) {
      fdb_iterator_close(it);
      std::string buff = common::MemSPrintf("del function error: %s", fdb_error_msg(rc));
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }
    fdb_doc_free(doc);
  } while (fdb_iterator_next(it) != FDB_RESULT_ITERATOR_FAIL);
  fdb_iterator_close(it);

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
  key_t key_str = cur.GetKey();
  std::string value_str = key.ValueString();
  common::Error err = SetInner(key_str, value_str);
  if (err && err->IsError()) {
    return err;
  }

  *added_key = key;
  return common::Error();
}

common::Error DBConnection::GetImpl(const NKey& key, NDbKValue* loaded_key) {
  key_t key_str = key.GetKey();
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
    key_t key_str = key.GetKey();
    common::Error err = DelInner(key_str);
    if (err && err->IsError()) {
      continue;
    }

    deleted_keys->push_back(key);
  }

  return common::Error();
}

common::Error DBConnection::RenameImpl(const NKey& key, string_key_t new_key) {
  key_t key_str = key.GetKey();
  std::string value_str;
  common::Error err = GetInner(key_str, &value_str);
  if (err && err->IsError()) {
    return err;
  }

  err = DelInner(key_str);
  if (err && err->IsError()) {
    return err;
  }

  err = SetInner(key_t(new_key), value_str);
  if (err && err->IsError()) {
    return err;
  }

  return common::Error();
}

common::Error DBConnection::SetTTLImpl(const NKey& key, ttl_t ttl) {
  UNUSED(key);
  UNUSED(ttl);
  return common::make_error_value("Sorry, but now " PROJECT_NAME_TITLE " for ForestDB not supported TTL commands.",
                                  common::ErrorValue::E_ERROR);
}

common::Error DBConnection::GetTTLImpl(const NKey& key, ttl_t* ttl) {
  UNUSED(key);
  UNUSED(ttl);
  return common::make_error_value("Sorry, but now " PROJECT_NAME_TITLE " for ForestDB not supported TTL commands.",
                                  common::ErrorValue::E_ERROR);
}

common::Error DBConnection::QuitImpl() {
  common::Error err = Disconnect();
  if (err && err->IsError()) {
    return err;
  }

  return common::Error();
}

}  // namespace forestdb
}  // namespace core
}  // namespace fastonosql
