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

#include "core/db/lmdb/db_connection.h"

#include <errno.h>   // for EACCES
#include <lmdb.h>    // for mdb_txn_abort, MDB_val
#include <stdlib.h>  // for NULL, free, calloc
#include <time.h>    // for time_t
#include <string>    // for string

#include <common/convert2string.h>
#include <common/file_system/string_path_utils.h>
#include <common/utils.h>  // for c_strornull
#include <common/value.h>  // for StringValue (ptr only)

#include "core/db/lmdb/command_translator.h"
#include "core/db/lmdb/config.h"  // for Config
#include "core/db/lmdb/database_info.h"
#include "core/db/lmdb/internal/commands_api.h"

#define LMDB_OK 0

namespace fastonosql {
namespace core {
namespace lmdb {
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
                                          CommandHolder(DB_GET_CONFIG_COMMAND,
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
                                          CommandHolder(LMDB_DROPDB_COMMAND,
                                                        "-",
                                                        "Drop database",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        0,
                                                        1,
                                                        CommandInfo::Native,
                                                        &CommandsApi::DropDatabase),
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
}
}  // namespace lmdb
template <>
const char* ConnectionTraits<LMDB>::GetBasedOn() {
  return "liblmdb";
}

template <>
const char* ConnectionTraits<LMDB>::GetVersionApi() {
  return STRINGIZE(MDB_VERSION_MAJOR) "." STRINGIZE(MDB_VERSION_MINOR) "." STRINGIZE(MDB_VERSION_PATCH);
}

namespace {
MDB_val ConvertToLMDBSlice(const char* key, size_t len) {
  MDB_val mkey;
  mkey.mv_size = len;
  mkey.mv_data = const_cast<command_buffer_char_t*>(key);
  return mkey;
}
}  // namespace
namespace lmdb {
struct lmdb {
  MDB_env* env;
  MDB_dbi dbi;
  char* db_name;
};

namespace {

unsigned int lmdb_db_flag_from_env_flags(int env_flags) {
  return (env_flags & MDB_RDONLY) ? MDB_RDONLY : 0;
}

int lmdb_create_db(lmdb* context, const char* db_name, int env_flags) {
  if (!context || !db_name) {
    return EINVAL;
  }

  MDB_txn* txn = NULL;
  int rc = mdb_txn_begin(context->env, NULL, lmdb_db_flag_from_env_flags(env_flags), &txn);
  if (rc != LMDB_OK) {
    return rc;
  }

  MDB_dbi ldbi = 0;
  unsigned int flg = env_flags & MDB_RDONLY ? 0 : MDB_CREATE;
  rc = mdb_dbi_open(txn, db_name, flg, &ldbi);
  if (rc != LMDB_OK) {
    mdb_txn_abort(txn);
    return rc;
  }

  mdb_txn_commit(txn);
  mdb_dbi_close(context->env, ldbi);
  return LMDB_OK;
}

int lmdb_remove_db(lmdb* context, const char* db_name, int env_flags) {
  if (!context || !db_name) {
    return EINVAL;
  }

  const unsigned int flg = lmdb_db_flag_from_env_flags(env_flags);
  MDB_dbi ldbi = 0;
  MDB_txn* txn = NULL;
  int rc = mdb_txn_begin(context->env, NULL, flg, &txn);
  if (rc != LMDB_OK) {
    return rc;
  }

  rc = mdb_dbi_open(txn, db_name, flg, &ldbi);
  if (rc != LMDB_OK) {
    mdb_txn_abort(txn);
    return rc;
  }

  rc = mdb_drop(txn, ldbi, 1);
  if (rc != LMDB_OK) {
    mdb_txn_abort(txn);
    return rc;
  }

  return mdb_txn_commit(txn);
}

int lmdb_select(lmdb* context, const char* db_name, int env_flags) {
  if (!context || !db_name) {  // only for named dbs
    return EINVAL;
  }

  if (context->db_name && strcmp(db_name, context->db_name) == 0) {  // lazy select
    return LMDB_OK;
  }

  // open db
  MDB_txn* txn = NULL;
  int rc = mdb_txn_begin(context->env, NULL, lmdb_db_flag_from_env_flags(env_flags), &txn);
  if (rc != LMDB_OK) {
    return rc;
  }

  MDB_dbi ldbi = 0;
  const unsigned int flg = env_flags & MDB_RDONLY ? 0 : MDB_CREATE;
  rc = mdb_dbi_open(txn, db_name, flg, &ldbi);
  if (rc != LMDB_OK) {
    mdb_txn_abort(txn);
    return rc;
  }

  rc = mdb_txn_commit(txn);
  if (rc != LMDB_OK) {
    return rc;
  }

  // cleanup old ref
  common::utils::freeifnotnull(context->db_name);
  context->db_name = NULL;
  mdb_dbi_close(context->env, context->dbi);
  context->dbi = 0;

  // assigne new
  context->dbi = ldbi;
  context->db_name = common::utils::strdupornull(db_name);
  return LMDB_OK;
}

int lmdb_open(lmdb** context, const char* db_path, const char* db_name, int env_flags, MDB_dbi max_dbs) {
  lmdb* lcontext = reinterpret_cast<lmdb*>(calloc(1, sizeof(lmdb)));
  int rc = mdb_env_create(&lcontext->env);
  if (rc != LMDB_OK) {
    free(lcontext);
    return rc;
  }

  rc = mdb_env_set_maxdbs(lcontext->env, max_dbs);
  if (rc != LMDB_OK) {
    free(lcontext);
    return rc;
  }

  rc = mdb_env_open(lcontext->env, db_path, env_flags, 0664);
  if (rc != LMDB_OK) {
    free(lcontext);
    return rc;
  }

  rc = lmdb_select(lcontext, db_name, env_flags);
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

  common::utils::freeifnotnull(lcontext->db_name);
  lcontext->db_name = NULL;
  mdb_dbi_close(lcontext->env, lcontext->dbi);
  lcontext->dbi = 0;
  mdb_env_close(lcontext->env);
  lcontext->env = NULL;
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
  common::Error err = lmdb::CreateConnection(config, &context);
  if (err) {
    return err;
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
const ConstantCommandsArray& CDBConnection<lmdb::NativeConnection, lmdb::Config, LMDB>::GetCommands() {
  return lmdb::g_commands;
}

}  // namespace internal

namespace lmdb {

common::Error CreateConnection(const Config& config, NativeConnection** context) {
  if (!context || config.db_name.empty() || config.db_path.empty()) {  // only named dbs
    return common::make_error_inval();
  }

  DCHECK(*context == NULL);
  struct lmdb* lcontext = NULL;
  std::string db_path = config.db_path;
  std::string folder = common::file_system::get_dir_path(db_path);
  bool is_dir_exist = common::file_system::is_directory_exist(folder);
  if (!is_dir_exist) {
    return common::make_error(
        common::MemSPrintf("Invalid input path: (%s), There is already a directory as the file name you specified, "
                           "specify a different name.",
                           folder));
  }

  bool is_single_file = config.IsSingleFileDB();
  common::tribool is_dir = common::file_system::is_directory(db_path);
  if (is_dir == common::SUCCESS) {
    if (is_single_file) {  // if dir but want single file
      return common::make_error(common::MemSPrintf("Invalid input path: (%s), please remove folder.", db_path));
    }
  } else if (is_dir == common::FAIL) {  // if file
    if (!is_single_file) {              // if file but want dir
      return common::make_error(
          common::MemSPrintf("Invalid input path: (%s), There is already a file as the folder name you specified, "
                             "specify a different name.",
                             db_path));
    }
  } else {                  // if nothing created
    if (!is_single_file) {  // if file but want dir
      return common::make_error(common::MemSPrintf("Invalid input path: (%s), please create folder.", db_path));
    }
  }

  const char* db_path_ptr = db_path.c_str();
  int env_flags = config.env_flags;
  unsigned int max_dbs = config.max_dbs;
  const char* db_name_ptr = config.db_name.c_str();
  int st = lmdb_open(&lcontext, db_path_ptr, db_name_ptr, env_flags, max_dbs);
  if (st != LMDB_OK) {
    std::string buff = common::MemSPrintf("Fail open database: %s", mdb_strerror(st));
    return common::make_error(buff);
  }

  *context = lcontext;
  return common::Error();
}  // namespace lmdb

common::Error TestConnection(const Config& config) {
  NativeConnection* ldb = nullptr;
  common::Error err = CreateConnection(config, &ldb);
  if (err) {
    return err;
  }

  lmdb_close(&ldb);
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

common::Error DBConnection::DropDatabase() {
  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  MDB_txn* txn = NULL;
  auto conf = GetConfig();
  int env_flags = conf->env_flags;
  err = CheckResultCommand(LMDB_DROPDB_COMMAND,
                           mdb_txn_begin(connection_.handle_->env, NULL, lmdb_db_flag_from_env_flags(env_flags), &txn));
  if (err) {
    return err;
  }

  err = CheckResultCommand(LMDB_DROPDB_COMMAND, mdb_drop(txn, connection_.handle_->dbi, 1));
  if (err) {
    mdb_txn_abort(txn);
    return err;
  }

  return CheckResultCommand(LMDB_DROPDB_COMMAND, mdb_txn_commit(txn));
}

common::Error DBConnection::SetInner(key_t key, const std::string& value) {
  const string_key_t key_str = key.GetKeyData();
  MDB_val key_slice = ConvertToLMDBSlice(key_str.data(), key_str.size());
  MDB_val mval;
  mval.mv_size = value.size();
  mval.mv_data = const_cast<char*>(value.c_str());

  MDB_txn* txn = NULL;
  auto conf = GetConfig();
  int env_flags = conf->env_flags;
  common::Error err = CheckResultCommand(
      DB_SET_KEY_COMMAND, mdb_txn_begin(connection_.handle_->env, NULL, lmdb_db_flag_from_env_flags(env_flags), &txn));
  if (err) {
    return err;
  }
  err = CheckResultCommand(DB_SET_KEY_COMMAND, mdb_put(txn, connection_.handle_->dbi, &key_slice, &mval, 0));
  if (err) {
    mdb_txn_abort(txn);
    return err;
  }

  return CheckResultCommand(DB_SET_KEY_COMMAND, mdb_txn_commit(txn));
}

common::Error DBConnection::GetInner(key_t key, std::string* ret_val) {
  const string_key_t key_str = key.GetKeyData();
  MDB_val key_slice = ConvertToLMDBSlice(key_str.data(), key_str.size());
  MDB_val mval;

  MDB_txn* txn = NULL;
  common::Error err =
      CheckResultCommand(DB_GET_KEY_COMMAND, mdb_txn_begin(connection_.handle_->env, NULL, MDB_RDONLY, &txn));
  if (err) {
    return err;
  }

  err = CheckResultCommand(DB_GET_KEY_COMMAND, mdb_get(txn, connection_.handle_->dbi, &key_slice, &mval));
  mdb_txn_abort(txn);
  if (err) {
    return err;
  }

  *ret_val = std::string(reinterpret_cast<const char*>(mval.mv_data), mval.mv_size);
  return common::Error();
}

common::Error DBConnection::DelInner(key_t key) {
  const string_key_t key_str = key.GetKeyData();
  MDB_val key_slice = ConvertToLMDBSlice(key_str.data(), key_str.size());

  MDB_txn* txn = NULL;
  auto conf = GetConfig();
  int env_flags = conf->env_flags;
  common::Error err =
      CheckResultCommand(DB_DELETE_KEY_COMMAND,
                         mdb_txn_begin(connection_.handle_->env, NULL, lmdb_db_flag_from_env_flags(env_flags), &txn));
  if (err) {
    return err;
  }

  err = CheckResultCommand(DB_DELETE_KEY_COMMAND, mdb_del(txn, connection_.handle_->dbi, &key_slice, NULL));
  if (err) {
    mdb_txn_abort(txn);
    return err;
  }

  return CheckResultCommand(DB_DELETE_KEY_COMMAND, mdb_txn_commit(txn));
}

common::Error DBConnection::ScanImpl(uint64_t cursor_in,
                                     const std::string& pattern,
                                     uint64_t count_keys,
                                     std::vector<std::string>* keys_out,
                                     uint64_t* cursor_out) {
  MDB_cursor* cursor = NULL;
  MDB_txn* txn = NULL;
  common::Error err =
      CheckResultCommand(DB_SCAN_COMMAND, mdb_txn_begin(connection_.handle_->env, NULL, MDB_RDONLY, &txn));
  if (err) {
    return err;
  }

  err = CheckResultCommand(DB_SCAN_COMMAND, mdb_cursor_open(txn, connection_.handle_->dbi, &cursor));
  if (err) {
    mdb_txn_abort(txn);
    return err;
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
  common::Error err =
      CheckResultCommand(DB_KEYS_COMMAND, mdb_txn_begin(connection_.handle_->env, NULL, MDB_RDONLY, &txn));
  if (err) {
    return err;
  }

  err = CheckResultCommand(DB_KEYS_COMMAND, mdb_cursor_open(txn, connection_.handle_->dbi, &cursor));
  if (err) {
    mdb_txn_abort(txn);
    return err;
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
  common::Error err =
      CheckResultCommand(DB_DBKCOUNT_COMMAND, mdb_txn_begin(connection_.handle_->env, NULL, MDB_RDONLY, &txn));
  if (err) {
    return err;
  }

  err = CheckResultCommand(DB_DBKCOUNT_COMMAND, mdb_cursor_open(txn, connection_.handle_->dbi, &cursor));
  if (err) {
    mdb_txn_abort(txn);
    return err;
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

common::Error DBConnection::CreateDBImpl(const std::string& name, IDataBaseInfo** info) {
  auto conf = GetConfig();
  int env_flags = conf->env_flags;
  const char* db_name = name.c_str();
  common::Error err = CheckResultCommand(DB_CREATEDB_COMMAND, lmdb_create_db(connection_.handle_, db_name, env_flags));
  if (err) {
    return err;
  }

  *info = new DataBaseInfo(name, false, 0);
  return common::Error();
}

common::Error DBConnection::RemoveDBImpl(const std::string& name, IDataBaseInfo** info) {
  auto conf = GetConfig();
  int env_flags = conf->env_flags;
  const char* db_name = name.c_str();
  common::Error err = CheckResultCommand(DB_REMOVEDB_COMMAND, lmdb_remove_db(connection_.handle_, db_name, env_flags));
  if (err) {
    return err;
  }

  *info = new DataBaseInfo(name, false, 0);
  return common::Error();
}

common::Error DBConnection::FlushDBImpl() {
  MDB_cursor* cursor = NULL;
  MDB_txn* txn = NULL;
  auto conf = GetConfig();
  int env_flags = conf->env_flags;
  common::Error err = CheckResultCommand(
      DB_FLUSHDB_COMMAND, mdb_txn_begin(connection_.handle_->env, NULL, lmdb_db_flag_from_env_flags(env_flags), &txn));
  if (err) {
    return err;
  }

  err = CheckResultCommand(DB_FLUSHDB_COMMAND, mdb_cursor_open(txn, connection_.handle_->dbi, &cursor));
  if (err) {
    mdb_txn_abort(txn);
    return err;
  }

  MDB_val key;
  MDB_val data;
  size_t sz = 0;
  while (mdb_cursor_get(cursor, &key, &data, MDB_NEXT) == LMDB_OK) {
    sz++;
    err = CheckResultCommand(DB_FLUSHDB_COMMAND, mdb_del(txn, connection_.handle_->dbi, &key, NULL));
    if (err) {
      mdb_cursor_close(cursor);
      mdb_txn_abort(txn);
      return err;
    }
  }

  mdb_cursor_close(cursor);
  if (sz != 0) {
    return CheckResultCommand(DB_DELETE_KEY_COMMAND, mdb_txn_commit(txn));
  }

  mdb_txn_abort(txn);
  return common::Error();
}

common::Error DBConnection::SelectImpl(const std::string& name, IDataBaseInfo** info) {
  auto conf = GetConfig();
  int env_flags = conf->env_flags;
  common::Error err =
      CheckResultCommand(DB_SELECTDB_COMMAND, lmdb_select(connection_.handle_, name.c_str(), env_flags));
  if (err) {
    return err;
  }

  connection_.config_->db_name = name;
  size_t kcount = 0;
  err = DBkcount(&kcount);
  DCHECK(!err) << err->GetDescription();
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

common::Error DBConnection::ConfigGetDatabasesImpl(std::vector<std::string>* dbs) {
  MDB_dbi ldbi = 0;
  {
    MDB_txn* txn = NULL;
    common::Error err =
        CheckResultCommand("CONFIG GET DATABASES", mdb_txn_begin(connection_.handle_->env, NULL, MDB_RDONLY, &txn));
    if (err) {
      return err;
    }

    err = CheckResultCommand("CONFIG GET DATABASES", mdb_dbi_open(txn, NULL, 0, &ldbi));
    mdb_txn_abort(txn);
    if (err) {
      return err;
    }
  }

  MDB_cursor* cursor = NULL;
  MDB_txn* txn_dbs = NULL;
  common::Error err =
      CheckResultCommand("CONFIG GET DATABASES", mdb_txn_begin(connection_.handle_->env, NULL, MDB_RDONLY, &txn_dbs));
  if (err) {
    mdb_dbi_close(connection_.handle_->env, ldbi);
    return err;
  }

  err = CheckResultCommand("CONFIG GET DATABASES", mdb_cursor_open(txn_dbs, ldbi, &cursor));
  if (err) {
    mdb_txn_abort(txn_dbs);
    mdb_dbi_close(connection_.handle_->env, ldbi);
    return err;
  }

  MDB_val key;
  MDB_val data;
  while ((mdb_cursor_get(cursor, &key, &data, MDB_NEXT) == LMDB_OK)) {
    std::string skey(reinterpret_cast<const char*>(key.mv_data), key.mv_size);
    // std::string sdata(reinterpret_cast<const char*>(data.mv_data), data.mv_size);
    dbs->push_back(skey);
  }

  mdb_cursor_close(cursor);
  mdb_txn_abort(txn_dbs);
  mdb_dbi_close(connection_.handle_->env, ldbi);
  return common::Error();
}

common::Error DBConnection::CheckResultCommand(const std::string& cmd, int err) {
  if (err != LMDB_OK) {
    return GenerateError(cmd, mdb_strerror(err));
  }

  return common::Error();
}

}  // namespace lmdb
}  // namespace core
}  // namespace fastonosql
