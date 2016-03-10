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

#include "core/leveldb/leveldb_raw.h"

#include <mutex>

#include "common/sprintf.h"

#define LEVELDB_HEADER_STATS    "                               Compactions\n"\
                                "Level  Files Size(MB) Time(sec) Read(MB) Write(MB)\n"\
                                "--------------------------------------------------\n"

namespace {

std::once_flag leveldb_version_once;
void leveldb_version_startup_function(char * version) {
  sprintf(version, "%d.%d", leveldb::kMajorVersion, leveldb::kMinorVersion);
}

}

namespace fastonosql {
namespace leveldb {

namespace {

common::Error createConnection(const LeveldbConfig& config, ::leveldb::DB** context) {
  if (!context) {
    return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
  }

  DCHECK(*context == nullptr);
  ::leveldb::DB* lcontext = nullptr;
  auto st = ::leveldb::DB::Open(config.options, config.dbname, &lcontext);
  if (!st.ok()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "Fail connect to server: %s!", st.ToString());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  *context = lcontext;
  return common::Error();
}

common::Error createConnection(LeveldbConnectionSettings* settings, ::leveldb::DB** context) {
  if (!settings) {
    return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
  }

  LeveldbConfig config = settings->info();
  return createConnection(config, context);
}

}  // namespace

common::Error testConnection(LeveldbConnectionSettings* settings) {
  if (!settings) {
    return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
  }

  ::leveldb::DB* ldb = nullptr;
  common::Error er = createConnection(settings, &ldb);
  if (er && er->isError()) {
    return er;
  }

  delete ldb;
  return common::Error();
}

LeveldbRaw::LeveldbRaw()
  : CommandHandler(leveldbCommands), leveldb_(nullptr) {
}

LeveldbRaw::~LeveldbRaw() {
  destroy(&leveldb_);
}

const char* LeveldbRaw::versionApi() {
  static char leveldb_version[32] = {0};
  std::call_once(leveldb_version_once, leveldb_version_startup_function, leveldb_version);
  return leveldb_version;
}

bool LeveldbRaw::isConnected() const {
  if (!leveldb_) {
    return false;
  }

  return true;
}

common::Error LeveldbRaw::connect() {
  if (isConnected()) {
    return common::Error();
  }

  ::leveldb::DB* context = nullptr;
  common::Error er = createConnection(config_, &context);
  if (er && er->isError()) {
    return er;
  }

  leveldb_ = context;
  return common::Error();
}

common::Error LeveldbRaw::disconnect() {
  if (!isConnected()) {
    return common::Error();
  }

  destroy(&leveldb_);
  return common::Error();
}

common::Error LeveldbRaw::dbsize(size_t* size) {
  if (!size) {
    return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
  }

  ::leveldb::ReadOptions ro;
  ::leveldb::Iterator* it = leveldb_->NewIterator(ro);
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

common::Error LeveldbRaw::info(const char* args, LeveldbServerInfo::Stats* statsout) {
  // sstables
  // stats
  // char prop[1024] = {0};
  // common::SNPrintf(prop, sizeof(prop), "leveldb.%s", args ? args : "stats");

  if (!statsout) {
    return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
  }

  std::string rets;
  bool isok = leveldb_->GetProperty("leveldb.stats", &rets);
  if (!isok) {
    return common::make_error_value("info function failed", common::ErrorValue::E_ERROR);
  }

  LeveldbServerInfo::Stats lstats;
  if (rets.size() > sizeof(LEVELDB_HEADER_STATS)) {
    const char * retsc = rets.c_str() + sizeof(LEVELDB_HEADER_STATS);
    char* p2 = strtok((char*)retsc, " ");
    int pos = 0;
    while (p2) {
      switch (pos++) {
        case 0:
          lstats.compactions_level = atoi(p2);
          break;
        case 1:
          lstats.file_size_mb = atoi(p2);
          break;
        case 2:
          lstats.time_sec = atoi(p2);
          break;
        case 3:
          lstats.read_mb = atoi(p2);
          break;
        case 4:
          lstats.write_mb = atoi(p2);
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

common::Error LeveldbRaw::get(const std::string& key, std::string* ret_val) {
  ::leveldb::ReadOptions ro;
  auto st = leveldb_->Get(ro, key, ret_val);
  if (!st.ok()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "get function error: %s", st.ToString());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error LeveldbRaw::put(const std::string& key, const std::string& value) {
  ::leveldb::WriteOptions wo;
  auto st = leveldb_->Put(wo, key, value);
  if (!st.ok()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "put function error: %s", st.ToString());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error LeveldbRaw::del(const std::string& key) {
  ::leveldb::WriteOptions wo;
  auto st = leveldb_->Delete(wo, key);
  if (!st.ok()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "del function error: %s", st.ToString());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error LeveldbRaw::keys(const std::string& key_start, const std::string& key_end,
                   uint64_t limit, std::vector<std::string> *ret) {
  ::leveldb::ReadOptions ro;
  ::leveldb::Iterator* it = leveldb_->NewIterator(ro);  // keys(key_start, key_end, limit, ret);
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

common::Error LeveldbRaw::help(int argc, char** argv) {
  return notSupported("HELP");
}

common::Error dbsize(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  LeveldbRaw* level = static_cast<LeveldbRaw*>(handler);

  size_t dbsize = 0;
  common::Error er = level->dbsize(&dbsize);
  if (!er) {
    common::FundamentalValue *val = common::Value::createUIntegerValue(dbsize);
    FastoObject* child = new FastoObject(out, val, level->config_.delimiter);
    out->addChildren(child);
  }
  return er;
}

common::Error info(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  LeveldbRaw* level = static_cast<LeveldbRaw*>(handler);

  LeveldbServerInfo::Stats statsout;
  common::Error er = level->info(argc == 1 ? argv[0] : nullptr, &statsout);
  if (!er) {
    LeveldbServerInfo linf(statsout);
    common::StringValue *val = common::Value::createStringValue(linf.toString());
    FastoObject* child = new FastoObject(out, val, level->config_.delimiter);
    out->addChildren(child);
  }
  return er;
}

common::Error get(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  LeveldbRaw* level = static_cast<LeveldbRaw*>(handler);

  std::string ret;
  common::Error er = level->get(argv[0], &ret);
  if (!er) {
    common::StringValue *val = common::Value::createStringValue(ret);
    FastoObject* child = new FastoObject(out, val, level->config_.delimiter);
    out->addChildren(child);
  }
  return er;
}

common::Error put(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  LeveldbRaw* level = static_cast<LeveldbRaw*>(handler);

  common::Error er = level->put(argv[0], argv[1]);
  if (!er) {
    common::StringValue *val = common::Value::createStringValue("STORED");
    FastoObject* child = new FastoObject(out, val, level->config_.delimiter);
    out->addChildren(child);
  }
  return er;
}

common::Error del(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  LeveldbRaw* level = static_cast<LeveldbRaw*>(handler);

  common::Error er = level->del(argv[0]);
  if (!er) {
    common::StringValue *val = common::Value::createStringValue("DELETED");
    FastoObject* child = new FastoObject(out, val, level->config_.delimiter);
    out->addChildren(child);
  }
  return er;
}

common::Error keys(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  LeveldbRaw* level = static_cast<LeveldbRaw*>(handler);

  std::vector<std::string> keysout;
  common::Error er = level->keys(argv[0], argv[1], atoll(argv[2]), &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue *val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, level->config_.delimiter);
    out->addChildren(child);
  }
  return er;
}

common::Error help(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  LeveldbRaw* level = static_cast<LeveldbRaw*>(handler);
  return level->help(argc - 1, argv + 1);
}

}  // namespace leveldb
}  // namespace fastonosql
