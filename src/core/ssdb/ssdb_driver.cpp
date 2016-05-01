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

#include "core/ssdb/ssdb_driver.h"

#include <string>
#include <vector>
#include <map>

#include "common/sprintf.h"
#include "common/utils.h"

#include "core/command_logger.h"

#define INFO_REQUEST "INFO"
#define GET_KEYS_PATTERN_1ARGS_I "KEYS a z %d"
#define DELETE_KEY_PATTERN_1ARGS_S "DEL %s"

#define GET_KEY_PATTERN_1ARGS_S "GET %s"
#define GET_KEY_LIST_PATTERN_1ARGS_S "LRANGE %s 0 -1"
#define GET_KEY_SET_PATTERN_1ARGS_S "SMEMBERS %s"
#define GET_KEY_ZSET_PATTERN_1ARGS_S "ZRANGE %s 0 -1"
#define GET_KEY_HASH_PATTERN_1ARGS_S "HGET %s"

#define SET_KEY_PATTERN_2ARGS_SS "SET %s %s"
#define SET_KEY_LIST_PATTERN_2ARGS_SS "LPUSH %s %s"
#define SET_KEY_SET_PATTERN_2ARGS_SS "SADD %s %s"
#define SET_KEY_ZSET_PATTERN_2ARGS_SS "ZADD %s %s"
#define SET_KEY_HASH_PATTERN_2ARGS_SS "HMSET %s %s"

