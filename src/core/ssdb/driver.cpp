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

#include "core/ssdb/driver.h"

#include <string>
#include <vector>
#include <map>

#include "common/sprintf.h"
#include "common/utils.h"

#include "core/command_logger.h"

#include "core/ssdb/database.h"
#include "core/ssdb/command.h"
#include "core/ssdb/db_connection.h"

#define SSDB_INFO_REQUEST "INFO"
#define SSDB_GET_KEYS_PATTERN_1ARGS_I "KEYS a z %d"
#define SSDB_DELETE_KEY_PATTERN_1ARGS_S "DEL %s"

#define SSDB_GET_KEY_PATTERN_1ARGS_S "GET %s"
#define SSDB_GET_KEY_LIST_PATTERN_1ARGS_S "LRANGE %s 0 -1"
#define SSDB_GET_KEY_SET_PATTERN_1ARGS_S "SMEMBERS %s"
#define SSDB_GET_KEY_ZSET_PATTERN_1ARGS_S "ZRANGE %s 0 -1"
#define SSDB_GET_KEY_HASH_PATTERN_1ARGS_S "HGET %s"

#define SSDB_SET_KEY_PATTERN_2ARGS_SS "SET %s %s"
#define SSDB_SET_KEY_LIST_PATTERN_2ARGS_SS "LPUSH %s %s"
#define SSDB_SET_KEY_SET_PATTERN_2ARGS_SS "SADD %s %s"
#define SSDB_SET_KEY_ZSET_PATTERN_2ARGS_SS "ZADD %s %s"
#define SSDB_SET_KEY_HASH_PATTERN_2ARGS_SS "HMSET %s %s"

