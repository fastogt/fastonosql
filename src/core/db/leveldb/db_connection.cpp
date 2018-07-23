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

#include "core/db/leveldb/db_connection.h"

#include <leveldb/c.h>  // for leveldb_major_version, etc
#include <leveldb/db.h>

#include <common/convert2string.h>
#include <common/file_system/string_path_utils.h>

#include "core/db/leveldb/command_translator.h"
#include "core/db/leveldb/comparators/indexed_db.h"
#include "core/db/leveldb/database_info.h"
#include "core/db/leveldb/internal/commands_api.h"

#define LEVELDB_HEADER_STATS                             \
  "                               Compactions\n"         \
  "Level  Files Size(MB) Time(sec) Read(MB) Write(MB)\n" \
  "--------------------------------------------------\n"

namespace fastonosql {
namespace core {
namespace leveldb {
namespace {
const ConstantCommandsArray g_commands = {CommandHolder(DB_HELP_COMMAND,
                                                        "[command]",
                                                        "Return how to use command",
                                                        UNDEFINED_SINCE,
                                                        DB_HELP_COMMAND " " DB_GET_KEY_COMMAND,
                                                        0,
                                                        1,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Help),
                                          CommandHolder(DB_INFO_COMMAND,
                                                        "[section]",
                                                        "These command return database information.",
                                                        UNDEFINED_SINCE,
                                                        DB_INFO_COMMAND " STATS",
                                                        0,
                                                        1,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Info),
                                          CommandHolder(DB_GET_CONFIG_COMMAND,
                                                        "<parameter>",
                                                        "Get the value of a configuration parameter",
                                                        UNDEFINED_SINCE,
                                                        DB_GET_CONFIG_COMMAND " databases",
                                                        1,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::ConfigGet),
                                          CommandHolder(DB_SCAN_COMMAND,
                                                        "<cursor> [MATCH pattern] [COUNT count]",
                                                        "Incrementally iterate the keys space",
                                                        UNDEFINED_SINCE,
                                                        DB_SCAN_COMMAND " 0 MATCH * COUNT 10",
                                                        1,
                                                        4,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Scan),
                                          CommandHolder(DB_JSONDUMP_COMMAND,
                                                        "<cursor> PATH absolute_path [MATCH pattern] [COUNT count]",
                                                        "Dump DB into json file by path.",
                                                        UNDEFINED_SINCE,
                                                        DB_JSONDUMP_COMMAND " PATH ~/dump.json MATCH * COUNT 10",
                                                        3,
                                                        4,
                                                        CommandInfo::Native,
                                                        &CommandsApi::JsonDump),
                                          CommandHolder(DB_KEYS_COMMAND,
                                                        "<key_start> <key_end> <limit>",
                                                        "Find all keys matching the given limits.",
                                                        UNDEFINED_SINCE,
                                                        DB_KEYS_COMMAND " a z 10",
                                                        3,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Keys),
                                          CommandHolder(DB_DBKCOUNT_COMMAND,
                                                        "-",
                                                        "Return the number of keys in the "
                                                        "selected database",
                                                        UNDEFINED_SINCE,
                                                        DB_DBKCOUNT_COMMAND,
                                                        0,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::DBkcount),
                                          CommandHolder(DB_FLUSHDB_COMMAND,
                                                        "-",
                                                        "Remove all keys from the current database",
                                                        UNDEFINED_SINCE,
                                                        DB_FLUSHDB_COMMAND,
                                                        0,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::FlushDB),
                                          CommandHolder(DB_SELECTDB_COMMAND,
                                                        "<name>",
                                                        "Change the selected database for the "
                                                        "current connection",
                                                        UNDEFINED_SINCE,
                                                        DB_SELECTDB_COMMAND " 0",
                                                        1,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Select),
                                          CommandHolder(DB_SET_KEY_COMMAND,
                                                        "<key> <value>",
                                                        "Set the value of a key.",
                                                        UNDEFINED_SINCE,
                                                        DB_SET_KEY_COMMAND " key value",
                                                        2,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Set),
                                          CommandHolder(DB_GET_KEY_COMMAND,
                                                        "<key>",
                                                        "Get the value of a key.",
                                                        UNDEFINED_SINCE,
                                                        DB_GET_KEY_COMMAND " key",
                                                        1,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Get),
                                          CommandHolder(DB_RENAME_KEY_COMMAND,
                                                        "<key> <newkey>",
                                                        "Rename a key",
                                                        UNDEFINED_SINCE,
                                                        DB_RENAME_KEY_COMMAND " old_name new_name",
                                                        2,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Rename),
                                          CommandHolder(DB_DELETE_KEY_COMMAND,
                                                        "<key> [key ...]",
                                                        "Delete key.",
                                                        UNDEFINED_SINCE,
                                                        DB_DELETE_KEY_COMMAND " key",
                                                        1,
                                                        INFINITE_COMMAND_ARGS,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Delete),
                                          CommandHolder(DB_QUIT_COMMAND,
                                                        "-",
                                                        "Close the connection",
                                                        UNDEFINED_SINCE,
                                                        DB_QUIT_COMMAND,
                                                        0,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Quit)};
}
}  // namespace leveldb
template <>
const char* ConnectionTraits<LEVELDB>::GetBasedOn() {
  return "leveldb";
}

template <>
const char* ConnectionTraits<LEVELDB>::GetVersionApi() {
  static std::string leveldb_version = common::MemSPrintf("%d.%d", leveldb_major_version(), leveldb_minor_version());
  return leveldb_version.c_str();
}

template <>
const ConstantCommandsArray& ConnectionCommandsTraits<LEVELDB>::GetCommands() {
  return leveldb::g_commands;
}
namespace internal {
template <>
common::Error ConnectionAllocatorTraits<leveldb::NativeConnection, leveldb::Config>::Connect(
    const leveldb::Config& config,
    leveldb::NativeConnection** hout) {
  leveldb::NativeConnection* context = nullptr;
  common::Error err = leveldb::CreateConnection(config, &context);
  if (err) {
    return err;
  }

  *hout = context;
  return common::Error();
}

template <>
common::Error ConnectionAllocatorTraits<leveldb::NativeConnection, leveldb::Config>::Disconnect(
    leveldb::NativeConnection** handle) {
  destroy(handle);
  return common::Error();
}

template <>
bool ConnectionAllocatorTraits<leveldb::NativeConnection, leveldb::Config>::IsConnected(
    leveldb::NativeConnection* handle) {
  if (!handle) {
    return false;
  }

  return true;
}
}  // namespace internal
namespace leveldb {

common::Error CreateConnection(const Config& config, NativeConnection** context) {
  if (!context) {
    return common::make_error_inval();
  }

  DCHECK(*context == nullptr);
  ::leveldb::DB* lcontext = nullptr;
  std::string folder = config.db_path;  // start point must be folder
  common::tribool is_dir = common::file_system::is_directory(folder);
  if (is_dir != common::SUCCESS && !config.create_if_missing) {
    return common::make_error(common::MemSPrintf("Invalid input path(%s)", folder));
  }

  ::leveldb::Options lv;
  lv.create_if_missing = config.create_if_missing;
  if (config.comparator == COMP_BYTEWISE) {
    lv.comparator = ::leveldb::BytewiseComparator();
  } else if (config.comparator == COMP_INDEXED_DB) {
    lv.comparator = new comparator::IndexedDB;
  }
  if (config.compression == kNoCompression) {
    lv.compression = ::leveldb::kNoCompression;
  } else if (config.compression == kSnappyCompression) {
    lv.compression = ::leveldb::kSnappyCompression;
  }

  auto st = ::leveldb::DB::Open(lv, folder, &lcontext);
  if (!st.ok()) {
    std::string buff = common::MemSPrintf("Fail connect to server: %s!", st.ToString());
    return common::make_error(buff);
  }

  *context = lcontext;
  return common::Error();
}

common::Error TestConnection(const Config& config) {
  leveldb::NativeConnection* ldb = nullptr;
  common::Error err = CreateConnection(config, &ldb);
  if (err) {
    return err;
  }

  delete ldb;
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

  std::string rets;
  bool isok = connection_.handle_->GetProperty("leveldb.stats", &rets);
  if (!isok) {
    return common::make_error("info function failed");
  }

  ServerInfo::Stats lstats;
  if (rets.size() > sizeof(LEVELDB_HEADER_STATS)) {
    const char* retsc = rets.c_str() + sizeof(LEVELDB_HEADER_STATS);
    char* p2 = strtok(const_cast<char*>(retsc), " ");
    int pos = 0;
    while (p2) {
      switch (pos++) {
        case 0: {
          uint32_t lv;
          if (common::ConvertFromString(p2, &lv)) {
            lstats.compactions_level = lv;
          }
          break;
        }
        case 1: {
          uint32_t file_size_mb;
          if (common::ConvertFromString(p2, &file_size_mb)) {
            lstats.file_size_mb = file_size_mb;
          }
          break;
        }
        case 2: {
          uint32_t time_sec;
          if (common::ConvertFromString(p2, &time_sec)) {
            lstats.time_sec = time_sec;
          }
          break;
        }
        case 3: {
          uint32_t read_mb;
          if (common::ConvertFromString(p2, &read_mb)) {
            lstats.read_mb = read_mb;
          }
          break;
        }
        case 4: {
          uint32_t write_mb;
          if (common::ConvertFromString(p2, &write_mb)) {
            lstats.read_mb = write_mb;
          }
          break;
        }
        default:
          break;
      }
      p2 = strtok(0, " ");
    }
  }

  *statsout = lstats;
  return common::Error();
}

common::Error DBConnection::DelInner(const key_t& key) {
  std::string exist_key;
  common::Error err = GetInner(key, &exist_key);
  if (err) {
    return err;
  }

  const readable_string_t key_str = key.GetData();
  const ::leveldb::Slice key_slice(reinterpret_cast<const char*>(key_str.data()), key_str.size());
  ::leveldb::WriteOptions wo;
  return CheckResultCommand(DB_DELETE_KEY_COMMAND, connection_.handle_->Delete(wo, key_slice));
}

common::Error DBConnection::SetInner(const key_t& key, const value_t& value) {
  const readable_string_t key_str = key.GetData();
  const readable_string_t value_str = value.GetData();
  const ::leveldb::Slice key_slice(reinterpret_cast<const char*>(key_str.data()), key_str.size());
  ::leveldb::WriteOptions wo;
  return CheckResultCommand(DB_SET_KEY_COMMAND, connection_.handle_->Put(wo, key_slice, value_str));
}

common::Error DBConnection::GetInner(const key_t& key, std::string* ret_val) {
  const readable_string_t key_str = key.GetData();
  const ::leveldb::Slice key_slice(key_str.data(), key_str.size());
  ::leveldb::ReadOptions ro;
  return CheckResultCommand(DB_GET_KEY_COMMAND, connection_.handle_->Get(ro, key_slice, ret_val));
}

common::Error DBConnection::ScanImpl(cursor_t cursor_in,
                                     const std::string& pattern,
                                     keys_limit_t count_keys,
                                     std::vector<std::string>* keys_out,
                                     cursor_t* cursor_out) {
  ::leveldb::ReadOptions ro;
  ::leveldb::Iterator* it = connection_.handle_->NewIterator(ro);
  uint64_t offset_pos = cursor_in;
  uint64_t lcursor_out = 0;
  std::vector<std::string> lkeys_out;
  for (it->SeekToFirst(); it->Valid(); it->Next()) {
    std::string key = it->key().ToString();
    if (lkeys_out.size() < count_keys) {
      if (common::MatchPattern(key, pattern)) {
        if (offset_pos == 0) {
          lkeys_out.push_back(key);
        } else {
          offset_pos--;
        }
      }
    } else {
      lcursor_out = cursor_in + count_keys;
      break;
    }
  }

  auto st = it->status();
  delete it;

  common::Error err = CheckResultCommand(DB_SCAN_COMMAND, st);
  if (err) {
    return err;
  }

  *keys_out = lkeys_out;
  *cursor_out = lcursor_out;
  return common::Error();
}

common::Error DBConnection::KeysImpl(const std::string& key_start,
                                     const std::string& key_end,
                                     keys_limit_t limit,
                                     std::vector<std::string>* ret) {
  std::string skey_start = common::ConvertToString(key_start);
  std::string skey_end = common::ConvertToString(key_end);

  ::leveldb::ReadOptions ro;
  ::leveldb::Iterator* it = connection_.handle_->NewIterator(ro);  // keys(key_start, key_end, limit, ret);
  for (it->Seek(skey_start); it->Valid(); it->Next()) {
    std::string key = it->key().ToString();
    if (ret->size() < limit) {
      if (key < skey_end) {
        ret->push_back(key);
      }
    } else {
      break;
    }
  }

  auto st = it->status();
  delete it;

  return CheckResultCommand(DB_KEYS_COMMAND, st);
}

common::Error DBConnection::DBkcountImpl(size_t* size) {
  ::leveldb::ReadOptions ro;
  ::leveldb::Iterator* it = connection_.handle_->NewIterator(ro);
  size_t sz = 0;
  for (it->SeekToFirst(); it->Valid(); it->Next()) {
    sz++;
  }

  auto st = it->status();
  delete it;

  common::Error err = CheckResultCommand(DB_DBKCOUNT_COMMAND, st);
  if (err) {
    return err;
  }

  *size = sz;
  return common::Error();
}

common::Error DBConnection::FlushDBImpl() {
  ::leveldb::ReadOptions ro;
  ::leveldb::WriteOptions wo;
  ::leveldb::Iterator* it = connection_.handle_->NewIterator(ro);
  for (it->SeekToFirst(); it->Valid(); it->Next()) {
    std::string key = it->key().ToString();
    common::Error err = CheckResultCommand(DB_FLUSHDB_COMMAND, connection_.handle_->Delete(wo, key));
    if (err) {
      delete it;
      return err;
    }
  }

  auto st = it->status();
  delete it;

  return CheckResultCommand(DB_FLUSHDB_COMMAND, st);
}

common::Error DBConnection::SelectImpl(const std::string& name, IDataBaseInfo** info) {
  if (name != GetCurrentDBName()) {
    return ICommandTranslator::InvalidInputArguments(DB_SELECTDB_COMMAND);
  }

  size_t kcount = 0;
  common::Error err = DBkcount(&kcount);
  DCHECK(!err) << err->GetDescription();
  *info = new DataBaseInfo(name, true, kcount);
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

common::Error DBConnection::SetImpl(const NDbKValue& key, NDbKValue* added_key) {
  NKey cur = key.GetKey();
  key_t key_str = cur.GetKey();
  NValue value = key.GetValue();
  value_t value_str = value.GetValue();
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

common::Error DBConnection::RenameImpl(const NKey& key, const key_t& new_key) {
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

  err = SetInner(new_key, value_str);
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
  std::vector<std::string> ldbs = {GetCurrentDBName()};
  *dbs = ldbs;
  return common::Error();
}

common::Error DBConnection::CheckResultCommand(const std::string& cmd, const ::leveldb::Status& err) {
  if (!err.ok()) {
    return GenerateError(cmd, err.ToString());
  }

  return common::Error();
}

}  // namespace leveldb
}  // namespace core
}  // namespace fastonosql
