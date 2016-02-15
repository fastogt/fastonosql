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

#include "core/unqlite/unqlite_driver.h"

#include <string>
#include <vector>

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
namespace unqlite {

UnqliteDriver::UnqliteDriver(IConnectionSettingsBaseSPtr settings)
  : IDriver(settings, UNQLITE), impl_(new UnqliteRaw) {
}

UnqliteDriver::~UnqliteDriver() {
  interrupt();
  stop();
  delete impl_;
}

bool UnqliteDriver::isConnected() const {
  return impl_->isConnected();
}

bool UnqliteDriver::isAuthenticated() const {
  return impl_->isConnected();
}

// ============== commands =============//
common::Error UnqliteDriver::commandDeleteImpl(CommandDeleteKey* command,
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

common::Error UnqliteDriver::commandLoadImpl(CommandLoadKey* command,
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

common::Error UnqliteDriver::commandCreateImpl(CommandCreateKey* command,
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
  common::SNPrintf(patternResult, sizeof(patternResult),
                   SET_KEY_PATTERN_2ARGS_SS, key_str, value_str);

  *cmdstring = patternResult;
  return common::Error();
}

common::Error UnqliteDriver::commandChangeTTLImpl(CommandChangeTTL* command,
                                                  std::string* cmdstring) const {
  if (!command || !cmdstring) {
    return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
  }

  char errorMsg[1024] = {0};
  common::SNPrintf(errorMsg, sizeof(errorMsg),
                   "Sorry, but now " PROJECT_NAME_TITLE " not supported change ttl command for %s.",
                   common::convertToString(connectionType()));
  return common::make_error_value(errorMsg, common::ErrorValue::E_ERROR);
}

// ============== commands =============//

common::net::hostAndPort UnqliteDriver::address() const {
  return common::net::hostAndPort();
}

std::string UnqliteDriver::outputDelemitr() const {
  return impl_->config_.delimiter;
}

void UnqliteDriver::initImpl() {
}

void UnqliteDriver::clearImpl() {
}

common::Error UnqliteDriver::executeImpl(int argc, char **argv, FastoObject* out) {
  return impl_->execute(argc, argv, out);
}

common::Error UnqliteDriver::serverInfo(ServerInfo **info) {
  LOG_COMMAND(Command(INFO_REQUEST, common::Value::C_INNER));
  UnqliteServerInfo::Stats cm;
  common::Error err = impl_->info(NULL, &cm);
  if (!err) {
    *info = new UnqliteServerInfo(cm);
  }

  return err;
}

common::Error UnqliteDriver::serverDiscoveryInfo(ServerInfo **sinfo,
                                                 ServerDiscoveryInfo **dinfo,
                                                 IDataBaseInfo** dbinfo) {
  ServerInfo *lsinfo = NULL;
  common::Error err = serverInfo(&lsinfo);
  if (err && err->isError()) {
    return err;
  }

  FastoObjectIPtr root = FastoObject::createRoot(GET_SERVER_TYPE);
  FastoObjectCommand* cmd = createCommand<UnqliteCommand>(root, GET_SERVER_TYPE,
                                                          common::Value::C_INNER);
  err = execute(cmd);

  if (!err) {
    FastoObject::child_container_type ch = root->childrens();
    if (ch.size()) {
      // *dinfo = makeOwnRedisDiscoveryInfo(ch[0]);
    }
  }

  IDataBaseInfo* ldbinfo = NULL;
  err = currentDataBaseInfo(&ldbinfo);
  if (err && err->isError()) {
    delete lsinfo;
    return err;
  }

  *sinfo = lsinfo;
  *dbinfo = ldbinfo;
  return err;
}

common::Error UnqliteDriver::currentDataBaseInfo(IDataBaseInfo** info) {
  size_t size = 0;
  impl_->dbsize(size);
  *info = new UnqliteDataBaseInfo("0", true, size);
  return common::Error();
}

void UnqliteDriver::handleConnectEvent(events::ConnectRequestEvent *ev) {
  QObject *sender = ev->sender();
  notifyProgress(sender, 0);
  events::ConnectResponceEvent::value_type res(ev->value());
  UnqliteConnectionSettings *set = dynamic_cast<UnqliteConnectionSettings*>(settings_.get());
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

void UnqliteDriver::handleDisconnectEvent(events::DisconnectRequestEvent* ev) {
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

void UnqliteDriver::handleExecuteEvent(events::ExecuteRequestEvent* ev) {
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
        er.reset(new common::ErrorValue("Interrupted exec.", common::ErrorValue::E_INTERRUPTED));
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
        FastoObjectCommand* cmd = createCommand<UnqliteCommand>(outRoot, stableCommand(command),
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

void UnqliteDriver::handleCommandRequestEvent(events::CommandRequestEvent* ev) {
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
  FastoObjectCommand* cmd = createCommand<UnqliteCommand>(root, cmdtext, common::Value::C_INNER);
  notifyProgress(sender, 50);
  er = execute(cmd);
  if (er && er->isError()) {
    res.setErrorInfo(er);
  }
  reply(sender, new events::CommandResponceEvent(this, res));
  notifyProgress(sender, 100);
}

void UnqliteDriver::handleLoadDatabaseInfosEvent(events::LoadDatabasesInfoRequestEvent* ev) {
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

void UnqliteDriver::handleLoadDatabaseContentEvent(events::LoadDatabaseContentRequestEvent *ev) {
  QObject *sender = ev->sender();
  notifyProgress(sender, 0);
  events::LoadDatabaseContentResponceEvent::value_type res(ev->value());
  char patternResult[1024] = {0};
  common::SNPrintf(patternResult, sizeof(patternResult), GET_KEYS_PATTERN_1ARGS_I, res.count_keys);
  FastoObjectIPtr root = FastoObject::createRoot(patternResult);
  notifyProgress(sender, 50);
  FastoObjectCommand* cmd = createCommand<UnqliteCommand>(root, patternResult,
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

void UnqliteDriver::handleSetDefaultDatabaseEvent(events::SetDefaultDatabaseRequestEvent* ev) {
  QObject *sender = ev->sender();
  notifyProgress(sender, 0);
  events::SetDefaultDatabaseResponceEvent::value_type res(ev->value());
  notifyProgress(sender, 50);
  reply(sender, new events::SetDefaultDatabaseResponceEvent(this, res));
  notifyProgress(sender, 100);
}

void UnqliteDriver::handleLoadServerInfoEvent(events::ServerInfoRequestEvent* ev) {
  QObject *sender = ev->sender();
  notifyProgress(sender, 0);
  events::ServerInfoResponceEvent::value_type res(ev->value());
  notifyProgress(sender, 50);
  LOG_COMMAND(Command(INFO_REQUEST, common::Value::C_INNER));
  UnqliteServerInfo::Stats cm;
  common::Error err = impl_->info(NULL, &cm);
  if (err) {
    res.setErrorInfo(err);
  } else {
    ServerInfoSPtr mem(new UnqliteServerInfo(cm));
    res.setInfo(mem);
  }
  notifyProgress(sender, 75);
  reply(sender, new events::ServerInfoResponceEvent(this, res));
  notifyProgress(sender, 100);
}

void UnqliteDriver::handleProcessCommandLineArgs(events::ProcessConfigArgsRequestEvent* ev) {
}

ServerInfoSPtr UnqliteDriver::makeServerInfoFromString(const std::string& val) {
  ServerInfoSPtr res(makeUnqliteServerInfo(val));
  return res;
}

}  // namespace unqlite
}  // namespace fastonosql
