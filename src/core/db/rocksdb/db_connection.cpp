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
                                          CommandHolder(DB_CREATEDB_COMMAND,
                                                        "<name>",
                                                        "Create database",
                                                        UNDEFINED_SINCE,
                                                        DB_CREATEDB_COMMAND " test",
                                                        1,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::CreateDatabase),
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
                                                        1,
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
                                                        &CommandsApi::Quit),
                                          CommandHolder(DB_REMOVEDB_COMMAND,
                                                        "<name>",
                                                        "Remove database",
                                                        UNDEFINED_SINCE,
                                                        DB_REMOVEDB_COMMAND " test",
                                                        1,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::RemoveDatabase)};
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

template <>
const ConstantCommandsArray& ConnectionCommandsTraits<ROCKSDB>::GetCommands() {
  return rocksdb::g_commands;
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

}  // namespace internal
namespace rocksdb {
class rocksdb_handle {
 public:
  rocksdb_handle(::rocksdb::DB* db, std::vector<::rocksdb::ColumnFamilyHandle*> handles)
      : db_(db), handles_(handles), current_db_index_(0) {}
  ~rocksdb_handle() {
    for (auto handle : handles_) {
      delete handle;
    }
    handles_.clear();
    delete db_;
  }

  bool GetProperty(const ::rocksdb::Slice& property, std::string* value) {
    return db_->GetProperty(GetCurrentColumn(), property, value);
  }

  ::rocksdb::Status Get(const ::rocksdb::ReadOptions& options, const ::rocksdb::Slice& key, std::string* value) {
    return db_->Get(options, GetCurrentColumn(), key, value);
  }

  std::vector<::rocksdb::Status> MultiGet(const ::rocksdb::ReadOptions& options,
                                          const std::vector<::rocksdb::Slice>& keys,
                                          std::vector<std::string>* values) {
    return db_->MultiGet(options, handles_, keys, values);
  }

  ::rocksdb::Status Merge(const ::rocksdb::WriteOptions& options,
                          const ::rocksdb::Slice& key,
                          const ::rocksdb::Slice& value) {
    return db_->Merge(options, GetCurrentColumn(), key, value);
  }

  ::rocksdb::Status Put(const ::rocksdb::WriteOptions& options,
                        const ::rocksdb::Slice& key,
                        const ::rocksdb::Slice& value) {
    return db_->Put(options, GetCurrentColumn(), key, value);
  }

  ::rocksdb::Status Delete(const ::rocksdb::WriteOptions& options, const ::rocksdb::Slice& key) {
    return db_->Delete(options, GetCurrentColumn(), key);
  }

  ::rocksdb::Iterator* NewIterator(const ::rocksdb::ReadOptions& options) {
    return db_->NewIterator(options, GetCurrentColumn());
  }

  ::rocksdb::ColumnFamilyHandle* GetCurrentColumn() const { return handles_[current_db_index_]; }
  std::string GetCurrentDBName() const {
    ::rocksdb::ColumnFamilyHandle* fam = GetCurrentColumn();
    return fam->GetName();
  }

  ::rocksdb::Status Select(const std::string& name) {
    if (name.empty()) {  // only for named dbs
      return ::rocksdb::Status::InvalidArgument();
    }

    if (name == GetCurrentDBName()) {  // lazy select
      return ::rocksdb::Status();
    }

    for (size_t i = 0; i < handles_.size(); ++i) {
      ::rocksdb::ColumnFamilyHandle* fam = handles_[i];
      if (fam->GetName() == name) {
        current_db_index_ = i;
        return ::rocksdb::Status();
      }
    }

    return ::rocksdb::Status::NotFound();
  }

  ::rocksdb::Status CreateDB(const std::string& name) {
    if (name.empty()) {  // only for named dbs
      return ::rocksdb::Status::InvalidArgument();
    }

    for (size_t i = 0; i < handles_.size(); ++i) {
      ::rocksdb::ColumnFamilyHandle* fam = handles_[i];
      if (fam->GetName() == name) {
        return ::rocksdb::Status();
      }
    }

    ::rocksdb::ColumnFamilyHandle* fam = nullptr;
    ::rocksdb::Status st = db_->CreateColumnFamily(::rocksdb::ColumnFamilyOptions(), name, &fam);
    if (st.ok()) {
      handles_.push_back(fam);
    }
    return st;
  }

  ::rocksdb::Status RemoveDB(const std::string& name) {
    if (name.empty()) {  // only for named dbs
      return ::rocksdb::Status::InvalidArgument();
    }

    if (name == GetCurrentDBName()) {  // don't want to delete selected db
      return ::rocksdb::Status::InvalidArgument();
    }

    for (size_t i = 0; i < handles_.size(); ++i) {
      ::rocksdb::ColumnFamilyHandle* fam = handles_[i];
      if (fam->GetName() == name) {
        ::rocksdb::Status st = db_->DropColumnFamily(fam);
        if (st.ok()) {
          handles_.erase(handles_.begin() + i);
          if (i < current_db_index_) {
            current_db_index_--;
          }
        }
        return st;
      }
    }

    return ::rocksdb::Status::NotFound();
  }

