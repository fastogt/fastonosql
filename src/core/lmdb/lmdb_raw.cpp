/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    SiteOnYourDevice is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SiteOnYourDevice is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SiteOnYourDevice.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "core/lmdb/lmdb_raw.h"

#include <errno.h>

#include <vector>
#include <string>

#include "common/file_system.h"
#include "common/utils.h"

#define LMDB_OK 0

namespace fastonosql {
namespace lmdb {

namespace {

int lmdb_open(lmdb **context, const char *dbname, bool create_if_missing) {
  if (create_if_missing) {
    common::Error err = common::file_system::create_directory(dbname, true);
    if (err && err->isError()) {
      return EACCES;
    }
    if (common::file_system::is_directory(dbname) != common::SUCCESS) {
      return EACCES;
    }
  }

  lmdb *lcontext = reinterpret_cast<lmdb*>(calloc(1, sizeof(lmdb)));
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

  MDB_txn *txn = NULL;
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

void lmdb_close(lmdb **context) {
  lmdb *lcontext = *context;
  if (!lcontext) {
    return;
  }

  mdb_dbi_close(lcontext->env, lcontext->dbir);
  mdb_env_close(lcontext->env);
  free(lcontext);
  *context = NULL;
}

common::Error createConnection(const lmdbConfig& config, struct lmdb** context) {
  DCHECK(*context == NULL);

  struct lmdb* lcontext = NULL;
  const char * dbname = common::utils::c_strornull(config.dbname);
  int st = lmdb_open(&lcontext, dbname, config.create_if_missing);
  if (st != LMDB_OK) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "Fail open database: %s", mdb_strerror(st));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  *context = lcontext;
  return common::Error();
}

common::Error createConnection(LmdbConnectionSettings* settings, struct lmdb** context) {
  if (!settings) {
    return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
  }

  lmdbConfig config = settings->info();
  return createConnection(config, context);
}

}  // namespace

common::Error testConnection(fastonosql::lmdb::LmdbConnectionSettings* settings) {
  struct lmdb* ldb = NULL;
  common::Error er = createConnection(settings, &ldb);
  if (er && er->isError()) {
    return er;
  }

  lmdb_close(&ldb);

  return common::Error();
}

LmdbRaw::LmdbRaw()
  : CommandHandler(lmdbCommands), lmdb_(NULL) {
}

LmdbRaw::~LmdbRaw() {
  lmdb_close(&lmdb_);
}

const char* LmdbRaw::versionApi() {
  return STRINGIZE(MDB_VERSION_MAJOR) "." STRINGIZE(MDB_VERSION_MINOR) "." STRINGIZE(MDB_VERSION_PATCH);
}

bool LmdbRaw::isConnected() const {
  if (!lmdb_) {
    return false;
  }

  return true;
}

common::Error LmdbRaw::connect() {
  if (isConnected()) {
    return common::Error();
  }

  struct lmdb* context = NULL;
  common::Error er = createConnection(config_, &context);
  if (er && er->isError()) {
    return er;
  }

  lmdb_ = context;
  return common::Error();
}

common::Error LmdbRaw::disconnect() {
  if (!isConnected()) {
      return common::Error();
  }

  lmdb_close(&lmdb_);
  return common::Error();
}

MDB_dbi LmdbRaw::curDb() const {
  if (lmdb_) {
    return lmdb_->dbir;
  }

  return 0;
}

common::Error LmdbRaw::info(const char* args, LmdbServerInfo::Stats* statsout) {
  if (!statsout) {
    NOTREACHED();
    return common::make_error_value("Invalid input argument for command: INFO",
                                    common::ErrorValue::E_ERROR);
  }

  LmdbServerInfo::Stats linfo;
  linfo.file_name = config_.dbname;

  *statsout = linfo;
  return common::Error();
}

common::Error LmdbRaw::dbsize(size_t& size) {
  MDB_cursor *cursor;
  MDB_txn *txn = NULL;
  int rc = mdb_txn_begin(lmdb_->env, NULL, MDB_RDONLY, &txn);
  if (rc == LMDB_OK) {
    rc = mdb_cursor_open(txn, lmdb_->dbir, &cursor);
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

  size = sz;

  return common::Error();
}

common::Error LmdbRaw::get(const std::string& key, std::string* ret_val) {
  MDB_val mkey;
  mkey.mv_size = key.size();
  mkey.mv_data = (void*)key.c_str();
  MDB_val mval;

  MDB_txn *txn = NULL;
  int rc = mdb_txn_begin(lmdb_->env, NULL, MDB_RDONLY, &txn);
  if (rc == LMDB_OK) {
    rc = mdb_get(txn, lmdb_->dbir, &mkey, &mval);
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

common::Error LmdbRaw::put(const std::string& key, const std::string& value) {
  MDB_val mkey;
  mkey.mv_size = key.size();
  mkey.mv_data = (void*)key.c_str();
  MDB_val mval;
  mval.mv_size = value.size();
  mval.mv_data = (void*)value.c_str();

  MDB_txn *txn = NULL;
  int rc = mdb_txn_begin(lmdb_->env, NULL, 0, &txn);
  if (rc == LMDB_OK) {
    rc = mdb_put(txn, lmdb_->dbir, &mkey, &mval, 0);
    if (rc == LMDB_OK) {
        rc = mdb_txn_commit(txn);
    } else {
        mdb_txn_abort(txn);
    }
  }

  if (rc != LMDB_OK) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "put function error: %s", mdb_strerror(rc));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error LmdbRaw::del(const std::string& key) {
  MDB_val mkey;
  mkey.mv_size = key.size();
  mkey.mv_data = (void*)key.c_str();

  MDB_txn *txn = NULL;
  int rc = mdb_txn_begin(lmdb_->env, NULL, 0, &txn);
  if (rc == LMDB_OK) {
    rc = mdb_del(txn, lmdb_->dbir, &mkey, NULL);
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

common::Error LmdbRaw::keys(const std::string &key_start, const std::string &key_end, uint64_t limit,
                   std::vector<std::string> *ret) {
  MDB_cursor *cursor;
  MDB_txn *txn = NULL;
  int rc = mdb_txn_begin(lmdb_->env, NULL, MDB_RDONLY, &txn);
  if (rc == LMDB_OK) {
    rc = mdb_cursor_open(txn, lmdb_->dbir, &cursor);
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

common::Error info(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  LmdbRaw* mdb = static_cast<LmdbRaw*>(handler);
  LmdbServerInfo::Stats statsout;
  common::Error er = mdb->info(argc == 1 ? argv[0] : NULL, &statsout);
  if (!er) {
    common::StringValue *val = common::Value::createStringValue(LmdbServerInfo(statsout).toString());
    FastoObject* child = new FastoObject(out, val, mdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error dbsize(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  LmdbRaw* mdb = static_cast<LmdbRaw*>(handler);
  size_t ret = 0;
  common::Error er = mdb->dbsize(ret);
  if (!er) {
    common::FundamentalValue *val = common::Value::createUIntegerValue(ret);
    FastoObject* child = new FastoObject(out, val, mdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error get(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  LmdbRaw* mdb = static_cast<LmdbRaw*>(handler);
  std::string ret;
  common::Error er = mdb->get(argv[0], &ret);
  if (!er) {
    common::StringValue *val = common::Value::createStringValue(ret);
    FastoObject* child = new FastoObject(out, val, mdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error put(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  LmdbRaw* mdb = static_cast<LmdbRaw*>(handler);
  common::Error er = mdb->put(argv[0], argv[1]);
  if (!er) {
    common::StringValue *val = common::Value::createStringValue("STORED");
    FastoObject* child = new FastoObject(out, val, mdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error del(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  LmdbRaw* mdb = static_cast<LmdbRaw*>(handler);
  common::Error er = mdb->del(argv[0]);
  if (!er) {
    common::StringValue *val = common::Value::createStringValue("DELETED");
    FastoObject* child = new FastoObject(out, val, mdb->config_.delimiter);
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
      common::StringValue *val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, mdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

}  // namespace lmdb
}  // namespace fastonosql
