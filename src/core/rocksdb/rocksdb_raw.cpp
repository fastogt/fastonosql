/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

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

#include "core/rocksdb/rocksdb_raw.h"

#include <vector>
#include <string>

#include "common/sprintf.h"

#define ROCKSDB_HEADER_STATS    "\n** Compaction Stats [default] **\n"\
                                "Level    Files   Size(MB) Score Read(GB)  Rn(GB) Rnp1(GB) "\
                                "Write(GB) Wnew(GB) Moved(GB) W-Amp Rd(MB/s) Wr(MB/s) "\
                                "Comp(sec) Comp(cnt) Avg(sec) "\
                                "Stall(cnt)  KeyIn KeyDrop\n"\
                                "--------------------------------------------------------------------"\
                                "-----------------------------------------------------------"\
                                "--------------------------------------\n"

namespace fastonosql {
namespace core {
template<>
common::Error DBAllocatorTraits<rocksdb::RocksDBConnection, rocksdb::RocksdbConfig>::connect(const rocksdb::RocksdbConfig& config, rocksdb::RocksDBConnection** hout) {
  rocksdb::RocksDBConnection* context = nullptr;
  common::Error er = rocksdb::createConnection(config, &context);
  if (er && er->isError()) {
    return er;
  }

  *hout = context;
  return common::Error();
}
template<>
common::Error DBAllocatorTraits<rocksdb::RocksDBConnection, rocksdb::RocksdbConfig>::disconnect(rocksdb::RocksDBConnection** handle) {
  destroy(handle);
  return common::Error();
}
template<>
bool DBAllocatorTraits<rocksdb::RocksDBConnection, rocksdb::RocksdbConfig>::isConnected(rocksdb::RocksDBConnection* handle) {
  if (!handle) {
    return false;
  }

  return true;
}
namespace rocksdb {

common::Error createConnection(const RocksdbConfig& config, RocksDBConnection** context) {
  if (!context) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  DCHECK(*context == nullptr);
  ::rocksdb::DB* lcontext = nullptr;
  auto st = ::rocksdb::DB::Open(config.options, config.dbname, &lcontext);
  if (!st.ok()) {
    std::string buff = common::MemSPrintf("Fail open database: %s!", st.ToString());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  *context = lcontext;
  return common::Error();
}

common::Error createConnection(RocksdbConnectionSettings* settings, RocksDBConnection** context) {
  if (!settings) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  RocksdbConfig config = settings->info();
  return createConnection(config, context);
}

common::Error testConnection(RocksdbConnectionSettings* settings) {
  if (!settings) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  ::rocksdb::DB* ldb = nullptr;
  common::Error er = createConnection(settings, &ldb);
  if (er && er->isError()) {
    return er;
  }

  delete ldb;
  return common::Error();
}

RocksdbRaw::RocksdbRaw()
  : CommandHandler(rocksdbCommands), connection_() {
}

common::Error RocksdbRaw::connect(const config_t& config) {
  return connection_.connect(config);
}

common::Error RocksdbRaw::disconnect() {
  return connection_.disconnect();
}

bool RocksdbRaw::isConnected() const {
  return connection_.isConnected();
}

std::string RocksdbRaw::delimiter() const {
  return connection_.config_.delimiter;
}

std::string RocksdbRaw::nsSeparator() const {
  return connection_.config_.ns_separator;
}

RocksdbRaw::config_t RocksdbRaw::config() const {
  return connection_.config_;
}

const char* RocksdbRaw::versionApi() {
  return STRINGIZE(ROCKSDB_MAJOR) "." STRINGIZE(ROCKSDB_MINOR) "." STRINGIZE(ROCKSDB_PATCH);
}

common::Error RocksdbRaw::info(const char* args, RocksdbServerInfo::Stats* statsout) {
  CHECK(isConnected());

  if (!statsout) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  std::string rets;
  bool isok = connection_.handle_->GetProperty("rocksdb.stats", &rets);
  if (!isok) {
    return common::make_error_value("info function failed", common::ErrorValue::E_ERROR);
  }

  RocksdbServerInfo::Stats lstatsout;
  if (rets.size() > sizeof(ROCKSDB_HEADER_STATS)) {
    const char* retsc = rets.c_str() + sizeof(ROCKSDB_HEADER_STATS);
    char* p2 = strtok((char*)retsc, " ");
    int pos = 0;
    while (p2) {
      switch (pos++) {
        case 0:
          lstatsout.compactions_level = atoi(p2);
          break;
        case 1:
          lstatsout.file_size_mb = atoi(p2);
          break;
        case 2:
          lstatsout.time_sec = atoi(p2);
          break;
        case 3:
          lstatsout.read_mb = atoi(p2);
          break;
        case 4:
          lstatsout.write_mb = atoi(p2);
          break;
        default:
          break;
        }
        p2 = strtok(0, " ");
      }
  }

  *statsout = lstatsout;
  return common::Error();
}

common::Error RocksdbRaw::dbsize(size_t* size) {
  CHECK(isConnected());

  if (!size) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  ::rocksdb::ReadOptions ro;
  ::rocksdb::Iterator* it = connection_.handle_->NewIterator(ro);
  size_t sz = 0;
  for (it->SeekToFirst(); it->Valid(); it->Next()) {
    sz++;
  }

  auto st = it->status();
  delete it;

  if (!st.ok()) {
    std::string buff = common::MemSPrintf("Couldn't determine DBSIZE error: %s", st.ToString());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  *size = sz;
  return common::Error();
}

std::string RocksdbRaw::currentDbName() const {
  CHECK(isConnected());

  ::rocksdb::ColumnFamilyHandle* fam = connection_.handle_->DefaultColumnFamily();
  if (fam) {
    return fam->GetName();
  }

  return "default";
}

common::Error RocksdbRaw::set(const std::string& key, const std::string& value) {
  CHECK(isConnected());

  ::rocksdb::WriteOptions wo;
  auto st = connection_.handle_->Put(wo, key, value);
  if (!st.ok()) {
    std::string buff = common::MemSPrintf("set function error: %s", st.ToString());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error RocksdbRaw::get(const std::string& key, std::string* ret_val) {
  CHECK(isConnected());

  ::rocksdb::ReadOptions ro;
  auto st = connection_.handle_->Get(ro, key, ret_val);
  if (!st.ok()) {
    std::string buff = common::MemSPrintf("get function error: %s", st.ToString());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error RocksdbRaw::mget(const std::vector< ::rocksdb::Slice>& keys, std::vector<std::string>* ret) {
  CHECK(isConnected());

  ::rocksdb::ReadOptions ro;
  auto sts = connection_.handle_->MultiGet(ro, keys, ret);
  for (size_t i = 0; i < sts.size(); ++i) {
    auto st = sts[i];
    if (st.ok()) {
      return common::Error();
    }
  }

  return common::make_error_value("mget function unknown error", common::ErrorValue::E_ERROR);
}

common::Error RocksdbRaw::merge(const std::string& key, const std::string& value) {
  CHECK(isConnected());

  ::rocksdb::WriteOptions wo;
  auto st = connection_.handle_->Merge(wo, key, value);
  if (!st.ok()) {
    std::string buff = common::MemSPrintf("merge function error: %s", st.ToString());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error RocksdbRaw::del(const std::string& key) {
  CHECK(isConnected());

  ::rocksdb::WriteOptions wo;
  auto st = connection_.handle_->Delete(wo, key);
  if (!st.ok()) {
    std::string buff = common::MemSPrintf("del function error: %s", st.ToString());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error RocksdbRaw::keys(const std::string& key_start, const std::string& key_end,
                   uint64_t limit, std::vector<std::string>* ret) {
  CHECK(isConnected());

  ::rocksdb::ReadOptions ro;
  ::rocksdb::Iterator* it = connection_.handle_->NewIterator(ro);  // keys(key_start, key_end, limit, ret);
  for (it->Seek(key_start); it->Valid() && it->key().ToString() < key_end; it->Next()) {
    std::string key = it->key().ToString();
    if (ret->size() <= limit) {
      ret->push_back(key);
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

common::Error RocksdbRaw::help(int argc, char** argv) {
  return notSupported("HELP");
}

common::Error RocksdbRaw::flushdb() {
  CHECK(isConnected());

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

common::Error info(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  RocksdbRaw* rocks = static_cast<RocksdbRaw*>(handler);
  RocksdbServerInfo::Stats statsout;
  common::Error er = rocks->info(argc == 1 ? argv[0] : nullptr, &statsout);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue(RocksdbServerInfo(statsout).toString());
    FastoObject* child = new FastoObject(out, val, rocks->delimiter(), rocks->nsSeparator());
    out->addChildren(child);
  }

  return er;
}

common::Error dbsize(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  RocksdbRaw* rocks = static_cast<RocksdbRaw*>(handler);
  size_t dbsize = 0;
  common::Error er = rocks->dbsize(&dbsize);
  if (!er) {
    common::FundamentalValue* val = common::Value::createUIntegerValue(dbsize);
    FastoObject* child = new FastoObject(out, val, rocks->delimiter(), rocks->nsSeparator());
    out->addChildren(child);
  }

  return er;
}

common::Error set(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  RocksdbRaw* rocks = static_cast<RocksdbRaw*>(handler);
  common::Error er = rocks->set(argv[0], argv[1]);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("STORED");
    FastoObject* child = new FastoObject(out, val, rocks->delimiter(), rocks->nsSeparator());
    out->addChildren(child);
  }

  return er;
}

common::Error get(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  RocksdbRaw* rocks = static_cast<RocksdbRaw*>(handler);
  std::string ret;
  common::Error er = rocks->get(argv[0], &ret);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue(ret);
    FastoObject* child = new FastoObject(out, val, rocks->delimiter(), rocks->nsSeparator());
    out->addChildren(child);
  }

  return er;
}

common::Error mget(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  RocksdbRaw* rocks = static_cast<RocksdbRaw*>(handler);
  std::vector< ::rocksdb::Slice> keysget;
  for (size_t i = 0; i < argc; ++i) {
    keysget.push_back(argv[i]);
  }

  std::vector<std::string> keysout;
  common::Error er = rocks->mget(keysget, &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, rocks->delimiter(), rocks->nsSeparator());
    out->addChildren(child);
  }

  return er;
}

common::Error merge(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  RocksdbRaw* rocks = static_cast<RocksdbRaw*>(handler);
  common::Error er = rocks->merge(argv[0], argv[1]);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("STORED");
    FastoObject* child = new FastoObject(out, val, rocks->delimiter(), rocks->nsSeparator());
    out->addChildren(child);
  }

  return er;
}

common::Error del(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  RocksdbRaw* rocks = static_cast<RocksdbRaw*>(handler);
  common::Error er = rocks->del(argv[0]);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("DELETED");
    FastoObject* child = new FastoObject(out, val, rocks->delimiter(), rocks->nsSeparator());
    out->addChildren(child);
  }

  return er;
}

common::Error keys(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  RocksdbRaw* rocks = static_cast<RocksdbRaw*>(handler);
  std::vector<std::string> keysout;
  common::Error er = rocks->keys(argv[0], argv[1], atoll(argv[2]), &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, rocks->delimiter(), rocks->nsSeparator());
    out->addChildren(child);
  }

  return er;
}

common::Error help(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  RocksdbRaw* rocks = static_cast<RocksdbRaw*>(handler);
  return rocks->help(argc - 1, argv + 1);
}

common::Error flushdb(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  RocksdbRaw* rocks = static_cast<RocksdbRaw*>(handler);
  return rocks->flushdb();
}

}  // namespace rocksdb
}  // namespace core
}  // namespace fastonosql