  std::vector<std::string> GetDatabasesNames() const {
    std::vector<std::string> res;
    for (size_t i = 0; i < handles_.size(); ++i) {
      ::rocksdb::ColumnFamilyHandle* fam = handles_[i];
      res.push_back(fam->GetName());
    }
    return res;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(rocksdb_handle);
  ::rocksdb::DB* db_;
  std::vector<::rocksdb::ColumnFamilyHandle*> handles_;
  size_t current_db_index_;
};
common::Error CreateConnection(const Config& config, NativeConnection** context) {
  if (!context) {
    return common::make_error_inval();
  }

  DCHECK(*context == nullptr);
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

  if (config.compression == kNoCompression) {
    rs.compression = ::rocksdb::kNoCompression;
  } else if (config.compression == kSnappyCompression) {
    rs.compression = ::rocksdb::kSnappyCompression;
  } else if (config.compression == kZlibCompression) {
    rs.compression = ::rocksdb::kZlibCompression;
  } else if (config.compression == kBZip2Compression) {
    rs.compression = ::rocksdb::kBZip2Compression;
  } else if (config.compression == kLZ4Compression) {
    rs.compression = ::rocksdb::kLZ4Compression;
  } else if (config.compression == kLZ4HCCompression) {
    rs.compression = ::rocksdb::kLZ4HCCompression;
  } else if (config.compression == kXpressCompression) {
    rs.compression = ::rocksdb::kXpressCompression;
  } else if (config.compression == kZSTD) {
    rs.compression = ::rocksdb::kZSTD;
  }

  std::vector<std::string> column_families_str;
  auto st = ::rocksdb::DB::ListColumnFamilies(rs, folder, &column_families_str);
  if (!st.ok()) {
    if (!rs.create_if_missing) {
      std::string buff = common::MemSPrintf("Fail open database: %s!", st.ToString());
      return common::make_error(buff);
    }
  }

  std::vector<::rocksdb::ColumnFamilyDescriptor> column_families;
  for (size_t i = 0; i < column_families_str.size(); ++i) {
    ::rocksdb::ColumnFamilyDescriptor descr(column_families_str[i], ::rocksdb::ColumnFamilyOptions());
    column_families.push_back(descr);
  }

  if (column_families.empty()) {
    column_families = {::rocksdb::ColumnFamilyDescriptor()};
  }

  ::rocksdb::DB* lcontext = nullptr;
  std::vector<::rocksdb::ColumnFamilyHandle*> lhandles;
  st = ::rocksdb::DB::Open(rs, folder, column_families, &lhandles, &lcontext);
  if (!st.ok()) {
    std::string buff = common::MemSPrintf("Fail open database: %s!", st.ToString());
    return common::make_error(buff);
  }

  *context = new rocksdb_handle(lcontext, lhandles);
  return common::Error();
}

common::Error TestConnection(const Config& config) {
  NativeConnection* ldb = nullptr;
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
    return connection_.handle_->GetCurrentDBName();
  }

  DNOTREACHED();
  return base_class::GetCurrentDBName();
}

common::Error DBConnection::GetInner(const key_t& key, std::string* ret_val) {
  ::rocksdb::ReadOptions ro;
  const readable_string_t key_str = key.GetData();
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

  std::vector<::rocksdb::Slice> rslice;
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

common::Error DBConnection::SetInner(const key_t& key, const value_t& value) {
  ::rocksdb::WriteOptions wo;
  const readable_string_t key_str = key.GetData();
  const readable_string_t value_str = value.GetData();
  const ::rocksdb::Slice key_slice(reinterpret_cast<const char*>(key_str.data()), key_str.size());
  return CheckResultCommand(DB_SET_KEY_COMMAND, connection_.handle_->Put(wo, key_slice, value_str));
}

common::Error DBConnection::DelInner(const key_t& key) {
  std::string exist_key;
  common::Error err = GetInner(key, &exist_key);
  if (err) {
    return err;
  }

  ::rocksdb::WriteOptions wo;
  const readable_string_t key_str = key.GetData();
  const ::rocksdb::Slice key_slice(reinterpret_cast<const char*>(key_str.data()), key_str.size());
  return CheckResultCommand(DB_DELETE_KEY_COMMAND, connection_.handle_->Delete(wo, key_slice));
}

common::Error DBConnection::ScanImpl(cursor_t cursor_in,
                                     const std::string& pattern,
                                     keys_limit_t count_keys,
                                     std::vector<std::string>* keys_out,
                                     cursor_t* cursor_out) {
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
                                     keys_limit_t limit,
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

common::Error DBConnection::CreateDBImpl(const std::string& name, IDataBaseInfo** info) {
  common::Error err = CheckResultCommand(DB_CREATEDB_COMMAND, connection_.handle_->CreateDB(name));
  if (err) {
    return err;
  }

  *info = new DataBaseInfo(name, false, 0);
  return common::Error();
}

common::Error DBConnection::RemoveDBImpl(const std::string& name, IDataBaseInfo** info) {
  common::Error err = CheckResultCommand(DB_REMOVEDB_COMMAND, connection_.handle_->RemoveDB(name));
  if (err) {
    return err;
  }

  *info = new DataBaseInfo(name, false, 0);
  return common::Error();
}

common::Error DBConnection::SelectImpl(const std::string& name, IDataBaseInfo** info) {
  common::Error err = CheckResultCommand(DB_SELECTDB_COMMAND, connection_.handle_->Select(name));
  if (err) {
    return err;
  }

  size_t kcount = 0;
  err = DBkcount(&kcount);
  DCHECK(!err) << err->GetDescription();
  *info = new DataBaseInfo(name, true, kcount);
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
  *dbs = connection_.handle_->GetDatabasesNames();
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
