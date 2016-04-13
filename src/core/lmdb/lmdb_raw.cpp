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

#include "core/lmdb/lmdb_raw.h"

#include <errno.h>

#include <vector>
#include <string>

#include "common/file_system.h"
#include "common/utils.h"

#define LMDB_OK 0

namespace fastonosql {
namespace core {
namespace lmdb {

namespace {

int lmdb_open(lmdb** context, const char* dbname, bool create_if_missing) {
  if (create_if_missing) {
    common::Error err = common::file_system::create_directory(dbname, true);
    if (err && err->isError()) {
      return EACCES;
    }
    if (common::file_system::is_directory(dbname) != common::SUCCESS) {
      return EACCES;
    }
  }

  lmdb* lcontext = reinterpret_cast<lmdb*>(calloc(1, sizeof(lmdb)));
  int rc = mdb_env_create(&lcontext->env);
  if (rc != LMDB_OK) {
    free(lcontext);
    return rc;
  }
  rc = mdb_env_open(lcontext->env, dbname, 0, 0664);
  if (rc != LMDB_OK) {
    free(lcontext);
    return rc;
  }

  MDB_txn* txn = NULL;
  rc = mdb_txn_begin(lcontext->env, NULL, 0, &txn);
  if (rc != LMDB_OK) {
    free(lcontext);
    return rc;
  }

  rc = mdb_dbi_open(txn, NULL, 0, &lcontext->dbir);
  mdb_txn_abort(txn);
  if (rc != LMDB_OK) {
    free(lcontext);
    return rc;
  }

  *context = lcontext;
  return rc;
}

void lmdb_close(lmdb** context) {
  if (!context) {
    return;
  }

  lmdb* lcontext = *context;
  if (!lcontext) {
    return;
  }

  mdb_dbi_close(lcontext->env, lcontext->dbir);
  mdb_env_close(lcontext->env);
  free(lcontext);
  *context = NULL;
}

}  // namespace

common::Error createConnection(const LmdbConfig& config, LMDBConnection** context) {
  if (!context) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  DCHECK(*context == nullptr);
  struct lmdb* lcontext = nullptr;
  const char* dbname = common::utils::c_strornull(config.dbname);
  int st = lmdb_open(&lcontext, dbname, config.create_if_missing);
  if (st != LMDB_OK) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "Fail open database: %s", mdb_strerror(st));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  *context = lcontext;
  return common::Error();
}

common::Error createConnection(LmdbConnectionSettings* settings, LMDBConnection** context) {
  if (!settings) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  LmdbConfig config = settings->info();
  return createConnection(config, context);
}

common::Error testConnection(LmdbConnectionSettings* settings) {
  if (!settings) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  struct lmdb* ldb = nullptr;
  common::Error er = createConnection(settings, &ldb);
  if (er && er->isError()) {
    return er;
  }

  lmdb_close(&ldb);
  return common::Error();
}

LmdbRaw::LmdbRaw()
  : CommandHandler(lmdbCommands), connection_() {
}

common::Error LmdbRaw::connect(const config_t& config) {
  return connection_.connect(config);
}

common::Error LmdbRaw::disconnect() {
  return connection_.disconnect();
}

bool LmdbRaw::isConnected() const {
  return connection_.isConnected();
}

std::string LmdbRaw::delimiter() const {
  return connection_.config_.delimiter;
}

LmdbRaw::config_t LmdbRaw::config() const {
  return connection_.config_;
}

const char* LmdbRaw::versionApi() {
  return STRINGIZE(MDB_VERSION_MAJOR) "." STRINGIZE(MDB_VERSION_MINOR) "." STRINGIZE(MDB_VERSION_PATCH);
}

MDB_dbi LmdbRaw::curDb() const {
  if (connection_.handle_) {
    return connection_.handle_->dbir;
  }

  return 0;
}

common::Error LmdbRaw::info(const char* args, LmdbServerInfo::Stats* statsout) {
  CHECK(isConnected());

  if (!statsout) {
    NOTREACHED();
    return common::make_error_value("Invalid input argument for command: INFO",
                                    common::ErrorValue::E_ERROR);
  }

  LmdbServerInfo::Stats linfo;
  LmdbConfig conf = config();
  linfo.file_name = conf.dbname;

  *statsout = linfo;
  return common::Error();
}

common::Error LmdbRaw::dbsize(size_t* size) {
  CHECK(isConnected());

  if (!size) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  MDB_cursor* cursor = NULL;
  MDB_txn* txn = NULL;
  int rc = mdb_txn_begin(connection_.handle_->env, NULL, MDB_RDONLY, &txn);
  if (rc == LMDB_OK) {
    rc = mdb_cursor_open(txn, connection_.handle_->dbir, &cursor);
  }

  if (rc != LMDB_OK) {
    mdb_txn_abort(txn);
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "dbsize function error: %s", mdb_strerror(rc));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  MDB_val key;
  MDB_val data;
  size_t sz = 0;
  while (mdb_cursor_get(cursor, &key, &data, MDB_NEXT) == LMDB_OK) {
    sz++;
  }
  mdb_cursor_close(cursor);
  mdb_txn_abort(txn);

  *size = sz;
  return common::Error();
}

common::Error LmdbRaw::set(const std::string& key, const std::string& value) {
  CHECK(isConnected());

  MDB_val mkey;
  mkey.mv_size = key.size();
  mkey.mv_data = (void*)key.c_str();
  MDB_val mval;
  mval.mv_size = value.size();
  mval.mv_data = (void*)value.c_str();

  MDB_txn* txn = NULL;
  int rc = mdb_txn_begin(connection_.handle_->env, NULL, 0, &txn);
  if (rc == LMDB_OK) {
    rc = mdb_put(txn, connection_.handle_->dbir, &mkey, &mval, 0);
    if (rc == LMDB_OK) {
      rc = mdb_txn_commit(txn);
    } else {
      mdb_txn_abort(txn);
    }
  }

  if (rc != LMDB_OK) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "set function error: %s", mdb_strerror(rc));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error LmdbRaw::get(const std::string& key, std::string* ret_val) {
  CHECK(isConnected());

  MDB_val mkey;
  mkey.mv_size = key.size();
  mkey.mv_data = (void*)key.c_str();
  MDB_val mval;

  MDB_txn* txn = NULL;
  int rc = mdb_txn_begin(connection_.handle_->env, NULL, MDB_RDONLY, &txn);
  if (rc == LMDB_OK) {
    rc = mdb_get(txn, connection_.handle_->dbir, &mkey, &mval);
  }
  mdb_txn_abort(txn);

  if (rc != LMDB_OK) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "get function error: %s", mdb_strerror(rc));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  ret_val->assign((const char*)mval.mv_data, mval.mv_size);

