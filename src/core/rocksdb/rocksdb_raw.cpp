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
namespace rocksdb {

namespace {

common::Error createConnection(const rocksdb::RocksdbConfig& config, ::rocksdb::DB** context) {
  if (!context) {
    return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
  }

  DCHECK(*context == nullptr);
  ::rocksdb::DB* lcontext = nullptr;
  auto st = ::rocksdb::DB::Open(config.options, config.dbname, &lcontext);
  if (!st.ok()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "Fail open database: %s!", st.ToString());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  *context = lcontext;
  return common::Error();
}

common::Error createConnection(RocksdbConnectionSettings* settings, ::rocksdb::DB** context) {
  if (!settings) {
    return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
  }

  RocksdbConfig config = settings->info();
  return createConnection(config, context);
}

}  // namesapce

common::Error testConnection(RocksdbConnectionSettings* settings) {
  if (!settings) {
    return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
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
  : CommandHandler(rocksdbCommands), rocksdb_(nullptr) {
}

const char* RocksdbRaw::versionApi() {
  return STRINGIZE(ROCKSDB_MAJOR) "." STRINGIZE(ROCKSDB_MINOR) "." STRINGIZE(ROCKSDB_PATCH);
}

bool RocksdbRaw::isConnected() const {
  if (!rocksdb_) {
    return false;
  }

  return true;
}

common::Error RocksdbRaw::connect() {
  if (isConnected()) {
    return common::Error();
  }

  ::rocksdb::DB* context = nullptr;
  common::Error er = createConnection(config_, &context);
  if (er && er->isError()) {
    return er;
  }

  rocksdb_ = context;
  return common::Error();
}

common::Error RocksdbRaw::disconnect() {
  if (!isConnected()) {
    return common::Error();
  }

  destroy(&rocksdb_);
  return common::Error();
}

common::Error RocksdbRaw::info(const char* args, RocksdbServerInfo::Stats* statsout) {
  // sstables
  // stats
  // char prop[1024] = {0};
  // common::SNPrintf(prop, sizeof(prop), "rocksdb.%s", args ? args : "stats");
  if (!statsout) {
    return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
  }

  std::string rets;
  bool isok = rocksdb_->GetProperty("rocksdb.stats", &rets);
  if (!isok) {
    return common::make_error_value("info function failed", common::ErrorValue::E_ERROR);
  }

  RocksdbServerInfo::Stats lstatsout;
  if (rets.size() > sizeof(ROCKSDB_HEADER_STATS)) {
    const char * retsc = rets.c_str() + sizeof(ROCKSDB_HEADER_STATS);
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
  if (!size) {
    return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
  }

  ::rocksdb::ReadOptions ro;
  ::rocksdb::Iterator* it = rocksdb_->NewIterator(ro);
  size_t sz = 0;
  for (it->SeekToFirst(); it->Valid(); it->Next()) {
    sz++;
  }

  auto st = it->status();
  delete it;

  if (!st.ok()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "Couldn't determine DBSIZE error: %s", st.ToString());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  *size = sz;
  return common::Error();
}

RocksdbRaw::~RocksdbRaw() {
  destroy(&rocksdb_);
}

std::string RocksdbRaw::currentDbName() const {
  ::rocksdb::ColumnFamilyHandle* fam = rocksdb_->DefaultColumnFamily();
  if (fam) {
    return fam->GetName();
  }

  return "default";
}

common::Error RocksdbRaw::get(const std::string& key, std::string* ret_val) {
  ::rocksdb::ReadOptions ro;
  auto st = rocksdb_->Get(ro, key, ret_val);
  if (!st.ok()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "get function error: %s", st.ToString());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error RocksdbRaw::mget(const std::vector< ::rocksdb::Slice>& keys, std::vector<std::string> *ret) {
  ::rocksdb::ReadOptions ro;
  auto sts = rocksdb_->MultiGet(ro, keys, ret);
  for (size_t i = 0; i < sts.size(); ++i) {
    auto st = sts[i];
    if (st.ok()) {
      return common::Error();
    }
  }

  return common::make_error_value("mget function unknown error", common::ErrorValue::E_ERROR);
}

common::Error RocksdbRaw::merge(const std::string& key, const std::string& value) {
  ::rocksdb::WriteOptions wo;
  auto st = rocksdb_->Merge(wo, key, value);
  if (!st.ok()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "merge function error: %s", st.ToString());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error RocksdbRaw::put(const std::string& key, const std::string& value) {
  ::rocksdb::WriteOptions wo;
  auto st = rocksdb_->Put(wo, key, value);
  if (!st.ok()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "put function error: %s", st.ToString());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error RocksdbRaw::del(const std::string& key) {
  ::rocksdb::WriteOptions wo;
  auto st = rocksdb_->Delete(wo, key);
  if (!st.ok()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "del function error: %s", st.ToString());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error RocksdbRaw::keys(const std::string& key_start, const std::string& key_end,
                   uint64_t limit, std::vector<std::string> *ret) {
  ret->clear();

  ::rocksdb::ReadOptions ro;
  ::rocksdb::Iterator* it = rocksdb_->NewIterator(ro);  // keys(key_start, key_end, limit, ret);
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
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "Keys function error: %s", st.ToString());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error RocksdbRaw::help(int argc, char** argv) {
  return notSupported("HELP");
}

common::Error info(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  RocksdbRaw* rocks = static_cast<RocksdbRaw*>(handler);
  RocksdbServerInfo::Stats statsout;
  common::Error er = rocks->info(argc == 1 ? argv[0] : nullptr, &statsout);
  if (!er) {
    common::StringValue *val = common::Value::createStringValue(RocksdbServerInfo(statsout).toString());
    FastoObject* child = new FastoObject(out, val, rocks->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error dbsize(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  RocksdbRaw* rocks = static_cast<RocksdbRaw*>(handler);
  size_t dbsize = 0;
  common::Error er = rocks->dbsize(&dbsize);
  if (!er) {
    common::FundamentalValue *val = common::Value::createUIntegerValue(dbsize);
    FastoObject* child = new FastoObject(out, val, rocks->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error get(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  RocksdbRaw* rocks = static_cast<RocksdbRaw*>(handler);
  std::string ret;
  common::Error er = rocks->get(argv[0], &ret);
  if (!er) {
    common::StringValue *val = common::Value::createStringValue(ret);
    FastoObject* child = new FastoObject(out, val, rocks->config_.delimiter);
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
      common::StringValue *val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, rocks->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error merge(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  RocksdbRaw* rocks = static_cast<RocksdbRaw*>(handler);
  common::Error er = rocks->merge(argv[0], argv[1]);
  if (!er) {
    common::StringValue *val = common::Value::createStringValue("STORED");
    FastoObject* child = new FastoObject(out, val, rocks->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error put(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  RocksdbRaw* rocks = static_cast<RocksdbRaw*>(handler);
  common::Error er = rocks->put(argv[0], argv[1]);
  if (!er) {
    common::StringValue *val = common::Value::createStringValue("STORED");
    FastoObject* child = new FastoObject(out, val, rocks->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error del(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  RocksdbRaw* rocks = static_cast<RocksdbRaw*>(handler);
  common::Error er = rocks->del(argv[0]);
  if (!er) {
    common::StringValue *val = common::Value::createStringValue("DELETED");
    FastoObject* child = new FastoObject(out, val, rocks->config_.delimiter);
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
      common::StringValue *val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, rocks->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error help(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  RocksdbRaw* rocks = static_cast<RocksdbRaw*>(handler);
  return rocks->help(argc - 1, argv + 1);
}

}  // namespace rocksdb
}  // namespace fastonosql