namespace fastonosql {
namespace core {
namespace ssdb {

Driver::Driver(IConnectionSettingsBaseSPtr settings)
  : IDriverRemote(settings), impl_(new DBConnection) {
  CHECK(type() == SSDB);
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

// ============== commands =============//
common::Error Driver::commandDeleteImpl(CommandDeleteKey* command,
                                            std::string* cmdstring) const {
  if (!command || !cmdstring) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  NDbKValue key = command->key();
  std::string key_str = key.keyString();
  *cmdstring = common::MemSPrintf(SSDB_DELETE_KEY_PATTERN_1ARGS_S, key_str);
  return common::Error();
}

common::Error Driver::commandLoadImpl(CommandLoadKey* command, std::string* cmdstring) const {
  if (!command || !cmdstring) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  std::string patternResult;
  NDbKValue key = command->key();
  common::Value::Type t = key.type();
  std::string key_str = key.keyString();
  if (t == common::Value::TYPE_ARRAY) {
    patternResult = common::MemSPrintf(SSDB_GET_KEY_LIST_PATTERN_1ARGS_S, key_str);
  } else if (t == common::Value::TYPE_SET) {
    patternResult = common::MemSPrintf(SSDB_GET_KEY_SET_PATTERN_1ARGS_S, key_str);
  } else if (t == common::Value::TYPE_ZSET) {
    patternResult = common::MemSPrintf(SSDB_GET_KEY_ZSET_PATTERN_1ARGS_S, key_str);
  } else if (t == common::Value::TYPE_HASH) {
    patternResult = common::MemSPrintf(SSDB_GET_KEY_HASH_PATTERN_1ARGS_S, key_str);
  } else {
    patternResult = common::MemSPrintf(SSDB_GET_KEY_PATTERN_1ARGS_S, key_str);
  }

  *cmdstring = patternResult;
  return common::Error();
}

common::Error Driver::commandCreateImpl(CommandCreateKey* command,
                                            std::string* cmdstring) const {
  if (!command || !cmdstring) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  std::string patternResult;
  NDbKValue key = command->key();
  NValue val = command->value();
  common::Value* rval = val.get();
  std::string key_str = key.keyString();
  std::string value_str = common::ConvertToString(rval, " ");
  common::Value::Type t = key.type();
  if (t == common::Value::TYPE_ARRAY) {
    patternResult = common::MemSPrintf(SSDB_SET_KEY_LIST_PATTERN_2ARGS_SS, key_str, value_str);
  } else if (t == common::Value::TYPE_SET) {
    patternResult = common::MemSPrintf(SSDB_SET_KEY_SET_PATTERN_2ARGS_SS, key_str, value_str);
  } else if (t == common::Value::TYPE_ZSET) {
    patternResult = common::MemSPrintf(SSDB_SET_KEY_ZSET_PATTERN_2ARGS_SS, key_str, value_str);
  } else if (t == common::Value::TYPE_HASH) {
    patternResult = common::MemSPrintf(SSDB_SET_KEY_HASH_PATTERN_2ARGS_SS, key_str, value_str);
  } else {
    patternResult = common::MemSPrintf(SSDB_SET_KEY_PATTERN_2ARGS_SS, key_str, value_str);
  }

  *cmdstring = patternResult;
  return common::Error();
}

common::Error Driver::commandChangeTTLImpl(CommandChangeTTL* command,
                                               std::string* cmdstring) const {
  if (!command || !cmdstring) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  std::string errorMsg = common::MemSPrintf("Sorry, but now " PROJECT_NAME_TITLE " not supported change ttl command for %s.",
                                          common::ConvertToString(type()));
  return common::make_error_value(errorMsg, common::ErrorValue::E_ERROR);
}
// ============== commands =============//

common::net::HostAndPort Driver::host() const {
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
  LOG_COMMAND(type(), fastonosql::Command(SSDB_INFO_REQUEST, common::Value::C_INNER));
  ServerInfo::Common cm;
  common::Error err = impl_->info(nullptr, &cm);
  if (!err) {
    *info = new ServerInfo(cm);
  }

  return err;
}

common::Error Driver::currentDataBaseInfo(IDataBaseInfo** info) {
  if (!info) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  size_t dbkcount = 0;
  impl_->dbkcount(&dbkcount);
  DataBaseInfo* sinfo = new DataBaseInfo("0", true, dbkcount);
  *info = sinfo;
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
  const std::string inputLine = res.text;
  if (inputLine.empty()) {
    res.setErrorInfo(common::make_error_value("Empty command line.", common::ErrorValue::E_ERROR));
    reply(sender, new events::ExecuteResponceEvent(this, res));
    notifyProgress(sender, 100);
  }

  size_t length = inputLine.length();
  int offset = 0;
  RootLocker lock = make_locker(sender, inputLine);
  FastoObjectIPtr obj = lock.root();
  const double step = 100.0 / length;
  for (size_t i = 0; i < length; ++i) {
    if (isInterrupted()) {
      res.setErrorInfo(common::make_error_value("Interrupted exec.",
                                                common::ErrorValue::E_INTERRUPTED,
                                                common::logging::L_WARNING));
      break;
    }

    if (inputLine[i] == '\n' || i == length - 1) {
      notifyProgress(sender, step * i);
      std::string command;
      if (i == length - 1) {
        command = inputLine.substr(offset);
      } else {
        command = inputLine.substr(offset, i - offset);
      }

      offset = i + 1;
      FastoObjectCommand* cmd = createCommand<Command>(obj, command, common::Value::C_USER);
      common::Error er = execute(cmd);
      if (er && er->isError()) {
        res.setErrorInfo(er);
        break;
      }
    }
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
    std::string patternResult = common::MemSPrintf(SSDB_GET_KEYS_PATTERN_1ARGS_I, res.count_keys);
    FastoObjectIPtr root = FastoObject::createRoot(patternResult);
  notifyProgress(sender, 50);
    FastoObjectCommand* cmd = createCommand<Command>(root, patternResult,
                                                         common::Value::C_INNER);
    common::Error er = execute(cmd);
    if (er && er->isError()) {
      res.setErrorInfo(er);
    } else {
      FastoObject::childs_t rchildrens = cmd->childrens();
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

        impl_->dbkcount(&res.db_keys_count);
      }
    }
done:
  notifyProgress(sender, 75);
    reply(sender, new events::LoadDatabaseContentResponceEvent(this, res));
  notifyProgress(sender, 100);
}

void Driver::handleClearDatabaseEvent(events::ClearDatabaseRequestEvent* ev) {
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

void Driver::handleProcessCommandLineArgs(events::ProcessConfigArgsRequestEvent* ev) {
  UNUSED(ev);
}

IServerInfoSPtr Driver::makeServerInfoFromString(const std::string& val) {
  IServerInfoSPtr res(makeSsdbServerInfo(val));
  return res;
}

}  // namespace ssdb
}  // namespace core
}  // namespace fastonosql