  return common::Error();
}

common::Error LmdbRaw::del(const std::string& key) {
  CHECK(isConnected());

  MDB_val mkey;
  mkey.mv_size = key.size();
  mkey.mv_data = (void*)key.c_str();

  MDB_txn* txn = NULL;
  int rc = mdb_txn_begin(connection_.handle_->env, NULL, 0, &txn);
  if (rc == LMDB_OK) {
    rc = mdb_del(txn, connection_.handle_->dbir, &mkey, NULL);
    if (rc == LMDB_OK) {
      rc = mdb_txn_commit(txn);
    } else {
      mdb_txn_abort(txn);
    }
  }

  if (rc != LMDB_OK) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "delete function error: %s", mdb_strerror(rc));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error LmdbRaw::keys(const std::string& key_start, const std::string& key_end,
                            uint64_t limit, std::vector<std::string>* ret) {
  CHECK(isConnected());

  MDB_cursor* cursor = NULL;
  MDB_txn* txn = NULL;
  int rc = mdb_txn_begin(connection_.handle_->env, NULL, MDB_RDONLY, &txn);
  if (rc == LMDB_OK) {
    rc = mdb_cursor_open(txn, connection_.handle_->dbir, &cursor);
  }

  if (rc != LMDB_OK) {
    mdb_txn_abort(txn);
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "Keys function error: %s", mdb_strerror(rc));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  MDB_val key;
  MDB_val data;
  while ((mdb_cursor_get(cursor, &key, &data, MDB_NEXT) == LMDB_OK) && limit > ret->size()) {
    std::string skey((const char*)key.mv_data, key.mv_size);
    if (key_start < skey && key_end > skey) {
      ret->push_back(skey);
    }
  }

  mdb_cursor_close(cursor);
  mdb_txn_abort(txn);
  return common::Error();
}

common::Error LmdbRaw::help(int argc, char** argv) {
  return notSupported("HELP");
}

common::Error LmdbRaw::flushdb() {
  CHECK(isConnected());

  MDB_cursor* cursor = NULL;
  MDB_txn* txn = NULL;
  int rc = mdb_txn_begin(connection_.handle_->env, NULL, 0, &txn);
  if (rc == LMDB_OK) {
    rc = mdb_cursor_open(txn, connection_.handle_->dbir, &cursor);
  }

  if (rc != LMDB_OK) {
    mdb_txn_abort(txn);
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "flushdb function error: %s", mdb_strerror(rc));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  MDB_val key;
  MDB_val data;
  size_t sz = 0;
  while (mdb_cursor_get(cursor, &key, &data, MDB_NEXT) == LMDB_OK) {
    sz++;
    rc = mdb_del(txn, connection_.handle_->dbir, &key, NULL);
    if (rc != LMDB_OK) {
      mdb_cursor_close(cursor);
      mdb_txn_abort(txn);
      std::string buff = common::MemSPrintf("del function error: %s", mdb_strerror(rc));
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }
  }

  mdb_cursor_close(cursor);
  if (sz != 0) {
    rc = mdb_txn_commit(txn);
    if (rc != LMDB_OK) {
      std::string buff = common::MemSPrintf("commit function error: %s", mdb_strerror(rc));
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }
    return common::Error();
  }

  mdb_txn_abort(txn);
  return common::Error();
}

common::Error info(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  LmdbRaw* mdb = static_cast<LmdbRaw*>(handler);
  LmdbServerInfo::Stats statsout;
  common::Error er = mdb->info(argc == 1 ? argv[0] : nullptr, &statsout);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue(LmdbServerInfo(statsout).toString());
    FastoObject* child = new FastoObject(out, val, mdb->delimiter());
    out->addChildren(child);
  }

