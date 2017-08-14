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

#include <string.h>  // for strtok

#include <memory>  // for __shared_ptr
#include <string>  // for string, operator<, etc
#include <vector>  // for vector

#include <rocksdb/db.h>

#include <common/convert2string.h>  // for ConvertFromString
#include <common/file_system.h>     // for is_directory
#include <common/sprintf.h>         // for MemSPrintf
#include <common/string_util.h>     // for MatchPattern
#include <common/types.h>           // for tribool, tribool::SUCCESS
#include <common/value.h>           // for Value::ErrorsType::E_ERROR, etc

#include "core/command_holder.h"       // for CommandHolder
#include "core/internal/connection.h"  // for Connection<>::handle_t, etc
#include "core/internal/db_connection.h"

#include "core/db/rocksdb/command_translator.h"
#include "core/db/rocksdb/config.h"  // for Config
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

template <>
const char* ConnectionTraits<ROCKSDB>::BasedOn() {
  return "librocksdb";
}

template <>
const char* ConnectionTraits<ROCKSDB>::VersionApi() {
  return STRINGIZE(ROCKSDB_MAJOR) "." STRINGIZE(ROCKSDB_MINOR) "." STRINGIZE(ROCKSDB_PATCH);
}

namespace internal {
template <>
common::Error ConnectionAllocatorTraits<rocksdb::NativeConnection, rocksdb::Config>::Connect(
    const rocksdb::Config& config,
    rocksdb::NativeConnection** hout) {
  rocksdb::NativeConnection* context = nullptr;
  common::Error er = rocksdb::CreateConnection(config, &context);
  if (er && er->IsError()) {
    return er;
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
ConstantCommandsArray CDBConnection<rocksdb::NativeConnection, rocksdb::Config, ROCKSDB>::Commands() {
  return rocksdb::g_commands;
}
}  // namespace internal
namespace rocksdb {

common::Error CreateConnection(const Config& config, NativeConnection** context) {
  if (!context) {
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  DCHECK(*context == nullptr);
  ::rocksdb::DB* lcontext = nullptr;
  std::string folder = config.db_path;  // start point must be folder
  common::tribool is_dir = common::file_system::is_directory(folder);
  if (is_dir != common::SUCCESS) {
    return common::make_error_value(common::MemSPrintf("Invalid input path(%s)", folder), common::ErrorValue::E_ERROR);
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
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  *context = lcontext;
  return common::Error();
}

common::Error TestConnection(const Config& config) {
  ::rocksdb::DB* ldb = nullptr;
  common::Error er = CreateConnection(config, &ldb);
  if (er && er->IsError()) {
    return er;
  }

  delete ldb;
  return common::Error();
}

DBConnection::DBConnection(CDBConnectionClient* client)
    : base_class(client, new CommandTranslator(base_class::Commands())) {}

common::Error DBConnection::Info(const std::string& args, ServerInfo::Stats* statsout) {
  UNUSED(args);
  if (!statsout) {
    DNOTREACHED();
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  std::string rets;
  bool isok = connection_.handle_->GetProperty("rocksdb.stats", &rets);
  if (!isok) {
    return common::make_error_value("info function failed", common::ErrorValue::E_ERROR);
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

std::string DBConnection::CurrentDBName() const {
  ::rocksdb::ColumnFamilyHandle* fam = connection_.handle_->DefaultColumnFamily();
  if (fam) {
    return fam->GetName();
  }

  return base_class::CurrentDBName();
}

common::Error DBConnection::GetInner(key_t key, std::string* ret_val) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  ::rocksdb::ReadOptions ro;
  const string_key_t key_str = key.ToBytes();
  const ::rocksdb::Slice key_slice(reinterpret_cast<const char*>(key_str.data()), key_str.size());
  auto st = connection_.handle_->Get(ro, key_slice, ret_val);
  if (!st.ok()) {
    std::string buff = common::MemSPrintf("get function error: %s", st.ToString());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error DBConnection::Mget(const std::vector<std::string>& keys, std::vector<std::string>* ret) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  std::vector< ::rocksdb::Slice> rslice;
  for (auto key : keys) {
    rslice.push_back(key);
  }
  ::rocksdb::ReadOptions ro;
  auto sts = connection_.handle_->MultiGet(ro, rslice, ret);
  for (size_t i = 0; i < sts.size(); ++i) {
    auto st = sts[i];
    if (st.ok()) {
      return common::Error();
    }
  }

  return common::make_error_value("mget function unknown error", common::ErrorValue::E_ERROR);
}

common::Error DBConnection::Merge(const std::string& key, const std::string& value) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  ::rocksdb::WriteOptions wo;
  auto st = connection_.handle_->Merge(wo, key, value);
  if (!st.ok()) {
    std::string buff = common::MemSPrintf("merge function error: %s", st.ToString());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error DBConnection::SetInner(key_t key, const std::string& value) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  ::rocksdb::WriteOptions wo;
  const string_key_t key_str = key.ToBytes();
  const ::rocksdb::Slice key_slice(reinterpret_cast<const char*>(key_str.data()), key_str.size());
  auto st = connection_.handle_->Put(wo, key_slice, value);
  if (!st.ok()) {
    std::string buff = common::MemSPrintf("set function error: %s", st.ToString());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

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

  ::rocksdb::WriteOptions wo;
  const string_key_t key_str = key.ToBytes();
  const ::rocksdb::Slice key_slice(reinterpret_cast<const char*>(key_str.data()), key_str.size());
  auto st = connection_.handle_->Delete(wo, key_slice);
  if (!st.ok()) {
    std::string buff = common::MemSPrintf("del function error: %s", st.ToString());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
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

  if (!st.ok()) {
    std::string buff = common::MemSPrintf("Keys function error: %s", st.ToString());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
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

  if (!st.ok()) {
    std::string buff = common::MemSPrintf("Keys function error: %s", st.ToString());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
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

  if (!st.ok()) {
    std::string buff = common::MemSPrintf("Couldn't determine DBKCOUNT error: %s", st.ToString());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
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
    auto st = connection_.handle_->Delete(wo, key);
    if (!st.ok()) {
      delete it;
      std::string buff = common::MemSPrintf("del function error: %s", st.ToString());
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }
  }

  auto st = it->status();
  delete it;

  if (!st.ok()) {
    std::string buff = common::MemSPrintf("Keys function error: %s", st.ToString());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
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
  return common::make_error_value("Sorry, but now " PROJECT_NAME_TITLE " for RocksDB not supported TTL commands.",
                                  common::ErrorValue::E_ERROR);
}

common::Error DBConnection::GetTTLImpl(const NKey& key, ttl_t* ttl) {
  UNUSED(key);
  UNUSED(ttl);
  return common::make_error_value("Sorry, but now " PROJECT_NAME_TITLE " for RocksDB not supported TTL commands.",
                                  common::ErrorValue::E_ERROR);
}

common::Error DBConnection::QuitImpl() {
  common::Error err = Disconnect();
  if (err && err->IsError()) {
    return err;
  }

  return common::Error();
}

}  // namespace rocksdb
}  // namespace core
}  // namespace fastonosql
