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

#include "core/leveldb/leveldb_driver.h"

#include <leveldb/db.h>

#include <vector>
#include <string>

#include "common/sprintf.h"
#include "common/utils.h"

#include "core/command_logger.h"

#define INFO_REQUEST "INFO"
#define GET_KEY_PATTERN_1ARGS_S "GET %s"
#define SET_KEY_PATTERN_2ARGS_SS "SET %s %s"

#define GET_KEYS_PATTERN_1ARGS_I "KEYS a z %d"
#define DELETE_KEY_PATTERN_1ARGS_S "DEL %s"

namespace fastonosql {
namespace core {
namespace leveldb {

LeveldbDriver::LeveldbDriver(IConnectionSettingsBaseSPtr settings)
  : IDriverLocal(settings), impl_(new LeveldbRaw) {
  CHECK(type() == LEVELDB);
}

LeveldbDriver::~LeveldbDriver() {
  delete impl_;
}

bool LeveldbDriver::isConnected() const {
  return impl_->isConnected();
}

bool LeveldbDriver::isAuthenticated() const {
  return impl_->isConnected();
}

// ============== commands =============//
common::Error LeveldbDriver::commandDeleteImpl(CommandDeleteKey* command,
                                               std::string* cmdstring) const {
  if (!command || !cmdstring) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  NDbKValue key = command->key();
  *cmdstring = common::MemSPrintf(DELETE_KEY_PATTERN_1ARGS_S, key.keyString());
  return common::Error();
}

common::Error LeveldbDriver::commandLoadImpl(CommandLoadKey* command,
                                             std::string* cmdstring) const {
  if (!command || !cmdstring) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  NDbKValue key = command->key();
  *cmdstring = common::MemSPrintf(GET_KEY_PATTERN_1ARGS_S, key.keyString());
  return common::Error();
}

common::Error LeveldbDriver::commandCreateImpl(CommandCreateKey* command,
                                               std::string* cmdstring) const {
  if (!command || !cmdstring) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  NDbKValue key = command->key();
  NValue val = command->value();
  common::Value* rval = val.get();
  std::string key_str = key.keyString();
  std::string value_str = common::convertToString(rval, " ");
  *cmdstring = common::MemSPrintf(SET_KEY_PATTERN_2ARGS_SS, key_str, value_str);
  return common::Error();
}

common::Error LeveldbDriver::commandChangeTTLImpl(CommandChangeTTL* command,
                                                  std::string* cmdstring) const {
  if (!command || !cmdstring) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  std::string errorMsg = common::MemSPrintf("Sorry, but now " PROJECT_NAME_TITLE " not supported change ttl command for %s.",
                                          common::convertToString(type()));
  return common::make_error_value(errorMsg, common::ErrorValue::E_ERROR);
}

// ============== commands =============//

std::string LeveldbDriver::path() const {
  LeveldbConfig config = impl_->config();
  return config.dbname;
}

std::string LeveldbDriver::outputDelemitr() const {
  return impl_->delimiter();
}

void LeveldbDriver::initImpl() {
}

void LeveldbDriver::clearImpl() {
}

common::Error LeveldbDriver::executeImpl(int argc, char** argv, FastoObject* out) {
  return impl_->execute(argc, argv, out);
}

common::Error LeveldbDriver::serverInfo(IServerInfo** info) {
  LOG_COMMAND(type(), Command(INFO_REQUEST, common::Value::C_INNER));
  LeveldbServerInfo::Stats cm;
  common::Error err = impl_->info(nullptr, &cm);
  if (!err) {
    *info = new LeveldbServerInfo(cm);
  }

  return err;
}

common::Error LeveldbDriver::serverDiscoveryInfo(ServerDiscoveryInfo** dinfo, IServerInfo** sinfo,
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

common::Error LeveldbDriver::currentDataBaseInfo(IDataBaseInfo** info) {
  if (!info) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  size_t dbsize = 0;
  impl_->dbsize(&dbsize);
  *info = new LeveldbDataBaseInfo("0", true, dbsize);
  return common::Error();
}

void LeveldbDriver::handleConnectEvent(events::ConnectRequestEvent* ev) {
  QObject* sender = ev->sender();
  notifyProgress(sender, 0);
  events::ConnectResponceEvent::value_type res(ev->value());
  LeveldbConnectionSettings* set = dynamic_cast<LeveldbConnectionSettings*>(settings_.get());
  if (set) {
    notifyProgress(sender, 25);
    common::Error er = impl_->connect(set->info());
    if (er && er->isError()) {
      res.setErrorInfo(er);
    }
    notifyProgress(sender, 75);
  } else {
    NOTREACHED();
  }
  reply(sender, new events::ConnectResponceEvent(this, res));
  notifyProgress(sender, 100);
}

void LeveldbDriver::handleDisconnectEvent(events::DisconnectRequestEvent* ev) {
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

void LeveldbDriver::handleExecuteEvent(events::ExecuteRequestEvent* ev) {
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
      if (inputLine[n] == '\n' || n == length - 1) {
        notifyProgress(sender, step * n);
        char command[128] = {0};
        if (n == length-1) {
          strcpy(command, inputLine + offset);
        } else {
          strncpy(command, inputLine + offset, n - offset);
        }
        offset = n + 1;
        FastoObjectCommand* cmd = createCommand<LeveldbCommand>(obj, command, common::Value::C_USER);
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

void LeveldbDriver::handleCommandRequestEvent(events::CommandRequestEvent* ev) {
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
  FastoObjectCommand* cmd = createCommand<LeveldbCommand>(obj, cmdtext, common::Value::C_INNER);
  notifyProgress(sender, 50);
  er = execute(cmd);
  if (er && er->isError()) {
    res.setErrorInfo(er);
  }
  reply(sender, new events::CommandResponceEvent(this, res));
  notifyProgress(sender, 100);
}

void LeveldbDriver::handleLoadDatabaseContentEvent(events::LoadDatabaseContentRequestEvent* ev) {
  QObject* sender = ev->sender();
  notifyProgress(sender, 0);
  events::LoadDatabaseContentResponceEvent::value_type res(ev->value());
  std::string patternResult = common::MemSPrintf(GET_KEYS_PATTERN_1ARGS_I, res.count_keys);
  FastoObjectIPtr root = FastoObject::createRoot(patternResult);
  notifyProgress(sender, 50);
  FastoObjectCommand* cmd = createCommand<LeveldbCommand>(root, patternResult,
                                                          common::Value::C_INNER);
  common::Error er = execute(cmd);
  if (er && er->isError()) {
    res.setErrorInfo(er);
  } else {
    FastoObject::child_container_t rchildrens = cmd->childrens();
    if (rchildrens.size()) {
      DCHECK_EQ(rchildrens.size(), 1);
      FastoObjectArray* array = dynamic_cast<FastoObjectArray*>(rchildrens[0]);
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

void LeveldbDriver::handleClearDatabaseEvent(events::ClearDatabaseRequestEvent* ev) {
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

void LeveldbDriver::handleProcessCommandLineArgs(events::ProcessConfigArgsRequestEvent* ev) {
}

IServerInfoSPtr LeveldbDriver::makeServerInfoFromString(const std::string& val) {
  IServerInfoSPtr res(makeLeveldbServerInfo(val));
  return res;
}

}  // namespace leveldb
}  // namespace core
}  // namespace fastonosql