  return er;
}

common::Error dbsize(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  LmdbRaw* mdb = static_cast<LmdbRaw*>(handler);
  size_t dbsize = 0;
  common::Error er = mdb->dbsize(&dbsize);
  if (!er) {
    common::FundamentalValue* val = common::Value::createUIntegerValue(dbsize);
    FastoObject* child = new FastoObject(out, val, mdb->delimiter());
    out->addChildren(child);
  }

  return er;
}

common::Error set(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  LmdbRaw* mdb = static_cast<LmdbRaw*>(handler);
  common::Error er = mdb->set(argv[0], argv[1]);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("STORED");
    FastoObject* child = new FastoObject(out, val, mdb->delimiter());
    out->addChildren(child);
  }

  return er;
}

common::Error get(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  LmdbRaw* mdb = static_cast<LmdbRaw*>(handler);
  std::string ret;
  common::Error er = mdb->get(argv[0], &ret);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue(ret);
    FastoObject* child = new FastoObject(out, val, mdb->delimiter());
    out->addChildren(child);
  }

  return er;
}

common::Error del(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  LmdbRaw* mdb = static_cast<LmdbRaw*>(handler);
  common::Error er = mdb->del(argv[0]);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("DELETED");
    FastoObject* child = new FastoObject(out, val, mdb->delimiter());
    out->addChildren(child);
  }

  return er;
}

common::Error keys(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  LmdbRaw* mdb = static_cast<LmdbRaw*>(handler);
  std::vector<std::string> keysout;
  common::Error er = mdb->keys(argv[0], argv[1], atoll(argv[2]), &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, mdb->delimiter());
    out->addChildren(child);
  }

  return er;
}

common::Error help(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  LmdbRaw* mdb = static_cast<LmdbRaw*>(handler);
  return mdb->help(argc - 1, argv + 1);
}

common::Error flushdb(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  LmdbRaw* mdb = static_cast<LmdbRaw*>(handler);
  return mdb->flushdb();
}

}  // namespace lmdb
template<>
common::Error DBAllocatorTraits<lmdb::LMDBConnection, lmdb::LmdbConfig>::connect(const lmdb::LmdbConfig& config, lmdb::LMDBConnection** hout) {
  lmdb::LMDBConnection* context = nullptr;
  common::Error er = lmdb::createConnection(config, &context);
  if (er && er->isError()) {
    return er;
  }

  *hout = context;
  return common::Error();
}
template<>
common::Error DBAllocatorTraits<lmdb::LMDBConnection, lmdb::LmdbConfig>::disconnect(lmdb::LMDBConnection** handle) {
  lmdb::lmdb_close(handle);
  *handle = nullptr;
  return common::Error();
}
template<>
bool DBAllocatorTraits<lmdb::LMDBConnection, lmdb::LmdbConfig>::isConnected(lmdb::LMDBConnection* handle) {
  if (!handle) {
    return false;
  }

  return true;
}
}  // namespace core
}  // namespace fastonosql
