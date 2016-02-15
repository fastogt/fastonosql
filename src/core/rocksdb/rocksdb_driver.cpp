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

#include "core/rocksdb/rocksdb_driver.h"

#include <vector>
#include <string>

#include <rocksdb/db.h>

#include "common/sprintf.h"
#include "common/utils.h"
#include "fasto/qt/logger.h"

#include "core/command_logger.h"

#define INFO_REQUEST "INFO"
#define GET_KEY_PATTERN_1ARGS_S "GET %s"
#define SET_KEY_PATTERN_2ARGS_SS "PUT %s %s"

#define GET_KEYS_PATTERN_1ARGS_I "KEYS a z %d"
#define DELETE_KEY_PATTERN_1ARGS_S "DEL %s"
#define GET_SERVER_TYPE ""

namespace fastonosql {
namespace rocksdb {

RocksdbDriver::RocksdbDriver(IConnectionSettingsBaseSPtr settings)
  : IDriver(settings, ROCKSDB), impl_(new RocksdbRaw) {
}

RocksdbDriver::~RocksdbDriver() {
  interrupt();
  stop();
  delete impl_;
}

bool RocksdbDriver::isConnected() const {
  return impl_->isConnected();
}

bool RocksdbDriver::isAuthenticated() const {
  return impl_->isConnected();
}

// ============== commands =============//
common::Error RocksdbDriver::commandDeleteImpl(CommandDeleteKey* command,
                                               std::string* cmdstring) const {
  if (!command || !cmdstring) {
    return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
  }

  char patternResult[1024] = {0};
  NDbKValue key = command->key();
  common::SNPrintf(patternResult, sizeof(patternResult),
                   DELETE_KEY_PATTERN_1ARGS_S, key.keyString());

  *cmdstring = patternResult;
  return common::Error();
}

common::Error RocksdbDriver::commandLoadImpl(CommandLoadKey* command,
                                             std::string* cmdstring) const {
  if (!command || !cmdstring) {
    return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
  }

  char patternResult[1024] = {0};
  NDbKValue key = command->key();
  common::SNPrintf(patternResult, sizeof(patternResult), GET_KEY_PATTERN_1ARGS_S, key.keyString());

  *cmdstring = patternResult;
  return common::Error();
}

common::Error RocksdbDriver::commandCreateImpl(CommandCreateKey* command,
                                               std::string* cmdstring) const {
  if (!command || !cmdstring) {
    return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
  }

  char patternResult[1024] = {0};
  NDbKValue key = command->key();
  NValue val = command->value();
  common::Value* rval = val.get();
  std::string key_str = key.keyString();
  std::string value_str = common::convertToString(rval, " ");
  common::SNPrintf(patternResult, sizeof(patternResult), SET_KEY_PATTERN_2ARGS_SS,
                   key_str, value_str);

  *cmdstring = patternResult;
  return common::Error();
}

common::Error RocksdbDriver::commandChangeTTLImpl(CommandChangeTTL* command,
                                                  std::string* cmdstring) const {
  if (!command || !cmdstring) {
    return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
  }

  char errorMsg[1024] = {0};
  common::SNPrintf(errorMsg, sizeof(errorMsg), "Sorry, but now " PROJECT_NAME_TITLE " not supported change ttl command for %s.", common::convertToString(connectionType()));
  return common::make_error_value(errorMsg, common::ErrorValue::E_ERROR);
}

// ============== commands =============//

common::net::hostAndPort RocksdbDriver::address() const {
  return common::net::hostAndPort();
}

std::string RocksdbDriver::outputDelemitr() const {
  return impl_->config_.delimiter;
}

void RocksdbDriver::initImpl() {
}

void RocksdbDriver::clearImpl() {
}

common::Error RocksdbDriver::executeImpl(int argc, char **argv, FastoObject* out) {
  return impl_->execute(argc, argv, out);
}

common::Error RocksdbDriver::serverInfo(ServerInfo **info) {
  LOG_COMMAND(Command(INFO_REQUEST, common::Value::C_INNER));
  RocksdbServerInfo::Stats cm;
  common::Error err = impl_->info(NULL, cm);
  if (!err) {
    *info = new RocksdbServerInfo(cm);
  }

  return err;
}

common::Error RocksdbDriver::serverDiscoveryInfo(ServerInfo **sinfo,
                                                 ServerDiscoveryInfo **dinfo,
                                                 IDataBaseInfo** dbinfo) {
  ServerInfo *lsinfo = NULL;
  common::Error er = serverInfo(&lsinfo);
  if (er && er->isError()) {
      return er;
  }

  FastoObjectIPtr root = FastoObject::createRoot(GET_SERVER_TYPE);
  FastoObjectCommand* cmd = createCommand<RocksdbCommand>(root,
                                                          GET_SERVER_TYPE, common::Value::C_INNER);
  er = execute(cmd);

  if (!er) {
    FastoObject::child_container_type ch = root->childrens();
    if (ch.size()) {
      // *dinfo = makeOwnRedisDiscoveryInfo(ch[0]);
    }
  }

  IDataBaseInfo* ldbinfo = NULL;
  er = currentDataBaseInfo(&ldbinfo);
  if (er && er->isError()) {
    delete lsinfo;
    return er;
  }

  *sinfo = lsinfo;
  *dbinfo = ldbinfo;
  return er;
}

common::Error RocksdbDriver::currentDataBaseInfo(IDataBaseInfo** info) {
  std::string name = impl_->currentDbName();
  size_t size = 0;
  impl_->dbsize(size);
  *info = new RocksdbDataBaseInfo(name, true, size);
  return common::Error();
}

void RocksdbDriver::handleConnectEvent(events::ConnectRequestEvent *ev) {
  QObject *sender = ev->sender();
  notifyProgress(sender, 0);
      events::ConnectResponceEvent::value_type res(ev->value());
      RocksdbConnectionSettings *set = dynamic_cast<RocksdbConnectionSettings*>(settings_.get());
      if (set) {
        impl_->config_ = set->info();
  notifyProgress(sender, 25);
        common::Error er = impl_->connect();
        if (er && er->isError()) {
          res.setErrorInfo(er);
        }
  notifyProgress(sender, 75);
      }
      reply(sender, new events::ConnectResponceEvent(this, res));
  notifyProgress(sender, 100);
}

void RocksdbDriver::handleDisconnectEvent(events::DisconnectRequestEvent* ev) {
  QObject *sender = ev->sender();
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

void RocksdbDriver::handleExecuteEvent(events::ExecuteRequestEvent* ev) {
  QObject *sender = ev->sender();
  notifyProgress(sender, 0);
      events::ExecuteRequestEvent::value_type res(ev->value());
      const char *inputLine = common::utils::c_strornull(res.text);

      common::Error er;
      if (inputLine) {
          size_t length = strlen(inputLine);
          int offset = 0;
          RootLocker lock = make_locker(sender, inputLine);
          FastoObjectIPtr outRoot = lock.root_;
          double step = 100.0f/length;
          for (size_t n = 0; n < length; ++n) {
              if (interrupt_) {
                  er.reset(new common::ErrorValue("Interrupted exec.",
                                                  common::ErrorValue::E_INTERRUPTED));
                  res.setErrorInfo(er);
                  break;
              }
              if (inputLine[n] == '\n' || n == length-1) {
                  notifyProgress(sender, step * n);
                  char command[128] = {0};
                  if (n == length-1) {
                    strcpy(command, inputLine + offset);
                  } else {
                    strncpy(command, inputLine + offset, n - offset);
                  }
                  offset = n + 1;
                  FastoObjectCommand* cmd = createCommand<RocksdbCommand>(outRoot,
                                                                          stableCommand(command),
                                                                          common::Value::C_USER);
                  er = execute(cmd);
                  if (er && er->isError()) {
                      res.setErrorInfo(er);
                      break;
                  }
              }
          }
      } else {
          er.reset(new common::ErrorValue("Empty command line.", common::ErrorValue::E_ERROR));
      }

      if (er) {  // E_INTERRUPTED
        LOG_ERROR(er, true);
      }
  notifyProgress(sender, 100);
}

void RocksdbDriver::handleCommandRequestEvent(events::CommandRequestEvent* ev) {
  QObject *sender = ev->sender();
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
      FastoObjectIPtr root = lock.root_;
      FastoObjectCommand* cmd = createCommand<RocksdbCommand>(root, cmdtext, common::Value::C_INNER);
  notifyProgress(sender, 50);
      er = execute(cmd);
      if (er && er->isError()) {
          res.setErrorInfo(er);
      }
      reply(sender, new events::CommandResponceEvent(this, res));
  notifyProgress(sender, 100);
}

void RocksdbDriver::handleLoadDatabaseInfosEvent(events::LoadDatabasesInfoRequestEvent* ev) {
  QObject *sender = ev->sender();
notifyProgress(sender, 0);
  events::LoadDatabasesInfoResponceEvent::value_type res(ev->value());
notifyProgress(sender, 50);
  IDataBaseInfoSPtr curdb = currentDatabaseInfo();
  CHECK(curdb);
  res.databases.push_back(curdb);
  reply(sender, new events::LoadDatabasesInfoResponceEvent(this, res));
notifyProgress(sender, 100);
}

void RocksdbDriver::handleLoadDatabaseContentEvent(events::LoadDatabaseContentRequestEvent *ev) {
  QObject *sender = ev->sender();
  notifyProgress(sender, 0);
    events::LoadDatabaseContentResponceEvent::value_type res(ev->value());
    char patternResult[1024] = {0};
    common::SNPrintf(patternResult, sizeof(patternResult), GET_KEYS_PATTERN_1ARGS_I, res.count_keys);
    FastoObjectIPtr root = FastoObject::createRoot(patternResult);
  notifyProgress(sender, 50);
    FastoObjectCommand* cmd = createCommand<RocksdbCommand>(root, patternResult,
                                                            common::Value::C_INNER);
    common::Error er = execute(cmd);
    if (er && er->isError()) {
        res.setErrorInfo(er);
    } else {
      FastoObject::child_container_type rchildrens = cmd->childrens();
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
              bool isok = ar->getString(i, &key);
              if (isok) {
                  NKey k(key);
                  NDbKValue ress(k, NValue());
                  res.keys.push_back(ress);
              }
          }
      }
    }
done:
  notifyProgress(sender, 75);
    reply(sender, new events::LoadDatabaseContentResponceEvent(this, res));
  notifyProgress(sender, 100);
}

void RocksdbDriver::handleSetDefaultDatabaseEvent(events::SetDefaultDatabaseRequestEvent* ev) {
  QObject *sender = ev->sender();
  notifyProgress(sender, 0);
      events::SetDefaultDatabaseResponceEvent::value_type res(ev->value());
  notifyProgress(sender, 50);
      reply(sender, new events::SetDefaultDatabaseResponceEvent(this, res));
  notifyProgress(sender, 100);
}

void RocksdbDriver::handleLoadServerInfoEvent(events::ServerInfoRequestEvent* ev) {
  QObject *sender = ev->sender();
  notifyProgress(sender, 0);
      events::ServerInfoResponceEvent::value_type res(ev->value());
  notifyProgress(sender, 50);
      LOG_COMMAND(Command(INFO_REQUEST, common::Value::C_INNER));
      RocksdbServerInfo::Stats cm;
      common::Error err = impl_->info(NULL, cm);
      if (err) {
          res.setErrorInfo(err);
      } else {
          ServerInfoSPtr mem(new RocksdbServerInfo(cm));
          res.setInfo(mem);
      }
  notifyProgress(sender, 75);
      reply(sender, new events::ServerInfoResponceEvent(this, res));
  notifyProgress(sender, 100);
}

void RocksdbDriver::handleProcessCommandLineArgs(events::ProcessConfigArgsRequestEvent* ev) {
}

ServerInfoSPtr RocksdbDriver::makeServerInfoFromString(const std::string& val) {
  ServerInfoSPtr res(makeRocksdbServerInfo(val));
  return res;
}

}  // namespace rocksdb
}  // namespace fastonosql
