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

#include "core/ssdb/ssdb_raw.h"

#include "common/sprintf.h"

namespace fastonosql {
namespace ssdb {

namespace {

common::Error createConnection(const SsdbConfig& config, const SSHInfo& sinfo,
                               ::ssdb::Client** context) {
  if (!context) {
    return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
  }

  DCHECK(*context == nullptr);
  UNUSED(sinfo);
  ::ssdb::Client *lcontext = ::ssdb::Client::connect(config.host.host, config.host.port);
  if (!lcontext) {
    return common::make_error_value("Fail connect to server!", common::ErrorValue::E_ERROR);
  }

  *context = lcontext;

  return common::Error();
}

common::Error createConnection(SsdbConnectionSettings* settings, ::ssdb::Client** context) {
  if (!settings) {
    return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
  }

  SsdbConfig config = settings->info();
  SSHInfo sinfo = settings->sshInfo();
  return createConnection(config, sinfo, context);
}

}  // namespace

common::Error testConnection(SsdbConnectionSettings* settings) {
  if (!settings) {
    return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
  }

  ::ssdb::Client* ssdb = nullptr;
  common::Error er = createConnection(settings, &ssdb);
  if (er && er->isError()) {
      return er;
  }

  delete ssdb;

  return common::Error();
}


SsdbRaw::SsdbRaw()
  : ssdb_(nullptr), CommandHandler(ssdbCommands) {
}

SsdbRaw::~SsdbRaw() {
  destroy(&ssdb_);
}

const char* SsdbRaw::versionApi() {
  return "1.9.2";
}

bool SsdbRaw::isConnected() const {
  if (!ssdb_) {
    return false;
  }

  return true;
}

common::Error SsdbRaw::connect() {
  if (isConnected()) {
    return common::Error();
  }

  ::ssdb::Client* context = nullptr;
  common::Error er = createConnection(config_, sinfo_, &context);
  if (er && er->isError()) {
    return er;
  }

  ssdb_ = context;
  return common::Error();
}

common::Error SsdbRaw::disconnect() {
  if (!isConnected()) {
    return common::Error();
  }

  destroy(&ssdb_);
  return common::Error();
}

common::Error SsdbRaw::info(const char* args, SsdbServerInfo::Common *statsout) {
  if (!statsout) {
    return common::make_error_value("Invalid input argument for command: INFO",
                                    common::ErrorValue::E_ERROR);
  }

  std::vector<std::string> ret;
  auto st = ssdb_->info(args ? args : std::string(), &ret);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "info function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  SsdbServerInfo::Common lstatsout;
  for (size_t i = 0; i < ret.size(); i += 2) {
    if (ret[i] == SSDB_VERSION_LABEL) {
      lstatsout.version = ret[i + 1];
    } else if (ret[i] == SSDB_LINKS_LABEL) {
      lstatsout.links = common::convertFromString<uint32_t>(ret[i + 1]);
    } else if (ret[i] == SSDB_TOTAL_CALLS_LABEL) {
      lstatsout.total_calls = common::convertFromString<uint32_t>(ret[i + 1]);
    } else if (ret[i] == SSDB_DBSIZE_LABEL) {
      lstatsout.dbsize = common::convertFromString<uint32_t>(ret[i + 1]);
    } else if (ret[i] == SSDB_BINLOGS_LABEL) {
      lstatsout.binlogs = ret[i + 1];
    }
  }

  *statsout = lstatsout;
  return common::Error();
}

common::Error SsdbRaw::dbsize(size_t* size) {
  if (!size) {
    return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
  }

  int64_t sz = 0;
  auto st = ssdb_->dbsize(&sz);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "Couldn't determine DBSIZE error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  *size = sz;
  return common::Error();
}

common::Error SsdbRaw::auth(const std::string& password) {
  auto st = ssdb_->auth(password);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "password function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::get(const std::string& key, std::string* ret_val) {
  auto st = ssdb_->get(key, ret_val);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "get function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::set(const std::string& key, const std::string& value) {
  auto st = ssdb_->set(key, value);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "set function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::setx(const std::string& key, const std::string& value, int ttl) {
  auto st = ssdb_->setx(key, value, ttl);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "setx function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::del(const std::string& key) {
  auto st = ssdb_->del(key);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "del function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::incr(const std::string& key, int64_t incrby, int64_t *ret) {
  auto st = ssdb_->incr(key, incrby, ret);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "Incr function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::keys(const std::string &key_start, const std::string &key_end,
                   uint64_t limit, std::vector<std::string> *ret) {
  auto st = ssdb_->keys(key_start, key_end, limit, ret);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "Keys function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::scan(const std::string &key_start, const std::string &key_end,
                   uint64_t limit, std::vector<std::string> *ret) {
  auto st = ssdb_->scan(key_start, key_end, limit, ret);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "scan function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::rscan(const std::string &key_start, const std::string &key_end,
                    uint64_t limit, std::vector<std::string> *ret) {
  auto st = ssdb_->rscan(key_start, key_end, limit, ret);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "rscan function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::multi_get(const std::vector<std::string>& keys, std::vector<std::string> *ret) {
  auto st = ssdb_->multi_get(keys, ret);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "multi_get function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::multi_set(const std::map<std::string, std::string> &kvs) {
  auto st = ssdb_->multi_set(kvs);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "multi_set function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::multi_del(const std::vector<std::string>& keys) {
  auto st = ssdb_->multi_del(keys);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "multi_del function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::hget(const std::string& name, const std::string& key, std::string *val) {
  auto st = ssdb_->hget(name, key, val);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "hget function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::hset(const std::string& name, const std::string& key, const std::string& val) {
  auto st = ssdb_->hset(name, key, val);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "hset function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error SsdbRaw::hdel(const std::string& name, const std::string& key) {
  auto st = ssdb_->hdel(name, key);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "hdel function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::hincr(const std::string &name, const std::string &key,
                    int64_t incrby, int64_t *ret) {
  auto st = ssdb_->hincr(name, key, incrby, ret);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "hincr function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::hsize(const std::string &name, int64_t *ret) {
  auto st = ssdb_->hsize(name, ret);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "hset function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::hclear(const std::string &name, int64_t *ret) {
  auto st = ssdb_->hclear(name, ret);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "hclear function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::hkeys(const std::string &name, const std::string &key_start,
                    const std::string &key_end, uint64_t limit, std::vector<std::string> *ret) {
  auto st = ssdb_->hkeys(name, key_start, key_end, limit, ret);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "hkeys function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::hscan(const std::string &name, const std::string &key_start,
                    const std::string &key_end, uint64_t limit, std::vector<std::string> *ret) {
  auto st = ssdb_->hscan(name, key_start, key_end, limit, ret);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "hscan function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::hrscan(const std::string &name, const std::string &key_start,
                     const std::string &key_end, uint64_t limit, std::vector<std::string> *ret) {
  auto st = ssdb_->hrscan(name, key_start, key_end, limit, ret);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "hrscan function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::multi_hget(const std::string &name, const std::vector<std::string> &keys,
                         std::vector<std::string> *ret) {
  auto st = ssdb_->multi_hget(name, keys, ret);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "hrscan function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::multi_hset(const std::string &name,
                         const std::map<std::string, std::string> &keys) {
  auto st = ssdb_->multi_hset(name, keys);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "multi_hset function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::zget(const std::string &name, const std::string &key, int64_t *ret) {
  auto st = ssdb_->zget(name, key, ret);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "zget function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::zset(const std::string &name, const std::string &key, int64_t score) {
  auto st = ssdb_->zset(name, key, score);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "zset function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::zdel(const std::string &name, const std::string &key) {
  auto st = ssdb_->zdel(name, key);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "Zdel function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::zincr(const std::string &name, const std::string &key, int64_t incrby, int64_t *ret) {
  auto st = ssdb_->zincr(name, key, incrby, ret);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "Zincr function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::zsize(const std::string &name, int64_t *ret) {
  auto st = ssdb_->zsize(name, ret);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "zsize function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::zclear(const std::string &name, int64_t *ret) {
  auto st = ssdb_->zclear(name, ret);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "zclear function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::zrank(const std::string &name, const std::string &key, int64_t *ret) {
  auto st = ssdb_->zrank(name, key, ret);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "zrank function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::zrrank(const std::string &name, const std::string &key, int64_t *ret) {
  auto st = ssdb_->zrrank(name, key, ret);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "zrrank function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::zrange(const std::string &name,
        uint64_t offset, uint64_t limit,
        std::vector<std::string> *ret) {
  auto st = ssdb_->zrange(name, offset, limit, ret);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "zrange function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::zrrange(const std::string &name,
        uint64_t offset, uint64_t limit,
        std::vector<std::string> *ret) {
  auto st = ssdb_->zrrange(name, offset, limit, ret);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "zrrange function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::zkeys(const std::string &name, const std::string &key_start,
    int64_t *score_start, int64_t *score_end,
    uint64_t limit, std::vector<std::string> *ret) {
  auto st = ssdb_->zkeys(name, key_start, score_start, score_end, limit, ret);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "zkeys function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::zscan(const std::string &name, const std::string &key_start,
    int64_t *score_start, int64_t *score_end,
    uint64_t limit, std::vector<std::string> *ret) {
  auto st = ssdb_->zscan(name, key_start, score_start, score_end, limit, ret);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "zscan function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::zrscan(const std::string &name, const std::string &key_start,
    int64_t *score_start, int64_t *score_end,
    uint64_t limit, std::vector<std::string> *ret) {
  auto st = ssdb_->zrscan(name, key_start, score_start, score_end, limit, ret);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "zrscan function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::multi_zget(const std::string &name, const std::vector<std::string> &keys,
    std::vector<std::string> *ret) {
  auto st = ssdb_->multi_zget(name, keys, ret);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "multi_zget function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::multi_zset(const std::string &name, const std::map<std::string, int64_t> &kss) {
  auto st = ssdb_->multi_zset(name, kss);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "multi_zset function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::multi_zdel(const std::string &name, const std::vector<std::string> &keys) {
  auto st = ssdb_->multi_zdel(name, keys);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "multi_zdel function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::qpush(const std::string &name, const std::string &item) {
  auto st = ssdb_->qpush(name, item);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "qpush function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::qpop(const std::string &name, std::string *item) {
  auto st = ssdb_->qpop(name, item);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "qpop function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::qslice(const std::string &name, int64_t begin, int64_t end,
                     std::vector<std::string> *ret) {
  auto st = ssdb_->qslice(name, begin, end, ret);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "qslice function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::qclear(const std::string &name, int64_t *ret) {
  auto st = ssdb_->qclear(name, ret);
  if (st.error()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "qclear function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error SsdbRaw::help(int argc, char** argv) {
  return notSupported("HELP");
}

common::Error info(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  SsdbServerInfo::Common statsout;
  common::Error er = ssdb->info(argc == 1 ? argv[0] : 0, &statsout);
  if (!er) {
    SsdbServerInfo sinf(statsout);
    common::StringValue *val = common::Value::createStringValue(sinf.toString());
    FastoObject* child = new FastoObject(out, val, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error dbsize(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  size_t dbsize = 0;
  common::Error er = ssdb->dbsize(&dbsize);
  if (!er) {
    common::FundamentalValue *val = common::Value::createUIntegerValue(dbsize);
    FastoObject* child = new FastoObject(out, val, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error auth(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  common::Error er = ssdb->auth(argv[0]);
  if (!er) {
    common::StringValue *val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error get(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  std::string ret;
  common::Error er = ssdb->get(argv[0], &ret);
  if (!er) {
    common::StringValue *val = common::Value::createStringValue(ret);
    FastoObject* child = new FastoObject(out, val, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error set(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  common::Error er = ssdb->set(argv[0], argv[1]);
  if (!er) {
    common::StringValue *val = common::Value::createStringValue("STORED");
    FastoObject* child = new FastoObject(out, val, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error setx(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  common::Error er = ssdb->setx(argv[0], argv[1], atoi(argv[2]));
  if (!er) {
    common::StringValue *val = common::Value::createStringValue("STORED");
    FastoObject* child = new FastoObject(out, val, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error del(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  common::Error er = ssdb->del(argv[0]);
  if (!er) {
    common::StringValue *val = common::Value::createStringValue("DELETED");
    FastoObject* child = new FastoObject(out, val, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error incr(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  int64_t ret = 0;
  common::Error er = ssdb->incr(argv[0], atoll(argv[1]), &ret);
  if (!er) {
    common::FundamentalValue *val = common::Value::createIntegerValue(ret);
    FastoObject* child = new FastoObject(out, val, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error keys(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  std::vector<std::string> keysout;
  common::Error er = ssdb->keys(argv[0], argv[1], atoll(argv[2]), &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue *val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error scan(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  std::vector<std::string> keysout;
  common::Error er = ssdb->scan(argv[0], argv[1], atoll(argv[2]), &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue *val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error rscan(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  std::vector<std::string> keysout;
  common::Error er = ssdb->rscan(argv[0], argv[1], atoll(argv[2]), &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue *val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error multi_get(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  std::vector<std::string> keysget;
  for (int i = 0; i < argc; ++i) {
    keysget.push_back(argv[i]);
  }

  std::vector<std::string> keysout;
  common::Error er = ssdb->multi_get(keysget, &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue *val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error multi_set(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  std::map<std::string, std::string> keysset;
  for (size_t i = 0; i < argc; i += 2) {
    keysset[argv[i]] = argv[i + 1];
  }

  common::Error er = ssdb->multi_set(keysset);
  if (!er) {
    common::StringValue *val = common::Value::createStringValue("STORED");
    FastoObject* child = new FastoObject(out, val, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error multi_del(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  std::vector<std::string> keysget;
  for (size_t i = 0; i < argc; ++i) {
    keysget.push_back(argv[i]);
  }

  common::Error er = ssdb->multi_del(keysget);
  if (!er) {
    common::StringValue *val = common::Value::createStringValue("DELETED");
    FastoObject* child = new FastoObject(out, val, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error hget(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  std::string ret;
  common::Error er = ssdb->hget(argv[0], argv[1], &ret);
  if (!er) {
    common::StringValue *val = common::Value::createStringValue(ret);
    FastoObject* child = new FastoObject(out, val, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error hset(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  common::Error er = ssdb->hset(argv[0], argv[1], argv[2]);
  if (!er) {
    common::StringValue *val = common::Value::createStringValue("STORED");
    FastoObject* child = new FastoObject(out, val, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error hdel(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  common::Error er = ssdb->hdel(argv[0], argv[1]);
  if (!er) {
     common::StringValue *val = common::Value::createStringValue("DELETED");
     FastoObject* child = new FastoObject(out, val, ssdb->config_.delimiter);
     out->addChildren(child);
  }

  return er;
}

common::Error hincr(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  int64_t res = 0;
  common::Error er = ssdb->hincr(argv[0], argv[1], atoll(argv[2]), &res);
  if (!er) {
    common::FundamentalValue *val = common::Value::createIntegerValue(res);
    FastoObject* child = new FastoObject(out, val, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error hsize(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  int64_t res = 0;
  common::Error er = ssdb->hsize(argv[0], &res);
  if (!er) {
    common::FundamentalValue *val = common::Value::createIntegerValue(res);
    FastoObject* child = new FastoObject(out, val, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error hclear(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  int64_t res = 0;
  common::Error er = ssdb->hclear(argv[0], &res);
  if (!er) {
    common::FundamentalValue *val = common::Value::createIntegerValue(res);
    FastoObject* child = new FastoObject(out, val, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error hkeys(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  std::vector<std::string> keysout;
  common::Error er = ssdb->hkeys(argv[0], argv[1], argv[2], atoll(argv[3]), &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue *val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error hscan(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  std::vector<std::string> keysout;
  common::Error er = ssdb->hscan(argv[0], argv[1], argv[2], atoll(argv[3]), &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue *val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error hrscan(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  std::vector<std::string> keysout;
  common::Error er = ssdb->hrscan(argv[0], argv[1], argv[2], atoll(argv[3]), &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue *val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error multi_hget(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  std::vector<std::string> keysget;
  for (int i = 1; i < argc; ++i) {
    keysget.push_back(argv[i]);
  }

  std::vector<std::string> keysout;
  common::Error er = ssdb->multi_hget(argv[0], keysget, &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue *val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error multi_hset(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  std::map<std::string, std::string> keys;
  for (int i = 1; i < argc; i += 2) {
    keys[argv[i]] = argv[i + 1];
  }

  common::Error er = ssdb->multi_hset(argv[0], keys);
  if (!er) {
    common::StringValue *val = common::Value::createStringValue("STORED");
    FastoObject* child = new FastoObject(out, val, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error zget(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  int64_t ret;
  common::Error er = ssdb->zget(argv[0], argv[1], &ret);
  if (!er) {
    common::FundamentalValue *val = common::Value::createIntegerValue(ret);
    FastoObject* child = new FastoObject(out, val, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error zset(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  common::Error er = ssdb->zset(argv[0], argv[1], atoll(argv[2]));
  if (!er) {
    common::StringValue *val = common::Value::createStringValue("STORED");
    FastoObject* child = new FastoObject(out, val, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error zdel(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  common::Error er = ssdb->zdel(argv[0], argv[1]);
  if (!er) {
      common::StringValue *val = common::Value::createStringValue("DELETED");
      FastoObject* child = new FastoObject(out, val, ssdb->config_.delimiter);
      out->addChildren(child);
  }
  return er;
}

common::Error zincr(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  int64_t ret = 0;
  common::Error er = ssdb->zincr(argv[0], argv[1], atoll(argv[2]), &ret);
  if (!er) {
    common::FundamentalValue *val = common::Value::createIntegerValue(ret);
    FastoObject* child = new FastoObject(out, val, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error zsize(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  int64_t res = 0;
  common::Error er = ssdb->zsize(argv[0], &res);
  if (!er) {
    common::FundamentalValue *val = common::Value::createIntegerValue(res);
    FastoObject* child = new FastoObject(out, val, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error zclear(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  int64_t res = 0;
  common::Error er = ssdb->zclear(argv[0], &res);
  if (!er) {
    common::FundamentalValue *val = common::Value::createIntegerValue(res);
    FastoObject* child = new FastoObject(out, val, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error zrank(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  int64_t res = 0;
  common::Error er = ssdb->zrank(argv[0], argv[1], &res);
  if (!er) {
    common::FundamentalValue *val = common::Value::createIntegerValue(res);
    FastoObject* child = new FastoObject(out, val, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error zrrank(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  int64_t res = 0;
  common::Error er = ssdb->zrrank(argv[0], argv[1], &res);
  if (!er) {
    common::FundamentalValue *val = common::Value::createIntegerValue(res);
    FastoObject* child = new FastoObject(out, val, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error zrange(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  std::vector<std::string> res;
  common::Error er = ssdb->zrange(argv[0], atoll(argv[1]), atoll(argv[2]), &res);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < res.size(); ++i) {
      common::StringValue *val = common::Value::createStringValue(res[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error zrrange(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  std::vector<std::string> res;
  common::Error er = ssdb->zrrange(argv[0], atoll(argv[1]), atoll(argv[2]), &res);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < res.size(); ++i) {
      common::StringValue *val = common::Value::createStringValue(res[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error zkeys(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  std::vector<std::string> res;
  int64_t st = atoll(argv[2]);
  int64_t end = atoll(argv[3]);
  common::Error er = ssdb->zkeys(argv[0], argv[1], &st, &end, atoll(argv[5]), &res);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < res.size(); ++i) {
      common::StringValue *val = common::Value::createStringValue(res[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error zscan(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  std::vector<std::string> res;
  int64_t st = atoll(argv[2]);
  int64_t end = atoll(argv[3]);
  common::Error er = ssdb->zscan(argv[0], argv[1], &st, &end, atoll(argv[4]), &res);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < res.size(); ++i) {
      common::StringValue *val = common::Value::createStringValue(res[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error zrscan(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  std::vector<std::string> res;
  int64_t st = atoll(argv[2]);
  int64_t end = atoll(argv[3]);
  common::Error er = ssdb->zrscan(argv[0], argv[1], &st, &end, atoll(argv[4]), &res);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < res.size(); ++i) {
      common::StringValue *val = common::Value::createStringValue(res[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error multi_zget(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  std::vector<std::string> keysget;
  for (int i = 1; i < argc; ++i) {
    keysget.push_back(argv[i]);
  }

  std::vector<std::string> res;
  common::Error er = ssdb->multi_zget(argv[0], keysget, &res);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < res.size(); ++i) {
      common::StringValue *val = common::Value::createStringValue(res[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error multi_zset(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  std::map<std::string, int64_t> keysget;
  for (int i = 1; i < argc; i += 2) {
    keysget[argv[i]] = atoll(argv[i+1]);
  }

  common::Error er = ssdb->multi_zset(argv[0], keysget);
  if (!er) {
    common::StringValue *val = common::Value::createStringValue("STORED");
    FastoObject* child = new FastoObject(out, val, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error multi_zdel(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  std::vector<std::string> keysget;
  for (int i = 1; i < argc; ++i) {
    keysget.push_back(argv[i]);
  }

  common::Error er = ssdb->multi_zdel(argv[0], keysget);
  if (!er) {
    common::StringValue *val = common::Value::createStringValue("DELETED");
    FastoObject* child = new FastoObject(out, val, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error qpush(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  common::Error er = ssdb->qpush(argv[0], argv[1]);
  if (!er) {
    common::StringValue *val = common::Value::createStringValue("STORED");
    FastoObject* child = new FastoObject(out, val, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error qpop(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  std::string ret;
  common::Error er = ssdb->qpop(argv[0], &ret);
  if (!er) {
    common::StringValue *val = common::Value::createStringValue(ret);
    FastoObject* child = new FastoObject(out, val, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error qslice(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  int64_t begin = atoll(argv[1]);
  int64_t end = atoll(argv[2]);

  std::vector<std::string> keysout;
  common::Error er = ssdb->qslice(argv[0], begin, end, &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue *val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error qclear(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  int64_t res = 0;
  common::Error er = ssdb->qclear(argv[0], &res);
  if (!er) {
      common::FundamentalValue *val = common::Value::createIntegerValue(res);
      FastoObject* child = new FastoObject(out, val, ssdb->config_.delimiter);
      out->addChildren(child);
  }

  return er;
}

common::Error help(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  SsdbRaw* ssdb = static_cast<SsdbRaw*>(handler);
  return ssdb->help(argc - 1, argv + 1);
}

}  // namespace ssdb
}  // namespace fastonosql
