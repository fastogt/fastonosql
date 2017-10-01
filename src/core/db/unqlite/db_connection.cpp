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

#include "core/db/unqlite/db_connection.h"

extern "C" {
#include <unqlite.h>
}

#include <common/file_system/string_path_utils.h>

#include "core/db/unqlite/command_translator.h"
#include "core/db/unqlite/database_info.h"
#include "core/db/unqlite/internal/commands_api.h"

namespace {

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
template <>
const char* ConnectionTraits<UNQLITE>::GetBasedOn() {
  return "unqlite";
}

template <>
const char* ConnectionTraits<UNQLITE>::GetVersionApi() {
  return UNQLITE_VERSION;
}
namespace internal {
template <>
common::Error ConnectionAllocatorTraits<unqlite::NativeConnection, unqlite::Config>::Connect(
    const unqlite::Config& config,
    unqlite::NativeConnection** hout) {
  unqlite::NativeConnection* context = NULL;
  common::Error err = unqlite::CreateConnection(config, &context);
  if (err) {
    return err;
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

template <>
const ConstantCommandsArray& CDBConnection<unqlite::NativeConnection, unqlite::Config, UNQLITE>::GetCommands() {
  return unqlite::g_commands;
}
}  // namespace internal
namespace unqlite {

common::Error CreateConnection(const Config& config, NativeConnection** context) {
  if (!context) {
    return common::make_error_inval();
  }

  DCHECK(*context == NULL);
  struct unqlite* lcontext = NULL;
  std::string db_path = config.db_path;  // start point must be folder
  std::string folder = common::file_system::get_dir_path(db_path);
  common::tribool is_dir = common::file_system::is_directory(folder);
  if (is_dir == common::INDETERMINATE) {
    return common::make_error(common::MemSPrintf("Invalid input path(%s)", folder));
  }

  if (is_dir != common::SUCCESS) {  // if not dir
    return common::make_error(common::MemSPrintf("Invalid input path(%s)", db_path));
  }

  const char* dbname = db_path.c_str();
  int env_flags = config.env_flags;
  int st = unqlite_open(&lcontext, dbname, env_flags);
  if (st != UNQLITE_OK) {
    std::string buff = common::MemSPrintf("Fail open database: %s!", unqlite_strerror(st));
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

  unqlite_close(ldb);
  return common::Error();
}

DBConnection::DBConnection(CDBConnectionClient* client)
    : base_class(client, new CommandTranslator(base_class::GetCommands())) {}

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
  const string_key_t key_slice = key.ToBytes();
  int rc = unqlite_kv_store(connection_.handle_, key_slice.data(), key_slice.size(), value.c_str(), value.length());
  if (rc != UNQLITE_OK) {
    std::string buff = common::MemSPrintf("set function error: %s", unqlite_strerror(rc));
    return common::make_error(buff);
  }

  return common::Error();
}

common::Error DBConnection::DelInner(key_t key) {
  const string_key_t key_slice = key.ToBytes();
  int rc = unqlite_kv_delete(connection_.handle_, key_slice.data(), key_slice.size());
  if (rc != UNQLITE_OK) {
    std::string buff = common::MemSPrintf("delete function error: %s", unqlite_strerror(rc));
    return common::make_error(buff);
  }

  return common::Error();
}

common::Error DBConnection::GetInner(key_t key, std::string* ret_val) {
  const string_key_t key_slice = key.ToBytes();
  int rc = unqlite_kv_fetch_callback(connection_.handle_, key_slice.data(), key_slice.size(), unqlite_data_callback,
                                     ret_val);
  if (rc != UNQLITE_OK) {
    std::string buff = common::MemSPrintf("get function error: %s", unqlite_strerror(rc));
    return common::make_error(buff);
  }

  return common::Error();
}

common::Error DBConnection::ScanImpl(uint64_t cursor_in,
                                     const std::string& pattern,
                                     uint64_t count_keys,
                                     std::vector<std::string>* keys_out,
                                     uint64_t* cursor_out) {
  unqlite_kv_cursor* pCur; /* Cursor handle */
  int rc = unqlite_kv_cursor_init(connection_.handle_, &pCur);
  if (rc != UNQLITE_OK) {
    std::string buff = common::MemSPrintf("Keys function error: %s", unqlite_strerror(rc));
    return common::make_error(buff);
  }
  /* Point to the first record */
  unqlite_kv_cursor_first_entry(pCur);

  /* Iterate over the entries */
  uint64_t offset_pos = cursor_in;
  uint64_t lcursor_out = 0;
  std::vector<std::string> lkeys_out;
  while (unqlite_kv_cursor_valid_entry(pCur)) {
    if (lkeys_out.size() < count_keys) {
      std::string skey;
      unqlite_kv_cursor_key_callback(pCur, unqlite_data_callback, &skey);
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

    /* Point to the next entry */
    unqlite_kv_cursor_next_entry(pCur);
  }
  /* Finally, Release our cursor */
  unqlite_kv_cursor_release(connection_.handle_, pCur);

  *keys_out = lkeys_out;
  *cursor_out = lcursor_out;
  return common::Error();
}

common::Error DBConnection::KeysImpl(const std::string& key_start,
                                     const std::string& key_end,
                                     uint64_t limit,
                                     std::vector<std::string>* ret) { /* Allocate a new cursor instance */
  unqlite_kv_cursor* pCur;                                            /* Cursor handle */
  int rc = unqlite_kv_cursor_init(connection_.handle_, &pCur);
  if (rc != UNQLITE_OK) {
    std::string buff = common::MemSPrintf("Keys function error: %s", unqlite_strerror(rc));
    return common::make_error(buff);
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

common::Error DBConnection::DBkcountImpl(size_t* size) {
  /* Allocate a new cursor instance */
  unqlite_kv_cursor* pCur; /* Cursor handle */
  int rc = unqlite_kv_cursor_init(connection_.handle_, &pCur);
  if (rc != UNQLITE_OK) {
    std::string buff = common::MemSPrintf("DBKCOUNT function error: %s", unqlite_strerror(rc));
    return common::make_error(buff);
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

common::Error DBConnection::FlushDBImpl() {
  unqlite_kv_cursor* pCur; /* Cursor handle */
  int rc = unqlite_kv_cursor_init(connection_.handle_, &pCur);
  if (rc != UNQLITE_OK) {
    std::string buff = common::MemSPrintf("FlushDB function error: %s", unqlite_strerror(rc));
    return common::make_error(buff);
  }
  /* Point to the first record */
  unqlite_kv_cursor_first_entry(pCur);

  /* Iterate over the entries */
  while (unqlite_kv_cursor_valid_entry(pCur)) {
    std::string key;
    unqlite_kv_cursor_key_callback(pCur, unqlite_data_callback, &key);
    common::Error err = DelInner(key_t(key));
    if (err) {
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
  if (name != GetCurrentDBName()) {
    return ICommandTranslator::InvalidInputArguments(DB_SELECTDB_COMMAND);
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

common::Error DBConnection::SetTTLImpl(const NKey& key, ttl_t ttl) {
  UNUSED(key);
  UNUSED(ttl);
  return common::make_error("Sorry, but now " PROJECT_NAME_TITLE " for UnqLite not supported TTL commands.");
}

common::Error DBConnection::GetTTLImpl(const NKey& key, ttl_t* ttl) {
  UNUSED(key);
  UNUSED(ttl);
  return common::make_error("Sorry, but now " PROJECT_NAME_TITLE " for Unqlite not supported TTL commands.");
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

common::Error DBConnection::QuitImpl() {
  common::Error err = Disconnect();
  if (err) {
    return err;
  }

  return common::Error();
}

}  // namespace unqlite
}  // namespace core
}  // namespace fastonosql
