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

#include "core/db/upscaledb/db_connection.h"

#include <ups/upscaledb.h>

#include <common/file_system/string_path_utils.h>

#include "core/db/upscaledb/command_translator.h"
#include "core/db/upscaledb/database_info.h"
#include "core/db/upscaledb/internal/commands_api.h"

namespace fastonosql {
namespace core {
template <>
const char* ConnectionTraits<UPSCALEDB>::GetBasedOn() {
  return "libupscaledb";
}

template <>
const char* ConnectionTraits<UPSCALEDB>::GetVersionApi() {
  return STRINGIZE(UPS_VERSION_MAJ) "." STRINGIZE(UPS_VERSION_MIN) "." STRINGIZE(UPS_VERSION_REV);
}
namespace {
ups_key_t ConvertToUpscaleDBSlice(const string_key_t& key) {
  ups_key_t dkey;
  memset(&dkey, 0, sizeof(dkey));
  dkey.size = key.size();
  dkey.data = const_cast<command_buffer_char_t*>(key.data());
  return dkey;
}
}  // namespace
namespace upscaledb {

struct upscaledb {
  ups_env_t* env;
  ups_db_t* db;
  uint16_t cur_db;
};

namespace {

int upscaledb_select(upscaledb* context, uint16_t num) {
  if (!context) {
    return EINVAL;
  }

  if (context->cur_db == num) {  // lazy select
    return UPS_SUCCESS;
  }

  ups_db_t* db = NULL;
  ups_status_t st = ups_env_open_db(context->env, &db, num, 0, NULL);
  if (st != UPS_SUCCESS) {
    return st;
  }

  st = ups_db_close(context->db, 0);
  DCHECK(st == UPS_SUCCESS);
  context->db = db;
  context->cur_db = num;
  return UPS_SUCCESS;
}

ups_status_t upscaledb_open(upscaledb** context, const char* dbpath, uint16_t db, bool create_if_missing) {
  upscaledb* lcontext = reinterpret_cast<upscaledb*>(calloc(1, sizeof(upscaledb)));
  bool need_to_create = false;
  if (create_if_missing) {
    bool exist = common::file_system::is_file_exist(std::string(dbpath));
    if (!exist) {
      need_to_create = true;
    }
  }

  ups_status_t st =
      need_to_create ? ups_env_create(&lcontext->env, dbpath, 0, 0664, 0) : ups_env_open(&lcontext->env, dbpath, 0, 0);
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
}  // namespace
}  // namespace upscaledb
namespace internal {
template <>
common::Error ConnectionAllocatorTraits<upscaledb::NativeConnection, upscaledb::Config>::Connect(
    const upscaledb::Config& config,
    upscaledb::NativeConnection** hout) {
  upscaledb::NativeConnection* context = nullptr;
  common::Error err = upscaledb::CreateConnection(config, &context);
  if (err) {
    return err;
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
const ConstantCommandsArray& CDBConnection<upscaledb::NativeConnection, upscaledb::Config, UPSCALEDB>::GetCommands() {
  return upscaledb::g_commands;
}
}  // namespace internal
namespace upscaledb {
common::Error CreateConnection(const Config& config, NativeConnection** context) {
  if (!context) {
    return common::make_error_inval();
  }

  DCHECK(*context == NULL);
  struct upscaledb* lcontext = NULL;
  std::string db_path = config.db_path;  // start point must be folder
  std::string folder = common::file_system::get_dir_path(db_path);
  common::tribool is_dir = common::file_system::is_directory(folder);
  if (is_dir == common::INDETERMINATE) {
    return common::make_error(common::MemSPrintf("Invalid input path(%s)", folder));
  }

  if (is_dir != common::SUCCESS) {
    return common::make_error(common::MemSPrintf("Invalid input path(%s)", db_path));
  }

  const char* dbname = db_path.empty() ? NULL : db_path.c_str();
  int st = upscaledb_open(&lcontext, dbname, config.dbnum, config.create_if_missing);
  if (st != UPS_SUCCESS) {
    std::string buff = common::MemSPrintf("Fail open database: %s", ups_strerror(st));
    return common::make_error(buff);
  }

  *context = lcontext;
  return common::Error();
}

common::Error TestConnection(const Config& config) {
  struct upscaledb* scaledb = NULL;
  common::Error err = CreateConnection(config, &scaledb);
  if (err) {
    return err;
  }

  upscaledb_close(&scaledb);
  return common::Error();
}

DBConnection::DBConnection(CDBConnectionClient* client)
    : base_class(client, new CommandTranslator(base_class::GetCommands())) {}

std::string DBConnection::GetCurrentDBName() const {
  if (connection_.handle_) {
    return common::ConvertToString(connection_.handle_->cur_db);
  }

  DNOTREACHED();
  return base_class::GetCurrentDBName();
}

common::Error DBConnection::Connect(const config_t& config) {
  common::Error err = base_class::Connect(config);
  if (err) {
    return err;
  }

  err = Select(common::ConvertToString(config->dbnum), NULL);
  if (err) {
    return err;
  }

  return common::Error();
}

common::Error DBConnection::Disconnect() {
  connection_.handle_->cur_db = 0;
  return base_class::Disconnect();
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

common::Error DBConnection::SetInner(key_t key, const std::string& value) {
  const string_key_t key_str = key.ToBytes();
  ups_key_t key_slice = ConvertToUpscaleDBSlice(key_str);

  ups_record_t rec;
  memset(&rec, 0, sizeof(rec));
  rec.data = const_cast<char*>(value.c_str());
  rec.size = value.size();
  return CheckResultCommand(DB_SET_KEY_COMMAND,
                            ups_db_insert(connection_.handle_->db, 0, &key_slice, &rec, UPS_OVERWRITE));
}

common::Error DBConnection::GetInner(key_t key, std::string* ret_val) {
  const string_key_t key_str = key.ToBytes();
  ups_key_t key_slice = ConvertToUpscaleDBSlice(key_str);

  ups_record_t rec;
  memset(&rec, 0, sizeof(rec));

  common::Error err =
      CheckResultCommand(DB_GET_KEY_COMMAND, ups_db_find(connection_.handle_->db, NULL, &key_slice, &rec, 0));
  if (err) {
    return err;
  }

  *ret_val = std::string(reinterpret_cast<const char*>(rec.data), rec.size);
  return common::Error();
}

common::Error DBConnection::DelInner(key_t key) {
  const string_key_t key_str = key.ToBytes();
  ups_key_t key_slice = ConvertToUpscaleDBSlice(key_str);
  return CheckResultCommand(DB_DELETE_KEY_COMMAND, ups_db_erase(connection_.handle_->db, 0, &key_slice, 0));
}

common::Error DBConnection::ScanImpl(uint64_t cursor_in,
                                     const std::string& pattern,
                                     uint64_t count_keys,
                                     std::vector<std::string>* keys_out,
                                     uint64_t* cursor_out) {
  ups_cursor_t* cursor; /* upscaledb cursor object */
  ups_key_t key;
  ups_record_t rec;

  memset(&key, 0, sizeof(key));
  memset(&rec, 0, sizeof(rec));

  /* create a new cursor */
  common::Error err = CheckResultCommand("SCAN", ups_cursor_create(&cursor, connection_.handle_->db, 0, 0));
  if (err) {
    return err;
  }

  ups_status_t st = UPS_SUCCESS;
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
      } else if (st != UPS_KEY_NOT_FOUND) {
        ups_cursor_close(cursor);
        std::string buff = common::MemSPrintf("SCAN function error: %s", ups_strerror(st));
        return common::make_error(buff);
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
  common::Error err = CheckResultCommand("KEYS", ups_cursor_create(&cursor, connection_.handle_->db, 0, 0));
  if (err) {
    return err;
  }

  ups_status_t st;
  do {
    st = ups_cursor_move(cursor, &key, &rec, UPS_CURSOR_NEXT | UPS_SKIP_DUPLICATES);
    if (st == UPS_SUCCESS) {
      std::string skey(reinterpret_cast<const char*>(key.data), key.size);
      if (key_start < skey && key_end > skey) {
        ret->push_back(skey);
      }
    } else if (st != UPS_KEY_NOT_FOUND) {
      ups_cursor_close(cursor);
      std::string buff = common::MemSPrintf("KEYS function error: %s", ups_strerror(st));
      return common::make_error(buff);
    }
  } while (st == UPS_SUCCESS && limit > ret->size());

  ups_cursor_close(cursor);
  return common::Error();
}

common::Error DBConnection::DBkcountImpl(size_t* size) {
  uint64_t sz = 0;
  common::Error err =
      CheckResultCommand(DB_DBKCOUNT_COMMAND, ups_db_count(connection_.handle_->db, NULL, UPS_SKIP_DUPLICATES, &sz));
  if (err) {
    return err;
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
  common::Error err = CheckResultCommand(DB_FLUSHDB_COMMAND, ups_cursor_create(&cursor, connection_.handle_->db, 0, 0));
  if (err) {
    return err;
  }

  ups_status_t st;
  do {
    /* fetch the next item, and repeat till we've reached the end
     * of the database */
    st = ups_cursor_move(cursor, &key, &rec, UPS_CURSOR_NEXT);
    if (st == UPS_SUCCESS) {
      ups_db_erase(connection_.handle_->db, 0, &key, 0);
    } else if (st && st != UPS_KEY_NOT_FOUND) {
      ups_cursor_close(cursor);
      std::string buff = common::MemSPrintf("FLUSHDB function error: %s", ups_strerror(st));
      return common::make_error(buff);
    }
  } while (st == UPS_SUCCESS);

  ups_cursor_close(cursor);
  return common::Error();
}

common::Error DBConnection::SelectImpl(const std::string& name, IDataBaseInfo** info) {
  uint16_t num;
  if (!common::ConvertFromString(name, &num)) {
    return common::make_error_inval();
  }

  ups_status_t st = upscaledb_select(connection_.handle_, num);
  if (st != UPS_SUCCESS) {
    std::string buff = common::MemSPrintf("SELECT function error: %s", ups_strerror(st));
    return common::make_error(buff);
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

common::Error DBConnection::CheckResultCommand(const std::string& cmd, ups_status_t err) {
  if (err != UPS_SUCCESS) {
    return GenerateError(cmd, ups_strerror(err));
  }

  return common::Error();
}

}  // namespace upscaledb
}  // namespace core
}  // namespace fastonosql
