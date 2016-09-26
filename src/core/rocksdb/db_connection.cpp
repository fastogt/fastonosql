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

#include "core/rocksdb/db_connection.h"

#include <stdlib.h>  // for atoll
#include <string.h>  // for strtok

#include <memory>  // for __shared_ptr
#include <string>  // for string, operator<, etc
#include <vector>  // for vector

#include <rocksdb/db.h>

#include "common/convert2string.h"  // for ConvertFromString
#include "common/sprintf.h"         // for MemSPrintf
#include "common/value.h"           // for Value::ErrorsType::E_ERROR, etc

#include "core/rocksdb/config.h"               // for Config
#include "core/rocksdb/connection_settings.h"  // for ConnectionSettings
#include "core/rocksdb/database.h"
#include "core/rocksdb/command_translator.h"

#include "global/global.h"  // for FastoObject, etc

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
common::Error ConnectionAllocatorTraits<rocksdb::NativeConnection, rocksdb::Config>::connect(
    const rocksdb::Config& config,
    rocksdb::NativeConnection** hout) {
  rocksdb::NativeConnection* context = nullptr;
  common::Error er = rocksdb::createConnection(config, &context);
  if (er && er->isError()) {
    return er;
  }

  *hout = context;
  return common::Error();
}
template <>
common::Error ConnectionAllocatorTraits<rocksdb::NativeConnection, rocksdb::Config>::disconnect(
    rocksdb::NativeConnection** handle) {
  destroy(handle);
  return common::Error();
}
template <>
bool ConnectionAllocatorTraits<rocksdb::NativeConnection, rocksdb::Config>::isConnected(
    rocksdb::NativeConnection* handle) {
  if (!handle) {
    return false;
  }

  return true;
}
namespace rocksdb {

common::Error createConnection(const Config& config, NativeConnection** context) {
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

common::Error createConnection(ConnectionSettings* settings, NativeConnection** context) {
  if (!settings) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  Config config = settings->info();
  return createConnection(config, context);
}

common::Error testConnection(ConnectionSettings* settings) {
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

DBConnection::DBConnection(CDBConnectionClient* client)
    : base_class(rocksdbCommands, client, new CommandTranslator) {}

const char* DBConnection::versionApi() {
  return STRINGIZE(ROCKSDB_MAJOR) "." STRINGIZE(ROCKSDB_MINOR) "." STRINGIZE(ROCKSDB_PATCH);
}

common::Error DBConnection::info(const char* args, ServerInfo::Stats* statsout) {
  UNUSED(args);

  if (!isConnected()) {
    DNOTREACHED();
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  if (!statsout) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
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
        case 0:
          lstatsout.compactions_level = common::ConvertFromString<uint32_t>(p2);
          break;
        case 1:
          lstatsout.file_size_mb = common::ConvertFromString<uint32_t>(p2);
          break;
        case 2:
          lstatsout.time_sec = common::ConvertFromString<uint32_t>(p2);
          break;
        case 3:
          lstatsout.read_mb = common::ConvertFromString<uint32_t>(p2);
          break;
        case 4:
          lstatsout.write_mb = common::ConvertFromString<uint32_t>(p2);
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

common::Error DBConnection::dbkcount(size_t* size) {
  if (!isConnected()) {
    DNOTREACHED();
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

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
    std::string buff = common::MemSPrintf("Couldn't determine DBKCOUNT error: %s", st.ToString());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  *size = sz;
  return common::Error();
}

std::string DBConnection::currentDbName() const {
  if (!isConnected()) {
    DNOTREACHED();
    return "unknown";
  }

  ::rocksdb::ColumnFamilyHandle* fam = connection_.handle_->DefaultColumnFamily();
  if (fam) {
    return fam->GetName();
  }

  return "default";
}

common::Error DBConnection::getInner(const std::string& key, std::string* ret_val) {
  if (!isConnected()) {
    DNOTREACHED();
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  ::rocksdb::ReadOptions ro;
  auto st = connection_.handle_->Get(ro, key, ret_val);
  if (!st.ok()) {
    std::string buff = common::MemSPrintf("get function error: %s", st.ToString());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error DBConnection::mget(const std::vector< ::rocksdb::Slice>& keys,
                                 std::vector<std::string>* ret) {
  if (!isConnected()) {
    DNOTREACHED();
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

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

common::Error DBConnection::merge(const std::string& key, const std::string& value) {
  if (!isConnected()) {
    DNOTREACHED();
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

common::Error DBConnection::setInner(const std::string& key, const std::string& value) {
  if (!isConnected()) {
    DNOTREACHED();
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  ::rocksdb::WriteOptions wo;
  auto st = connection_.handle_->Put(wo, key, value);
  if (!st.ok()) {
    std::string buff = common::MemSPrintf("set function error: %s", st.ToString());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error DBConnection::delInner(const std::string& key) {
  if (!isConnected()) {
    DNOTREACHED();
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  ::rocksdb::WriteOptions wo;
  auto st = connection_.handle_->Delete(wo, key);
  if (!st.ok()) {
    std::string buff = common::MemSPrintf("del function error: %s", st.ToString());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::keys(const std::string& key_start,
                                 const std::string& key_end,
                                 uint64_t limit,
                                 std::vector<std::string>* ret) {
  if (!isConnected()) {
    DNOTREACHED();
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  ::rocksdb::ReadOptions ro;
  ::rocksdb::Iterator* it =
      connection_.handle_->NewIterator(ro);  // keys(key_start, key_end, limit, ret);
  for (it->Seek(key_start); it->Valid() && it->key().ToString() < key_end; it->Next()) {
    std::string key = it->key().ToString();
    if (ret->size() < limit) {
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

common::Error DBConnection::help(int argc, const char** argv) {
  UNUSED(argc);
  UNUSED(argv);

  return notSupported("HELP");
}

common::Error DBConnection::flushdb() {
  if (!isConnected()) {
    DNOTREACHED();
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

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

common::Error DBConnection::selectImpl(const std::string& name, IDataBaseInfo** info) {
  size_t kcount = 0;
  common::Error err = dbkcount(&kcount);
  MCHECK(!err);
  *info = new DataBaseInfo(name, true, kcount);
  return common::Error();
}

common::Error DBConnection::setImpl(const key_and_value_t& key, key_and_value_t* added_key) {
  std::string key_str = key.keyString();
  std::string value_str = key.valueString();
  common::Error err = setInner(key_str, value_str);
  if (err && err->isError()) {
    return err;
  }

  *added_key = key;
  return common::Error();
}

common::Error DBConnection::getImpl(const key_t& key, key_and_value_t* loaded_key) {
  std::string key_str = key.key();
  std::string value_str;
  common::Error err = getInner(key_str, &value_str);
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue(value_str);
  *loaded_key = key_and_value_t(key, common::make_value(val));
  return common::Error();
}

common::Error DBConnection::delImpl(const keys_t& keys, keys_t* deleted_keys) {
  for (size_t i = 0; i < keys.size(); ++i) {
    NKey key = keys[i];
    std::string key_str = key.key();
    common::Error err = delInner(key_str);
    if (err && err->isError()) {
      continue;
    }

    deleted_keys->push_back(key);
  }

  return common::Error();
}

common::Error DBConnection::setTTLImpl(const key_t& key, ttl_t ttl) {
  UNUSED(key);
  UNUSED(ttl);
  return common::make_error_value("Sorry, but now " PROJECT_NAME_TITLE
                                  " for SSDB not supported TTL commands.",
                                  common::ErrorValue::E_ERROR);
}

common::Error info(CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  DBConnection* rocks = static_cast<DBConnection*>(handler);
  ServerInfo::Stats statsout;
  common::Error er = rocks->info(argc == 1 ? argv[0] : nullptr, &statsout);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue(ServerInfo(statsout).toString());
    FastoObject* child = new FastoObject(out, val, rocks->delimiter());
    out->addChildren(child);
  }

  return er;
}

common::Error select(CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* level = static_cast<DBConnection*>(handler);
  common::Error err = level->select(argv[0], NULL);
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, level->delimiter());
  out->addChildren(child);
  return common::Error();
}

common::Error set(CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  NKey key(argv[0]);
  common::StringValue* string_val = common::Value::createStringValue(argv[1]);
  key_and_value_t kv(key, common::make_value(string_val));

  DBConnection* red = static_cast<DBConnection*>(handler);
  key_and_value_t key_added;
  common::Error err = red->set(kv, &key_added);
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, red->delimiter());
  out->addChildren(child);
  return common::Error();
}

common::Error get(CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  NKey key(argv[0]);
  DBConnection* unqlite = static_cast<DBConnection*>(handler);
  key_and_value_t key_loaded;
  common::Error err = unqlite->get(key, &key_loaded);
  if (err && err->isError()) {
    return err;
  }

  value_t val = key_loaded.value();
  common::Value* copy = val->deepCopy();
  FastoObject* child = new FastoObject(out, copy, unqlite->delimiter());
  out->addChildren(child);
  return common::Error();
}

common::Error mget(CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  DBConnection* rocks = static_cast<DBConnection*>(handler);
  std::vector< ::rocksdb::Slice> keysget;
  for (int i = 0; i < argc; ++i) {
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
    FastoObjectArray* child = new FastoObjectArray(out, ar, rocks->delimiter());
    out->addChildren(child);
  }

  return er;
}

common::Error merge(CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* rocks = static_cast<DBConnection*>(handler);
  common::Error er = rocks->merge(argv[0], argv[1]);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, rocks->delimiter());
    out->addChildren(child);
  }

  return er;
}

common::Error del(CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  keys_t keysdel;
  for (int i = 0; i < argc; ++i) {
    keysdel.push_back(NKey(argv[i]));
  }

  DBConnection* level = static_cast<DBConnection*>(handler);
  keys_t keys_deleted;
  common::Error err = level->del(keysdel, &keys_deleted);
  if (err && err->isError()) {
    return err;
  }

  common::FundamentalValue* val = common::Value::createUIntegerValue(keys_deleted.size());
  FastoObject* child = new FastoObject(out, val, level->delimiter());
  out->addChildren(child);
  return common::Error();
}

common::Error set_ttl(CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(out);
  UNUSED(argc);

  DBConnection* level = static_cast<DBConnection*>(handler);
  key_t key(argv[0]);
  ttl_t ttl = common::ConvertFromString<ttl_t>(argv[1]);
  common::Error err = level->setTTL(key, ttl);
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, level->delimiter());
  out->addChildren(child);
  return common::Error();
}

common::Error keys(CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* rocks = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysout;
  common::Error er = rocks->keys(argv[0], argv[1], atoll(argv[2]), &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, rocks->delimiter());
    out->addChildren(child);
  }

  return er;
}

common::Error dbkcount(CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);
  UNUSED(argv);

  DBConnection* rocks = static_cast<DBConnection*>(handler);
  size_t dbkcount = 0;
  common::Error er = rocks->dbkcount(&dbkcount);
  if (!er) {
    common::FundamentalValue* val = common::Value::createUIntegerValue(dbkcount);
    FastoObject* child = new FastoObject(out, val, rocks->delimiter());
    out->addChildren(child);
  }

  return er;
}

common::Error help(CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(out);

  DBConnection* rocks = static_cast<DBConnection*>(handler);
  return rocks->help(argc - 1, argv + 1);
}

common::Error flushdb(CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);
  UNUSED(argv);
  UNUSED(out);

  DBConnection* rocks = static_cast<DBConnection*>(handler);
  return rocks->flushdb();
}

}  // namespace rocksdb
}  // namespace core
}  // namespace fastonosql
