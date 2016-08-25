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

#include <stdlib.h>                     // for atoll
#include <string.h>                     // for strtok

#include <memory>                       // for __shared_ptr

#include <leveldb/c.h>                  // for leveldb_major_version, etc
#include <leveldb/options.h>            // for ReadOptions, WriteOptions
#include <leveldb/db.h>

#include "common/convert2string.h"      // for ConvertFromString
#include "common/value.h"               // for Value, etc
#include "common/sprintf.h"

#include "core/leveldb/config.h"        // for Config
#include "core/leveldb/connection_settings.h"  // for ConnectionSettings

#include "global/global.h"              // for FastoObject, etc


#define LEVELDB_HEADER_STATS    "                               Compactions\n"\
                                "Level  Files Size(MB) Time(sec) Read(MB) Write(MB)\n"\
                                "--------------------------------------------------\n"

namespace fastonosql {
namespace core {
template<>
common::Error ConnectionAllocatorTraits<leveldb::NativeConnection, leveldb::Config>::connect(const leveldb::Config& config, leveldb::NativeConnection** hout) {
  leveldb::NativeConnection* context = nullptr;
  common::Error er = leveldb::createConnection(config, &context);
  if (er && er->isError()) {
    return er;
  }

  *hout = context;
  return common::Error();
}
template<>
common::Error ConnectionAllocatorTraits<leveldb::NativeConnection, leveldb::Config>::disconnect(leveldb::NativeConnection** handle) {
  destroy(handle);
  return common::Error();
}
template<>
bool ConnectionAllocatorTraits<leveldb::NativeConnection, leveldb::Config>::isConnected(leveldb::NativeConnection* handle) {
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
  auto st = ::leveldb::DB::Open(config.options, config.dbname, &lcontext);
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

DBConnection::DBConnection()
  : CommandHandler(leveldbCommands), connection_() {
}

common::Error DBConnection::connect(const config_t& config) {
  return connection_.connect(config);
}

common::Error DBConnection::disconnect() {
  return connection_.disconnect();
}

bool DBConnection::isConnected() const {
  return connection_.isConnected();
}

std::string DBConnection::delimiter() const {
  return connection_.config_.delimiter;
}

std::string DBConnection::nsSeparator() const {
  return connection_.config_.ns_separator;
}

DBConnection::config_t DBConnection::config() const {
  return connection_.config_;
}

const char* DBConnection::versionApi() {
  static std::string leveldb_version = common::MemSPrintf("%d.%d", leveldb_major_version(), leveldb_minor_version());
  return leveldb_version.c_str();
}

common::Error DBConnection::dbkcount(size_t* size) {
  if (!isConnected()) {
    DNOTREACHED();
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  if (!size) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
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

  if (!isConnected()) {
    DNOTREACHED();
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  if (!statsout) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
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

common::Error DBConnection::set(const std::string& key, const std::string& value) {
  if (!isConnected()) {
    DNOTREACHED();
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

common::Error DBConnection::get(const std::string& key, std::string* ret_val) {
  if (!isConnected()) {
    DNOTREACHED();
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

common::Error DBConnection::del(const std::string& key) {
  if (!isConnected()) {
    DNOTREACHED();
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

common::Error DBConnection::keys(const std::string& key_start, const std::string& key_end,
                   uint64_t limit, std::vector<std::string>* ret) {
  if (!isConnected()) {
    DNOTREACHED();
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  ::leveldb::ReadOptions ro;
  ::leveldb::Iterator* it = connection_.handle_->NewIterator(ro);  // keys(key_start, key_end, limit, ret);
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

common::Error DBConnection::help(int argc, char** argv) {
  UNUSED(argc);
  UNUSED(argv);

  return notSupported("HELP");
}

common::Error DBConnection::flushdb() {
  if (!isConnected()) {
    DNOTREACHED();
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

common::Error dbkcount(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
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

common::Error info(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
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

common::Error set(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* level = static_cast<DBConnection*>(handler);

  common::Error er = level->set(argv[0], argv[1]);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, level->delimiter());
    out->addChildren(child);
  }
  return er;
}

common::Error get(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* level = static_cast<DBConnection*>(handler);

  std::string ret;
  common::Error er = level->get(argv[0], &ret);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue(ret);
    FastoObject* child = new FastoObject(out, val, level->delimiter());
    out->addChildren(child);
  }
  return er;
}

common::Error del(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* level = static_cast<DBConnection*>(handler);

  common::Error er = level->del(argv[0]);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, level->delimiter());
    out->addChildren(child);
  }
  return er;
}

common::Error keys(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* level = static_cast<DBConnection*>(handler);

  std::vector<std::string> keysout;
  common::Error er = level->keys(argv[0], argv[1], atoll(argv[2]), &keysout);
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

common::Error help(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  UNUSED(out);

  DBConnection* level = static_cast<DBConnection*>(handler);
  return level->help(argc - 1, argv + 1);
}

common::Error flushdb(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  UNUSED(argc);
  UNUSED(argv);
  UNUSED(out);

  DBConnection* level = static_cast<DBConnection*>(handler);
  return level->flushdb();
}

}  // namespace leveldb
}  // namespace core
}  // namespace fastonosql
