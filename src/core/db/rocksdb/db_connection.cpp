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

#include "core/db/rocksdb/db_connection.h"

#include <common/convert2string.h>
#include <common/file_system/string_path_utils.h>

#include <rocksdb/db.h>

#include "core/db/rocksdb/command_translator.h"
#include "core/db/rocksdb/database_info.h"
#include "core/db/rocksdb/internal/commands_api.h"

#define ROCKSDB_HEADER_STATS                               \
  "\n** Compaction Stats [default] **\n"                   \
  "Level    Files   Size(MB) Score Read(GB)  Rn(GB) "      \
  "Rnp1(GB) "                                              \
  "Write(GB) Wnew(GB) Moved(GB) W-Amp Rd(MB/s) Wr(MB/s) "  \
  "Comp(sec) Comp(cnt) Avg(sec) "                          \
  "Stall(cnt)  KeyIn KeyDrop\n"                            \
  "------------------------------------------------------" \
  "--------------"                                         \
  "------------------------------------------------------" \
  "-----"                                                  \
  "--------------------------------------\n"

namespace fastonosql {
namespace core {

namespace rocksdb {
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
                                          CommandHolder("MGET",
                                                        "<key> [key ...]",
                                                        "Get the value of a key.",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        1,
                                                        INFINITE_COMMAND_ARGS,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Mget),
                                          CommandHolder("MERGE",
                                                        "<key> <value>",
                                                        "Merge the database entry for \"key\" "
                                                        "with \"value\"",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        2,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Merge),
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
}  // namespace rocksdb

template <>
const char* ConnectionTraits<ROCKSDB>::GetBasedOn() {
  return "librocksdb";
}

template <>
const char* ConnectionTraits<ROCKSDB>::GetVersionApi() {
  return STRINGIZE(ROCKSDB_MAJOR) "." STRINGIZE(ROCKSDB_MINOR) "." STRINGIZE(ROCKSDB_PATCH);
}

namespace internal {
template <>
common::Error ConnectionAllocatorTraits<rocksdb::NativeConnection, rocksdb::Config>::Connect(
    const rocksdb::Config& config,
    rocksdb::NativeConnection** hout) {
  rocksdb::NativeConnection* context = nullptr;
  common::Error err = rocksdb::CreateConnection(config, &context);
  if (err) {
    return err;
  }

  *hout = context;
  return common::Error();
}

template <>
common::Error ConnectionAllocatorTraits<rocksdb::NativeConnection, rocksdb::Config>::Disconnect(
    rocksdb::NativeConnection** handle) {
  destroy(handle);
  return common::Error();
}

template <>
bool ConnectionAllocatorTraits<rocksdb::NativeConnection, rocksdb::Config>::IsConnected(
    rocksdb::NativeConnection* handle) {
  if (!handle) {
    return false;
  }

  return true;
}

template <>
const ConstantCommandsArray& CDBConnection<rocksdb::NativeConnection, rocksdb::Config, ROCKSDB>::GetCommands() {
  return rocksdb::g_commands;
}

}  // namespace internal
namespace rocksdb {

common::Error CreateConnection(const Config& config, NativeConnection** context) {
  if (!context) {
    return common::make_error_inval();
  }

  DCHECK(*context == nullptr);
  ::rocksdb::DB* lcontext = nullptr;
  std::string folder = config.db_path;  // start point must be folder
  common::tribool is_dir = common::file_system::is_directory(folder);
  if (is_dir != common::SUCCESS && !config.create_if_missing) {
    return common::make_error(common::MemSPrintf("Invalid input path(%s)", folder));
  }

  ::rocksdb::Options rs;
  rs.create_if_missing = config.create_if_missing;
  if (config.comparator == COMP_BYTEWISE) {
    rs.comparator = ::rocksdb::BytewiseComparator();
  } else if (config.comparator == COMP_REVERSE_BYTEWISE) {
    rs.comparator = ::rocksdb::ReverseBytewiseComparator();
  }
  auto st = ::rocksdb::DB::Open(rs, folder, &lcontext);
  if (!st.ok()) {
    std::string buff = common::MemSPrintf("Fail open database: %s!", st.ToString());
    return common::make_error(buff);
  }

  *context = lcontext;
  return common::Error();
}

common::Error TestConnection(const Config& config) {
  ::rocksdb::DB* ldb = nullptr;
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
  bool isok = connection_.handle_->GetProperty("rocksdb.stats", &rets);
  if (!isok) {
    return common::make_error("info function failed");
  }

  ServerInfo::Stats lstatsout;
  if (rets.size() > sizeof(ROCKSDB_HEADER_STATS)) {
    const char* retsc = rets.c_str() + sizeof(ROCKSDB_HEADER_STATS);
    char* p2 = strtok(const_cast<char*>(retsc), " ");
    int pos = 0;
    while (p2) {
      switch (pos++) {
        case 0: {
          uint32_t compactions_level;
          if (common::ConvertFromString(p2, &compactions_level)) {
            lstatsout.compactions_level = compactions_level;
          }
          break;
        }
        case 1: {
          uint32_t file_size_mb;
          if (common::ConvertFromString(p2, &file_size_mb)) {
            lstatsout.file_size_mb = file_size_mb;
          }
          break;
        }
        case 2: {
          uint32_t time_sec;
          if (common::ConvertFromString(p2, &time_sec)) {
            lstatsout.time_sec = time_sec;
          }
          break;
        }
        case 3: {
          uint32_t read_mb;
          if (common::ConvertFromString(p2, &read_mb)) {
            lstatsout.read_mb = read_mb;
          }
          break;
        }
        case 4: {
          uint32_t write_mb;
          if (common::ConvertFromString(p2, &write_mb)) {
            lstatsout.write_mb = write_mb;
          }
          break;
        }
        default:
          break;
      }
      p2 = strtok(0, " ");
    }
  }

  *statsout = lstatsout;
  return common::Error();
}

std::string DBConnection::GetCurrentDBName() const {
  if (IsConnected()) {
    ::rocksdb::ColumnFamilyHandle* fam = connection_.handle_->DefaultColumnFamily();
    if (fam) {
      return fam->GetName();
    }
  }

  DNOTREACHED();
  return base_class::GetCurrentDBName();
}

common::Error DBConnection::GetInner(key_t key, std::string* ret_val) {
  ::rocksdb::ReadOptions ro;
  const string_key_t key_str = key.GetKeyData();
  const ::rocksdb::Slice key_slice(reinterpret_cast<const char*>(key_str.data()), key_str.size());
  return CheckResultCommand(DB_GET_KEY_COMMAND, connection_.handle_->Get(ro, key_slice, ret_val));
}

common::Error DBConnection::Mget(const std::vector<std::string>& keys, std::vector<std::string>* ret) {
  if (keys.empty() || !ret) {
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  std::vector< ::rocksdb::Slice> rslice;
  for (auto key : keys) {
    rslice.push_back(key);
  }
  ::rocksdb::ReadOptions ro;
  auto sts = connection_.handle_->MultiGet(ro, rslice, ret);
  for (size_t i = 0; i < sts.size(); ++i) {
    common::Error err = CheckResultCommand("MGET", sts[i]);
    if (err) {
      return err;
    }
  }

  return common::Error();
}

common::Error DBConnection::Merge(const std::string& key, const std::string& value) {
  if (key.empty() || value.empty()) {
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  ::rocksdb::WriteOptions wo;
  return CheckResultCommand("MERGE", connection_.handle_->Merge(wo, key, value));
}

common::Error DBConnection::SetInner(key_t key, const std::string& value) {
  ::rocksdb::WriteOptions wo;
  const string_key_t key_str = key.GetKeyData();
  const ::rocksdb::Slice key_slice(reinterpret_cast<const char*>(key_str.data()), key_str.size());
  return CheckResultCommand(DB_SET_KEY_COMMAND, connection_.handle_->Put(wo, key_slice, value));
}

common::Error DBConnection::DelInner(key_t key) {
  std::string exist_key;
  common::Error err = GetInner(key, &exist_key);
  if (err) {
    return err;
  }

  ::rocksdb::WriteOptions wo;
  const string_key_t key_str = key.GetKeyData();
  const ::rocksdb::Slice key_slice(reinterpret_cast<const char*>(key_str.data()), key_str.size());
  return CheckResultCommand(DB_DELETE_KEY_COMMAND, connection_.handle_->Delete(wo, key_slice));
}

common::Error DBConnection::ScanImpl(uint64_t cursor_in,
                                     const std::string& pattern,
                                     uint64_t count_keys,
                                     std::vector<std::string>* keys_out,
                                     uint64_t* cursor_out) {
  ::rocksdb::ReadOptions ro;
  ::rocksdb::Iterator* it = connection_.handle_->NewIterator(ro);  // keys(key_start, key_end, limit, ret);
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
                                     uint64_t limit,
                                     std::vector<std::string>* ret) {
  ::rocksdb::ReadOptions ro;
  ::rocksdb::Iterator* it = connection_.handle_->NewIterator(ro);  // keys(key_start, key_end, limit, ret);
  for (it->Seek(key_start); it->Valid(); it->Next()) {
    std::string key = it->key().ToString();
    if (ret->size() < limit) {
      if (key < key_end) {
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
  ::rocksdb::ReadOptions ro;
  ::rocksdb::Iterator* it = connection_.handle_->NewIterator(ro);
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
  ::rocksdb::ReadOptions ro;
  ::rocksdb::WriteOptions wo;
  ::rocksdb::Iterator* it = connection_.handle_->NewIterator(ro);
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

common::Error DBConnection::CheckResultCommand(const std::string& cmd, const ::rocksdb::Status& err) {
  if (!err.ok()) {
    return GenerateError(cmd, err.ToString());
  }

  return common::Error();
}

}  // namespace rocksdb
}  // namespace core
}  // namespace fastonosql
