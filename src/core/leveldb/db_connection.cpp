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

#include "core/leveldb/db_connection.h"

#include <leveldb/c.h>  // for leveldb_major_version, etc
#include <leveldb/db.h>
#include <leveldb/options.h>  // for ReadOptions, WriteOptions

#include <common/sprintf.h>
#include <common/convert2string.h>  // for ConvertFromString

#include "core/leveldb/connection_settings.h"  // for ConnectionSettings
#include "core/leveldb/command_translator.h"
#include "core/leveldb/database.h"

#include "global/global.h"  // for FastoObject, etc

#define LEVELDB_HEADER_STATS                             \
  "                               Compactions\n"         \
  "Level  Files Size(MB) Time(sec) Read(MB) Write(MB)\n" \
  "--------------------------------------------------\n"

namespace fastonosql {
namespace core {
template <>
common::Error ConnectionAllocatorTraits<leveldb::NativeConnection, leveldb::Config>::connect(
    const leveldb::Config& config,
    leveldb::NativeConnection** hout) {
  leveldb::NativeConnection* context = nullptr;
  common::Error er = leveldb::createConnection(config, &context);
  if (er && er->isError()) {
    return er;
  }

  *hout = context;
  return common::Error();
}
template <>
common::Error ConnectionAllocatorTraits<leveldb::NativeConnection, leveldb::Config>::disconnect(
    leveldb::NativeConnection** handle) {
  destroy(handle);
  return common::Error();
}
template <>
bool ConnectionAllocatorTraits<leveldb::NativeConnection, leveldb::Config>::isConnected(
    leveldb::NativeConnection* handle) {
  if (!handle) {
    return false;
  }

  return true;
}
namespace leveldb {

common::Error createConnection(const Config& config, NativeConnection** context) {
  if (!context) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  DCHECK(*context == nullptr);
  ::leveldb::DB* lcontext = nullptr;
  std::string folder = config.dbname;  // start point must be folder
  common::tribool is_dir = common::file_system::is_directory(folder);
  if (is_dir != common::SUCCESS) {
    folder = common::file_system::get_dir_path(folder);
  }
  auto st = ::leveldb::DB::Open(config.options, folder, &lcontext);
  if (!st.ok()) {
    std::string buff = common::MemSPrintf("Fail connect to server: %s!", st.ToString());
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

  leveldb::NativeConnection* ldb = nullptr;
  common::Error er = createConnection(settings, &ldb);
  if (er && er->isError()) {
    return er;
  }

  delete ldb;
  return common::Error();
}

DBConnection::DBConnection(CDBConnectionClient* client)
    : base_class(leveldbCommands, client, new CommandTranslator) {}

const char* DBConnection::versionApi() {
  static std::string leveldb_version =
      common::MemSPrintf("%d.%d", leveldb_major_version(), leveldb_minor_version());
  return leveldb_version.c_str();
}

common::Error DBConnection::dbkcount(size_t* size) {
  if (!size) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!isConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  ::leveldb::ReadOptions ro;
  ::leveldb::Iterator* it = connection_.handle_->NewIterator(ro);
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

common::Error DBConnection::info(const char* args, ServerInfo::Stats* statsout) {
  UNUSED(args);

  if (!statsout) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!isConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  std::string rets;
  bool isok = connection_.handle_->GetProperty("leveldb.stats", &rets);
  if (!isok) {
    return common::make_error_value("info function failed", common::ErrorValue::E_ERROR);
  }

  ServerInfo::Stats lstats;
  if (rets.size() > sizeof(LEVELDB_HEADER_STATS)) {
    const char* retsc = rets.c_str() + sizeof(LEVELDB_HEADER_STATS);
    char* p2 = strtok(const_cast<char*>(retsc), " ");
    int pos = 0;
    while (p2) {
      switch (pos++) {
        case 0:
          lstats.compactions_level = common::ConvertFromString<uint32_t>(p2);
          break;
        case 1:
          lstats.file_size_mb = common::ConvertFromString<uint32_t>(p2);
          break;
        case 2:
          lstats.time_sec = common::ConvertFromString<uint32_t>(p2);
          break;
        case 3:
          lstats.read_mb = common::ConvertFromString<uint32_t>(p2);
          break;
        case 4:
          lstats.write_mb = common::ConvertFromString<uint32_t>(p2);
          break;
        default:
          break;
      }
      p2 = strtok(0, " ");
    }
  }

  *statsout = lstats;
  return common::Error();
}

common::Error DBConnection::delInner(const std::string& key) {
  if (!isConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  ::leveldb::WriteOptions wo;
  auto st = connection_.handle_->Delete(wo, key);
  if (!st.ok()) {
    std::string buff = common::MemSPrintf("del function error: %s", st.ToString());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::setInner(const std::string& key, const std::string& value) {
  if (!isConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  ::leveldb::WriteOptions wo;
  auto st = connection_.handle_->Put(wo, key, value);
  if (!st.ok()) {
    std::string buff = common::MemSPrintf("set function error: %s", st.ToString());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error DBConnection::getInner(const std::string& key, std::string* ret_val) {
  if (!isConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  ::leveldb::ReadOptions ro;
  auto st = connection_.handle_->Get(ro, key, ret_val);
  if (!st.ok()) {
    std::string buff = common::MemSPrintf("get function error: %s", st.ToString());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error DBConnection::keys(const std::string& key_start,
                                 const std::string& key_end,
                                 uint64_t limit,
                                 std::vector<std::string>* ret) {
  if (!isConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  ::leveldb::ReadOptions ro;
  ::leveldb::Iterator* it =
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
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  ::leveldb::ReadOptions ro;
  ::leveldb::WriteOptions wo;
  ::leveldb::Iterator* it = connection_.handle_->NewIterator(ro);
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

common::Error DBConnection::delImpl(const NKeys& keys, NKeys* deleted_keys) {
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

common::Error DBConnection::setImpl(const NDbKValue& key, NDbKValue* added_key) {
  std::string key_str = key.keyString();
  std::string value_str = key.valueString();
  common::Error err = setInner(key_str, value_str);
  if (err && err->isError()) {
    return err;
  }

  *added_key = key;
  return common::Error();
}

common::Error DBConnection::getImpl(const NKey& key, NDbKValue* loaded_key) {
  std::string key_str = key.key();
  std::string value_str;
  common::Error err = getInner(key_str, &value_str);
  if (err && err->isError()) {
    return err;
  }

  NValue val(common::Value::createStringValue(value_str));
  *loaded_key = NDbKValue(key, val);
  return common::Error();
}

common::Error DBConnection::renameImpl(const NKey& key, const std::string& new_key) {
  std::string key_str = key.key();
  std::string value_str;
  common::Error err = getInner(key_str, &value_str);
  if (err && err->isError()) {
    return err;
  }

  err = delInner(key_str);
  if (err && err->isError()) {
    return err;
  }

  err = setInner(new_key, value_str);
  if (err && err->isError()) {
    return err;
  }

  return common::Error();
}

common::Error DBConnection::setTTLImpl(const NKey& key, ttl_t ttl) {
  UNUSED(key);
  UNUSED(ttl);
  return common::make_error_value("Sorry, but now " PROJECT_NAME_TITLE
                                  " for LevelDB not supported TTL commands.",
                                  common::ErrorValue::E_ERROR);
}

common::Error dbkcount(CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);
  UNUSED(argv);

  DBConnection* level = static_cast<DBConnection*>(handler);

  size_t dbkcount = 0;
  common::Error er = level->dbkcount(&dbkcount);
  if (!er) {
    common::FundamentalValue* val = common::Value::createUIntegerValue(dbkcount);
    FastoObject* child = new FastoObject(out, val, level->delimiter());
    out->addChildren(child);
  }
  return er;
}

common::Error info(CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  DBConnection* level = static_cast<DBConnection*>(handler);

  ServerInfo::Stats statsout;
  common::Error er = level->info(argc == 1 ? argv[0] : nullptr, &statsout);
  if (!er) {
    ServerInfo linf(statsout);
    common::StringValue* val = common::Value::createStringValue(linf.toString());
    FastoObject* child = new FastoObject(out, val, level->delimiter());
    out->addChildren(child);
  }
  return er;
}

common::Error select(CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* level = static_cast<DBConnection*>(handler);
  common::Error err = level->select(argv[0], nullptr);
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
  NValue string_val(common::Value::createStringValue(argv[1]));
  NDbKValue kv(key, string_val);

  DBConnection* red = static_cast<DBConnection*>(handler);
  NDbKValue key_added;
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
  NDbKValue key_loaded;
  common::Error err = unqlite->get(key, &key_loaded);
  if (err && err->isError()) {
    return err;
  }

  NValue val = key_loaded.value();
  common::Value* copy = val->deepCopy();
  FastoObject* child = new FastoObject(out, copy, unqlite->delimiter());
  out->addChildren(child);
  return common::Error();
}

common::Error del(CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  NKeys keysdel;
  for (int i = 0; i < argc; ++i) {
    keysdel.push_back(NKey(argv[i]));
  }

  DBConnection* level = static_cast<DBConnection*>(handler);
  NKeys keys_deleted;
  common::Error err = level->del(keysdel, &keys_deleted);
  if (err && err->isError()) {
    return err;
  }

  common::FundamentalValue* val = common::Value::createUIntegerValue(keys_deleted.size());
  FastoObject* child = new FastoObject(out, val, level->delimiter());
  out->addChildren(child);
  return common::Error();
}

common::Error rename(CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  NKey key(argv[0]);
  DBConnection* red = static_cast<DBConnection*>(handler);
  common::Error err = red->rename(key, argv[1]);
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, red->delimiter());
  out->addChildren(child);
  return common::Error();
}

common::Error set_ttl(CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(out);
  UNUSED(argc);

  DBConnection* level = static_cast<DBConnection*>(handler);
  NKey key(argv[0]);
  time_t ttl = common::ConvertFromString<time_t>(argv[1]);
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

  DBConnection* level = static_cast<DBConnection*>(handler);

  std::vector<std::string> keysout;
  common::Error er =
      level->keys(argv[0], argv[1], common::ConvertFromString<uint64_t>(argv[2]), &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, level->delimiter());
    out->addChildren(child);
  }
  return er;
}

common::Error help(CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(out);

  DBConnection* level = static_cast<DBConnection*>(handler);
  return level->help(argc - 1, argv + 1);
}

common::Error flushdb(CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);
  UNUSED(argv);
  UNUSED(out);

  DBConnection* level = static_cast<DBConnection*>(handler);
  return level->flushdb();
}

}  // namespace leveldb
}  // namespace core
}  // namespace fastonosql
