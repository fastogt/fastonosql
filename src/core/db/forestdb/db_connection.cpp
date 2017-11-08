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

#include <libforestdb/forestdb.h>

#include <common/file_system/string_path_utils.h>
#include <common/utils.h>  // for c_strornull

#include "core/db/forestdb/command_translator.h"
#include "core/db/forestdb/database_info.h"
#include "core/db/forestdb/internal/commands_api.h"

namespace fastonosql {
namespace core {
template <>
const char* ConnectionTraits<FORESTDB>::GetBasedOn() {
  return "libforestdb";
}

template <>
const char* ConnectionTraits<FORESTDB>::GetVersionApi() {
  return fdb_get_lib_version();
}
namespace forestdb {
struct fdb {
  fdb_file_handle* handle;
  fdb_kvs_handle* kvs;
  char* db_name;
};

namespace {

const ConstantCommandsArray g_commands = {CommandHolder(DB_HELP_COMMAND,
                                                        "[command]",
                                                        "Return how to use command",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        0,
                                                        1,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Help),
                                          CommandHolder(DB_INFO_COMMAND,
                                                        "[section]",
                                                        "These command return database information.",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        0,
                                                        1,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Info),
                                          CommandHolder("CONFIG GET",
                                                        "<parameter>",
                                                        "Get the value of a configuration parameter",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        1,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::ConfigGet),
                                          CommandHolder(DB_CREATEDB_COMMAND,
                                                        "<name>",
                                                        "Create database",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        1,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::CreateDatabase),
                                          CommandHolder(DB_REMOVEDB_COMMAND,
                                                        "<name>",
                                                        "Remove database",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        1,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::RemoveDatabase),
                                          CommandHolder(DB_SCAN_COMMAND,
                                                        "<cursor> [MATCH pattern] [COUNT count]",
                                                        "Incrementally iterate the keys space",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        1,
                                                        4,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Scan),
                                          CommandHolder(DB_KEYS_COMMAND,
                                                        "<key_start> <key_end> <limit>",
                                                        "Find all keys matching the given limits.",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        3,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Keys),
                                          CommandHolder(DB_DBKCOUNT_COMMAND,
                                                        "-",
                                                        "Return the number of keys in the "
                                                        "selected database",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        0,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::DBkcount),
                                          CommandHolder(DB_FLUSHDB_COMMAND,
                                                        "-",
                                                        "Remove all keys from the current database",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        0,
                                                        1,
                                                        CommandInfo::Native,
                                                        &CommandsApi::FlushDB),
                                          CommandHolder(DB_SELECTDB_COMMAND,
                                                        "<name>",
                                                        "Change the selected database for the "
                                                        "current connection",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        1,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Select),
                                          CommandHolder(DB_SET_KEY_COMMAND,
                                                        "<key> <value>",
                                                        "Set the value of a key.",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        2,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Set),
                                          CommandHolder(DB_GET_KEY_COMMAND,
                                                        "<key>",
                                                        "Get the value of a key.",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        1,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Get),
                                          CommandHolder(DB_RENAME_KEY_COMMAND,
                                                        "<key> <newkey>",
                                                        "Rename a key",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        2,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Rename),
                                          CommandHolder(DB_DELETE_KEY_COMMAND,
                                                        "<key> [key ...]",
                                                        "Delete key.",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        1,
                                                        INFINITE_COMMAND_ARGS,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Delete),
                                          CommandHolder(DB_QUIT_COMMAND,
                                                        "-",
                                                        "Close the connection",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        0,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Quit)};

fdb_status forestdb_create_db(fdb* context, const char* db_name) {
  if (!context || !db_name) {
    return FDB_RESULT_INVALID_ARGS;
  }

  fdb_kvs_config kvs_config = fdb_get_default_kvs_config();
  kvs_config.create_if_missing = true;
  fdb_kvs_handle* kvs = NULL;
  fdb_status rc = fdb_kvs_open(context->handle, &kvs, db_name, &kvs_config);
  if (rc != FDB_RESULT_SUCCESS) {
    return rc;
  }

  fdb_commit_opt_t opt = FDB_COMMIT_NORMAL;
  fdb_commit(context->handle, opt);
  fdb_kvs_close(kvs);
  return FDB_RESULT_SUCCESS;
}

fdb_status forestdb_remove_db(fdb* context, const char* db_name) {
  if (!context || !db_name) {
    return FDB_RESULT_INVALID_ARGS;
  }

  fdb_status rc = fdb_kvs_remove(context->handle, db_name);
  if (rc != FDB_RESULT_SUCCESS) {
    return rc;
  }

  fdb_commit_opt_t opt = FDB_COMMIT_NORMAL;
  fdb_commit(context->handle, opt);
  return FDB_RESULT_SUCCESS;
}

fdb_status forestdb_select(fdb* context, const char* db_name) {
  if (!context || !db_name) {  // only for named dbs
    return FDB_RESULT_INVALID_ARGS;
  }

  if (context->db_name && strcmp(db_name, context->db_name) == 0) {  // lazy select
    return FDB_RESULT_SUCCESS;
  }

  // open db
  fdb_kvs_config kvs_config = fdb_get_default_kvs_config();
  kvs_config.create_if_missing = false;
  fdb_kvs_handle* kvs = NULL;
  fdb_status rc = fdb_kvs_open(context->handle, &kvs, db_name, &kvs_config);
  if (rc != FDB_RESULT_SUCCESS) {
    return rc;
  }

  // cleanup old ref
  fdb_commit_opt_t opt = FDB_COMMIT_NORMAL;
  fdb_commit(context->handle, opt);
  common::utils::freeifnotnull(context->db_name);
  context->db_name = NULL;
  fdb_kvs_close(context->kvs);
  context->kvs = NULL;

  // assigne new
  context->kvs = kvs;
  context->db_name = common::utils::strdupornull(db_name);
  return FDB_RESULT_SUCCESS;
}

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
  common::Error err = forestdb::CreateConnection(config, &context);
  if (err) {
    return err;
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
const ConstantCommandsArray& CDBConnection<forestdb::NativeConnection, forestdb::Config, FORESTDB>::GetCommands() {
  return forestdb::g_commands;
}

}  // namespace internal

namespace forestdb {

common::Error CreateConnection(const Config& config, NativeConnection** context) {
  if (!context) {
    return common::make_error_inval();
  }

  DCHECK(*context == NULL);
  NativeConnection* lcontext = NULL;
  fdb_config fconfig = fdb_get_default_config();
  // fconfig.flags = FDB_OPEN_FLAG_CREATE;
  const char* db_path = config.db_path.empty() ? NULL : config.db_path.c_str();  // start point must be file
  const char* db_name = config.db_name.empty() ? NULL : config.db_name.c_str();
  fdb_status st = forestdb_open(&lcontext, db_path, db_name, &fconfig);
  if (st != FDB_RESULT_SUCCESS) {
    std::string buff = common::MemSPrintf("Fail open database: %s", fdb_error_msg(st));
    return common::make_error(buff);
  }

  *context = lcontext;
  return common::Error();
}

common::Error TestConnection(const Config& config) {
  NativeConnection* ldb = nullptr;
  common::Error err = CreateConnection(config, &ldb);
  if (err) {
    return err;
  }

  forestdb_close(&ldb);
  return common::Error();
}

DBConnection::DBConnection(CDBConnectionClient* client)
    : base_class(client, new CommandTranslator(base_class::GetCommands())) {}

std::string DBConnection::GetCurrentDBName() const {
  if (IsConnected()) {  // if connected
    auto conf = GetConfig();
    return connection_.handle_->db_name ? connection_.handle_->db_name : conf->db_name;
  }

  DNOTREACHED() << "GetCurrentDBName failed!";
  return base_class::GetCurrentDBName();
}

common::Error DBConnection::Info(const std::string& args, ServerInfo::Stats* statsout) {
  UNUSED(args);
  if (!statsout) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  ServerInfo::Stats linfo;
  auto conf = GetConfig();
  linfo.db_path = conf->db_path;

  *statsout = linfo;
  return common::Error();
}

common::Error DBConnection::ConfigGetDatabases(std::vector<std::string>* dbs) {
  if (!dbs) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  fdb_kvs_name_list forestdb_dbs;
  err = CheckResultCommand("CONFIG GET DATABASES", fdb_get_kvs_name_list(connection_.handle_->handle, &forestdb_dbs));
  if (err) {
    return err;
  }

  for (size_t i = 0; i < forestdb_dbs.num_kvs_names; ++i) {
    dbs->push_back(forestdb_dbs.kvs_names[i]);
  }

  err = CheckResultCommand("CONFIG GET DATABASES", fdb_free_kvs_name_list(&forestdb_dbs));
  if (err) {
    return err;
  }
  return common::Error();
}

common::Error DBConnection::SetInner(key_t key, const std::string& value) {
  const string_key_t key_slice = key.GetKeyData();
  return CheckResultCommand(DB_SET_KEY_COMMAND, fdb_set_kv(connection_.handle_->kvs, key_slice.data(), key_slice.size(),
                                                           value.c_str(), value.size()));
}

common::Error DBConnection::GetInner(key_t key, std::string* ret_val) {
  const string_key_t key_slice = key.GetKeyData();
  void* value_out = NULL;
  size_t valuelen_out = 0;
  common::Error err = CheckResultCommand(DB_GET_KEY_COMMAND, fdb_get_kv(connection_.handle_->kvs, key_slice.data(),
                                                                        key_slice.size(), &value_out, &valuelen_out));
  if (err) {
    return err;
  }

  *ret_val = std::string(reinterpret_cast<const char*>(value_out), valuelen_out);
  return common::Error();
}

common::Error DBConnection::DelInner(key_t key) {
  std::string exist_key;
  common::Error err = GetInner(key, &exist_key);
  if (err) {
    return err;
  }

  const string_key_t key_slice = key.GetKeyData();
  return CheckResultCommand(DB_DELETE_KEY_COMMAND,
                            fdb_del_kv(connection_.handle_->kvs, key_slice.data(), key_slice.size()));
}

common::Error DBConnection::ScanImpl(uint64_t cursor_in,
                                     const std::string& pattern,
                                     uint64_t count_keys,
                                     std::vector<std::string>* keys_out,
                                     uint64_t* cursor_out) {
  fdb_iterator* it = NULL;
  fdb_iterator_opt_t opt = FDB_ITR_NONE;

  common::Error err =
      CheckResultCommand(DB_SCAN_COMMAND, fdb_iterator_init(connection_.handle_->kvs, &it, NULL, 0, NULL, 0, opt));
  if (err) {
    return err;
  }

  fdb_doc* doc = NULL;
  uint64_t offset_pos = cursor_in;
  uint64_t lcursor_out = 0;
  std::vector<std::string> lkeys_out;
  do {
    fdb_status rc = fdb_iterator_get(it, &doc);
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
  common::Error err =
      CheckResultCommand(DB_KEYS_COMMAND, fdb_iterator_init(connection_.handle_->kvs, &it, key_start.c_str(),
                                                            key_start.size(), key_end.c_str(), key_end.size(), opt));
  if (err) {
    return err;
  }

  fdb_doc* doc = NULL;
  do {
    fdb_status rc = fdb_iterator_get(it, &doc);
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

  common::Error err =
      CheckResultCommand(DB_DBKCOUNT_COMMAND, fdb_iterator_init(connection_.handle_->kvs, &it, NULL, 0, NULL, 0, opt));
  if (err) {
    return err;
  }

  size_t sz = 0;
  fdb_doc* doc = NULL;
  do {
    fdb_status rc = fdb_iterator_get(it, &doc);
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

  common::Error err =
      CheckResultCommand(DB_FLUSHDB_COMMAND, fdb_iterator_init(connection_.handle_->kvs, &it, NULL, 0, NULL, 0, opt));
  if (err) {
    return err;
  }

  fdb_doc* doc = NULL;
  do {
    fdb_status rc = fdb_iterator_get(it, &doc);
    if (rc != FDB_RESULT_SUCCESS) {
      break;
    }

    std::string key;
    err = CheckResultCommand(DB_FLUSHDB_COMMAND, fdb_del_kv(connection_.handle_->kvs, key.c_str(), key.size()));
    if (err) {
      fdb_iterator_close(it);
      return err;
    }
    fdb_doc_free(doc);
  } while (fdb_iterator_next(it) != FDB_RESULT_ITERATOR_FAIL);
  fdb_iterator_close(it);

  return common::Error();
}

common::Error DBConnection::CreateDBImpl(const std::string& name, IDataBaseInfo** info) {
  auto conf = GetConfig();
  const char* db_name = name.c_str();
  common::Error err = CheckResultCommand(DB_CREATEDB_COMMAND, forestdb_create_db(connection_.handle_, db_name));
  if (err) {
    return err;
  }

  *info = new DataBaseInfo(name, false, 0);
  return common::Error();
}

common::Error DBConnection::RemoveDBImpl(const std::string& name, IDataBaseInfo** info) {
  auto conf = GetConfig();
  const char* db_name = name.c_str();
  common::Error err = CheckResultCommand(DB_REMOVEDB_COMMAND, forestdb_remove_db(connection_.handle_, db_name));
  if (err) {
    return err;
  }

  *info = new DataBaseInfo(name, false, 0);
  return common::Error();
}

common::Error DBConnection::SelectImpl(const std::string& name, IDataBaseInfo** info) {
  auto conf = GetConfig();
  common::Error err = CheckResultCommand(DB_SELECTDB_COMMAND, forestdb_select(connection_.handle_, name.c_str()));
  if (err) {
    return err;
  }

  connection_.config_->db_name = name;
  size_t kcount = 0;
  err = DBkcount(&kcount);
  DCHECK(!err) << "DBkcount failed!";
  *info = new DataBaseInfo(name, true, kcount);
  return common::Error();
}

common::Error DBConnection::SetImpl(const NDbKValue& key, NDbKValue* added_key) {
  const NKey cur = key.GetKey();
  key_t key_str = cur.GetKey();
  std::string value_str = key.GetValueString();
  common::Error err = SetInner(key_str, value_str);
  if (err) {
    return err;
  }

  *added_key = key;
  return common::Error();
}

common::Error DBConnection::GetImpl(const NKey& key, NDbKValue* loaded_key) {
  key_t key_str = key.GetKey();
  std::string value_str;
  common::Error err = GetInner(key_str, &value_str);
  if (err) {
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
    if (err) {
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
  if (err) {
    return err;
  }

  err = DelInner(key_str);
  if (err) {
    return err;
  }

  err = SetInner(key_t(new_key), value_str);
  if (err) {
    return err;
  }

  return common::Error();
}

common::Error DBConnection::QuitImpl() {
  common::Error err = Disconnect();
  if (err) {
    return err;
  }

  return common::Error();
}

common::Error DBConnection::CheckResultCommand(const std::string& cmd, fdb_status err) {
  if (err != FDB_RESULT_SUCCESS) {
    return GenerateError(cmd, fdb_error_msg(err));
  }

  return common::Error();
}

}  // namespace forestdb
}  // namespace core
}  // namespace fastonosql