namespace fastonosql {
namespace core {
namespace ssdb {

SsdbDriver::SsdbDriver(IConnectionSettingsBaseSPtr settings)
  : IDriverRemote(settings), impl_(new SsdbRaw) {
  CHECK(type() == SSDB);
}

SsdbDriver::~SsdbDriver() {
  delete impl_;
}

bool SsdbDriver::isConnected() const {
  return impl_->isConnected();
}

bool SsdbDriver::isAuthenticated() const {
  return impl_->isConnected();
}

// ============== commands =============//
common::Error SsdbDriver::commandDeleteImpl(CommandDeleteKey* command,
                                            std::string* cmdstring) const {
  if (!command || !cmdstring) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  NDbKValue key = command->key();
  *cmdstring = common::MemSPrintf(DELETE_KEY_PATTERN_1ARGS_S, key.keyString());
  return common::Error();
}

common::Error SsdbDriver::commandLoadImpl(CommandLoadKey* command, std::string* cmdstring) const {
  if (!command || !cmdstring) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  std::string patternResult;
  NDbKValue key = command->key();
  common::Value::Type t = key.type();
  if (t == common::Value::TYPE_ARRAY) {
    patternResult = common::MemSPrintf(GET_KEY_LIST_PATTERN_1ARGS_S, key.keyString());
  } else if (t == common::Value::TYPE_SET) {
    patternResult = common::MemSPrintf(GET_KEY_SET_PATTERN_1ARGS_S, key.keyString());
  } else if (t == common::Value::TYPE_ZSET) {
    patternResult = common::MemSPrintf(GET_KEY_ZSET_PATTERN_1ARGS_S, key.keyString());
  } else if (t == common::Value::TYPE_HASH) {
    patternResult = common::MemSPrintf(GET_KEY_HASH_PATTERN_1ARGS_S, key.keyString());
  } else {
    patternResult = common::MemSPrintf(GET_KEY_PATTERN_1ARGS_S, key.keyString());
  }

  *cmdstring = patternResult;
  return common::Error();
}

common::Error SsdbDriver::commandCreateImpl(CommandCreateKey* command,
                                            std::string* cmdstring) const {
  if (!command || !cmdstring) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  std::string patternResult;
  NDbKValue key = command->key();
  NValue val = command->value();
  common::Value* rval = val.get();
  std::string key_str = key.keyString();
  std::string value_str = common::convertToString(rval, " ");
  common::Value::Type t = key.type();
  if (t == common::Value::TYPE_ARRAY) {
    patternResult = common::MemSPrintf(SET_KEY_LIST_PATTERN_2ARGS_SS, key_str, value_str);
  } else if (t == common::Value::TYPE_SET) {
    patternResult = common::MemSPrintf(SET_KEY_SET_PATTERN_2ARGS_SS, key_str, value_str);
  } else if (t == common::Value::TYPE_ZSET) {
    patternResult = common::MemSPrintf(SET_KEY_ZSET_PATTERN_2ARGS_SS, key_str, value_str);
  } else if (t == common::Value::TYPE_HASH) {
    patternResult = common::MemSPrintf(SET_KEY_HASH_PATTERN_2ARGS_SS, key_str, value_str);
  } else {
    patternResult = common::MemSPrintf(SET_KEY_PATTERN_2ARGS_SS, key_str, value_str);
  }

  *cmdstring = patternResult;
  return common::Error();
}

common::Error SsdbDriver::commandChangeTTLImpl(CommandChangeTTL* command,
                                               std::string* cmdstring) const {
  if (!command || !cmdstring) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  std::string errorMsg = common::MemSPrintf("Sorry, but now " PROJECT_NAME_TITLE " not supported change ttl command for %s.",
                                          common::convertToString(type()));
  return common::make_error_value(errorMsg, common::ErrorValue::E_ERROR);
}
// ============== commands =============//

common::net::hostAndPort SsdbDriver::host() const {
  SsdbConfig conf = impl_->config();
  return conf.host;
}

std::string SsdbDriver::nsSeparator() const {
  return impl_->nsSeparator();
}

std::string SsdbDriver::outputDelemitr() const {
  return impl_->delimiter();
}

void SsdbDriver::initImpl() {
}

void SsdbDriver::clearImpl() {
}

common::Error SsdbDriver::executeImpl(int argc, char** argv, FastoObject* out) {
  return impl_->execute(argc, argv, out);
}

common::Error SsdbDriver::serverInfo(IServerInfo** info) {
  LOG_COMMAND(type(), Command(INFO_REQUEST, common::Value::C_INNER));
  SsdbServerInfo::Common cm;
  common::Error err = impl_->info(nullptr, &cm);
  if (!err) {
    *info = new SsdbServerInfo(cm);
  }

  return err;
}

common::Error SsdbDriver::serverDiscoveryClusterInfo(ServerDiscoveryClusterInfo** dinfo, IServerInfo** sinfo,
                                              IDataBaseInfo** dbinfo) {
  UNUSED(dinfo);

  IServerInfo* lsinfo = nullptr;
  common::Error er = serverInfo(&lsinfo);
  if (er && er->isError()) {
    return er;
  }

  IDataBaseInfo* ldbinfo = nullptr;
  er = currentDataBaseInfo(&ldbinfo);
  if (er && er->isError()) {
    delete lsinfo;
    return er;
  }

  *sinfo = lsinfo;
  *dbinfo = ldbinfo;
  return er;
}

common::Error SsdbDriver::currentDataBaseInfo(IDataBaseInfo** info) {
  if (!info) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  size_t dbsize = 0;
  impl_->dbsize(&dbsize);
  SsdbDataBaseInfo* sinfo = new SsdbDataBaseInfo("0", true, dbsize);
  *info = sinfo;
  return common::Error();
}

void SsdbDriver::handleConnectEvent(events::ConnectRequestEvent* ev) {
  QObject* sender = ev->sender();
  notifyProgress(sender, 0);
  events::ConnectResponceEvent::value_type res(ev->value());
  SsdbConnectionSettings* set = dynamic_cast<SsdbConnectionSettings*>(settings_.get());  // +
  CHECK(set);
  notifyProgress(sender, 25);
  common::Error er = impl_->connect(set->info());
  if (er && er->isError()) {
    res.setErrorInfo(er);
  }
  notifyProgress(sender, 75);
  reply(sender, new events::ConnectResponceEvent(this, res));
  notifyProgress(sender, 100);
}

void SsdbDriver::handleDisconnectEvent(events::DisconnectRequestEvent* ev) {
  QObject* sender = ev->sender();
  notifyProgress(sender, 0);
  events::DisconnectResponceEvent::value_type res(ev->value());
  notifyProgress(sender, 50);

  common::Error er = impl_->disconnect();
  if (er && er->isError()) {
    res.setErrorInfo(er);
  }

  reply(sender, new events::DisconnectResponceEvent(this, res));
  notifyProgress(sender, 100);
}

void SsdbDriver::handleExecuteEvent(events::ExecuteRequestEvent* ev) {
  QObject* sender = ev->sender();
  notifyProgress(sender, 0);
  events::ExecuteResponceEvent::value_type res(ev->value());
  const char* inputLine = common::utils::c_strornull(res.text);

  common::Error er;
  if (inputLine) {
    size_t length = strlen(inputLine);
    int offset = 0;
    RootLocker lock = make_locker(sender, inputLine);
    FastoObjectIPtr obj = lock.root();
    double step = 100.0f/length;
    for (size_t n = 0; n < length; ++n) {
      if (isInterrupted()) {
        er.reset(new common::ErrorValue("Interrupted exec.", common::ErrorValue::E_INTERRUPTED));
        res.setErrorInfo(er);
        break;
      }
      if (inputLine[n] == '\n' || n == length-1) {
        notifyProgress(sender, step * n);
        char command[128] = {0};
        if (n == length - 1) {
          strcpy(command, inputLine + offset);
        } else {
          strncpy(command, inputLine + offset, n - offset);
        }
        offset = n + 1;
        FastoObjectCommand* cmd = createCommand<SsdbCommand>(obj, command, common::Value::C_USER);
        er = execute(cmd);
        if (er && er->isError()) {
          res.setErrorInfo(er);
          break;
        }
      }
    }
  } else {
    er.reset(new common::ErrorValue("Empty command line.", common::ErrorValue::E_ERROR));
    res.setErrorInfo(er);
  }

  reply(sender, new events::ExecuteResponceEvent(this, res));
  notifyProgress(sender, 100);
}

void SsdbDriver::handleCommandRequestEvent(events::CommandRequestEvent* ev) {
  QObject* sender = ev->sender();
  notifyProgress(sender, 0);
    events::CommandResponceEvent::value_type res(ev->value());
    std::string cmdtext;
    common::Error er = commandByType(res.cmd, &cmdtext);
    if (er && er->isError()) {
      res.setErrorInfo(er);
      reply(sender, new events::CommandResponceEvent(this, res));
      notifyProgress(sender, 100);
      return;
    }

    RootLocker lock = make_locker(sender, cmdtext);
    FastoObjectIPtr obj = lock.root();
    FastoObjectCommand* cmd = createCommand<SsdbCommand>(obj, cmdtext, common::Value::C_INNER);
  notifyProgress(sender, 50);
    er = execute(cmd);
    if (er && er->isError()) {
      res.setErrorInfo(er);
    }
    reply(sender, new events::CommandResponceEvent(this, res));
  notifyProgress(sender, 100);
}

void SsdbDriver::handleLoadDatabaseContentEvent(events::LoadDatabaseContentRequestEvent* ev) {
  QObject* sender = ev->sender();
  notifyProgress(sender, 0);
    events::LoadDatabaseContentResponceEvent::value_type res(ev->value());
    std::string patternResult = common::MemSPrintf(GET_KEYS_PATTERN_1ARGS_I, res.count_keys);
    FastoObjectIPtr root = FastoObject::createRoot(patternResult);
  notifyProgress(sender, 50);
    FastoObjectCommand* cmd = createCommand<SsdbCommand>(root, patternResult,
                                                         common::Value::C_INNER);
    common::Error er = execute(cmd);
    if (er && er->isError()) {
      res.setErrorInfo(er);
    } else {
      FastoObject::child_container_t rchildrens = cmd->childrens();
      if (rchildrens.size()) {
        CHECK_EQ(rchildrens.size(), 1);
        FastoObjectArray* array = dynamic_cast<FastoObjectArray*>(rchildrens[0]);  // +
        if (!array) {
          goto done;
        }
        common::ArrayValue* ar = array->array();
        if (!ar) {
          goto done;
        }

        for (size_t i = 0; i < ar->size(); ++i) {
          std::string key;
          if (ar->getString(i, &key)) {
            NKey k(key);
            NDbKValue ress(k, NValue());
            res.keys.push_back(ress);
          }
        }

        impl_->dbsize(&res.dbsize);
      }
    }
done:
  notifyProgress(sender, 75);
    reply(sender, new events::LoadDatabaseContentResponceEvent(this, res));
  notifyProgress(sender, 100);
}

void SsdbDriver::handleClearDatabaseEvent(events::ClearDatabaseRequestEvent* ev) {
  QObject* sender = ev->sender();
  notifyProgress(sender, 0);
  events::ClearDatabaseResponceEvent::value_type res(ev->value());
  notifyProgress(sender, 50);
  common::Error er = impl_->flushdb();
  if (er && er->isError()) {
    res.setErrorInfo(er);
  }
  notifyProgress(sender, 75);
  reply(sender, new events::ClearDatabaseResponceEvent(this, res));
  notifyProgress(sender, 100);
}

void SsdbDriver::handleProcessCommandLineArgs(events::ProcessConfigArgsRequestEvent* ev) {
}

IServerInfoSPtr SsdbDriver::makeServerInfoFromString(const std::string& val) {
  IServerInfoSPtr res(makeSsdbServerInfo(val));
  return res;
}

}
}  // namespace core
}  // namespace fastonosql
