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

#include "core/memcached/driver.h"

#include <string>

#include "common/utils.h"
#include "common/sprintf.h"

#include "core/command_logger.h"
#include "core/memcached/database.h"
#include "core/memcached/command.h"

#define INFO_REQUEST "STATS"
#define GET_KEYS "STATS ITEMS"

#define DELETE_KEY_PATTERN_1ARGS_S "DELETE %s"
#define GET_KEY_PATTERN_1ARGS_S "GET %s"
#define SET_KEY_PATTERN_2ARGS_SS "SET %s 0 0 %s"

namespace fastonosql {
namespace core {
namespace memcached {

Driver::Driver(IConnectionSettingsBaseSPtr settings)
  : IDriverRemote(settings), impl_(new DBConnection) {
  CHECK(type() == MEMCACHED);
}

Driver::~Driver() {
  delete impl_;
}

bool Driver::isConnected() const {
  return impl_->isConnected();
}

bool Driver::isAuthenticated() const {
  return impl_->isConnected();
}

common::net::hostAndPort Driver::host() const {
  Config conf = impl_->config();
  return conf.host;
}

std::string Driver::nsSeparator() const {
  return impl_->nsSeparator();
}

std::string Driver::outputDelemitr() const {
  return impl_->delimiter();
}

void Driver::initImpl() {
}

void Driver::clearImpl() {
}

common::Error Driver::executeImpl(int argc, char** argv, FastoObject* out) {
  return impl_->execute(argc, argv, out);
}

common::Error Driver::serverInfo(IServerInfo** info) {
  LOG_COMMAND(type(), fastonosql::Command(INFO_REQUEST, common::Value::C_INNER));
  ServerInfo::Common cm;
  common::Error err = impl_->info(nullptr, &cm);
  if (!err) {
    *info = new ServerInfo(cm);
  }

  return err;
}

common::Error Driver::serverDiscoveryClusterInfo(ServerDiscoveryClusterInfo** dinfo, IServerInfo** sinfo,
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

common::Error Driver::currentDataBaseInfo(IDataBaseInfo** info) {
  if (!info) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  size_t dbsize = 0;
  impl_->dbsize(&dbsize);
  *info = new DataBaseInfo("0", true, dbsize);
  return common::Error();
}

void Driver::handleConnectEvent(events::ConnectRequestEvent* ev) {
  QObject* sender = ev->sender();
  notifyProgress(sender, 0);
  events::ConnectResponceEvent::value_type res(ev->value());
  ConnectionSettings* set = dynamic_cast<ConnectionSettings*>(settings_.get());  // +
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

void Driver::handleDisconnectEvent(events::DisconnectRequestEvent* ev) {
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

void Driver::handleExecuteEvent(events::ExecuteRequestEvent* ev) {
  QObject* sender = ev->sender();
  notifyProgress(sender, 0);
  events::ExecuteResponceEvent::value_type res(ev->value());
  const char* inputLine = common::utils::c_strornull(res.text);

  if (inputLine) {
    size_t length = strlen(inputLine);
    int offset = 0;
    RootLocker lock = make_locker(sender, inputLine);
    FastoObjectIPtr obj = lock.root();
    const double step = 100.0 / length;
    for (size_t n = 0; n < length; ++n) {
      if (isInterrupted()) {
        res.setErrorInfo(common::make_error_value("Interrupted exec.",
                                                  common::ErrorValue::E_INTERRUPTED,
                                                  common::logging::L_WARNING));
        break;
      }
      if (inputLine[n] == '\n' || n == length - 1) {
        notifyProgress(sender, step * n);
        char command[128] = {0};
        if (n == length - 1) {
          strcpy(command, inputLine + offset);
        } else {
          strncpy(command, inputLine + offset, n - offset);
        }
        offset = n + 1;
        FastoObjectCommand* cmd = createCommand<Command>(obj, command, common::Value::C_USER);
        common::Error er = execute(cmd);
        if (er && er->isError()) {
          res.setErrorInfo(er);
          break;
        }
      }
    }
  } else {
    res.setErrorInfo(common::make_error_value("Empty command line.", common::ErrorValue::E_ERROR));
  }

  reply(sender, new events::ExecuteResponceEvent(this, res));
  notifyProgress(sender, 100);
}

void Driver::handleCommandRequestEvent(events::CommandRequestEvent* ev) {
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
  FastoObjectCommand* cmd = createCommand<Command>(obj, cmdtext, common::Value::C_INNER);
  notifyProgress(sender, 50);
  er = execute(cmd);
  if (er && er->isError()) {
    res.setErrorInfo(er);
  }
  reply(sender, new events::CommandResponceEvent(this, res));
  notifyProgress(sender, 100);
}

void Driver::handleLoadDatabaseContentEvent(events::LoadDatabaseContentRequestEvent* ev) {
  QObject* sender = ev->sender();
  notifyProgress(sender, 0);
  events::LoadDatabaseContentResponceEvent::value_type res(ev->value());
  FastoObjectIPtr root = FastoObject::createRoot(GET_KEYS);
  notifyProgress(sender, 50);
  FastoObjectCommand* cmd = createCommand<Command>(root, GET_KEYS, common::Value::C_INNER);
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
        bool isok = ar->getString(i, &key);
        if (isok) {
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

// ============== commands =============//
common::Error Driver::commandDeleteImpl(CommandDeleteKey* command,
                                                 std::string* cmdstring) const {
  if (!command || !cmdstring) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  NDbKValue key = command->key();
  *cmdstring = common::MemSPrintf(DELETE_KEY_PATTERN_1ARGS_S, key.keyString());
  return common::Error();
}

common::Error Driver::commandLoadImpl(CommandLoadKey* command,
                                               std::string* cmdstring) const {
  if (!command || !cmdstring) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  NDbKValue key = command->key();
  *cmdstring = common::MemSPrintf(GET_KEY_PATTERN_1ARGS_S, key.keyString());
  return common::Error();
}

common::Error Driver::commandCreateImpl(CommandCreateKey* command,
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

common::Error Driver::commandChangeTTLImpl(CommandChangeTTL* command,
                                                    std::string* cmdstring) const {
  if (!command || !cmdstring) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  std::string errorMsg = common::MemSPrintf("Sorry, but now " PROJECT_NAME_TITLE " not supported change ttl command for %s.",
                                            common::convertToString(type()));
  return common::make_error_value(errorMsg, common::ErrorValue::E_ERROR);
}

// ============== commands =============//

void Driver::handleProcessCommandLineArgs(events::ProcessConfigArgsRequestEvent* ev) {
}

IServerInfoSPtr Driver::makeServerInfoFromString(const std::string& val) {
  IServerInfoSPtr res(makeMemcachedServerInfo(val));
  return res;
}

}  // namespace memcached
}  // namespace core
}  // namespace fastonosql
