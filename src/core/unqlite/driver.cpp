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

#include "core/unqlite/driver.h"

#include <stddef.h>                     // for size_t

#include <memory>                       // for __shared_ptr
#include <string>                       // for string

#include "common/log_levels.h"          // for LEVEL_LOG::L_WARNING
#include "common/qt/utils_qt.h"         // for Event<>::value_type
#include "common/sprintf.h"             // for MemSPrintf
#include "common/value.h"               // for ErrorValue, etc

#include "core/command.h"           // for createCommand, etc
#include "core/command_logger.h"        // for LOG_COMMAND
#include "core/connection_types.h"      // for ConvertToString, etc
#include "core/db_key.h"                // for NDbKValue, NValue, NKey

#include "core/events/events_info.h"

#include "core/unqlite/command.h"       // for Command
#include "core/unqlite/config.h"        // for Config
#include "core/unqlite/connection_settings.h"  // for ConnectionSettings
#include "core/unqlite/database.h"      // for DataBaseInfo
#include "core/unqlite/db_connection.h"  // for DBConnection
#include "core/unqlite/server_info.h"   // for ServerInfo, etc

#include "global/global.h"              // for FastoObject::childs_t, etc
#include "global/types.h"               // for Command

#define UNQLITE_INFO_REQUEST "INFO"
#define UNQLITE_GET_KEY_PATTERN_1ARGS_S "GET %s"
#define UNQLITE_SET_KEY_PATTERN_2ARGS_SS "SET %s %s"

#define UNQLITE_GET_KEYS_PATTERN_1ARGS_I "KEYS a z %d"
#define UNQLITE_DELETE_KEY_PATTERN_1ARGS_S "DEL %s"

namespace fastonosql {
namespace core {
namespace unqlite {

Driver::Driver(IConnectionSettingsBaseSPtr settings)
  : IDriverLocal(settings), impl_(new DBConnection) {
  COMPILE_ASSERT(DBConnection::connection_t == UNQLITE, "DBConnection must be the same type as Driver!");
  CHECK(type() == UNQLITE);
}

Driver::~Driver() {
  delete impl_;
}

bool Driver::isInterrupted() const {
  return impl_->isInterrupted();
}

void Driver::setInterrupted(bool interrupted) {
  impl_->setInterrupted(interrupted);
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
  *cmdstring = common::MemSPrintf(UNQLITE_DELETE_KEY_PATTERN_1ARGS_S, key_str);
  return common::Error();
}

common::Error Driver::commandLoadImpl(CommandLoadKey* command,
                                             std::string* cmdstring) const {
  if (!command || !cmdstring) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  NDbKValue key = command->key();
  std::string key_str = key.keyString();
  *cmdstring = common::MemSPrintf(UNQLITE_GET_KEY_PATTERN_1ARGS_S, key_str);
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
  std::string value_str = common::ConvertToString(rval, " ");
  *cmdstring = common::MemSPrintf(UNQLITE_SET_KEY_PATTERN_2ARGS_SS, key_str, value_str);
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

std::string Driver::path() const {
  Config conf = impl_->config();
  return conf.dbname;
}

std::string Driver::nsSeparator() const {
  return impl_->nsSeparator();
}

std::string Driver::delimiter() const {
  return impl_->delimiter();
}

void Driver::initImpl() {
}

void Driver::clearImpl() {
}

common::Error Driver::syncConnect() {
  ConnectionSettings* set = dynamic_cast<ConnectionSettings*>(settings_.get());  // +
  CHECK(set);
  return impl_->connect(set->info());
}

common::Error Driver::syncDisconnect() {
  return impl_->disconnect();
}

common::Error Driver::executeImpl(int argc, char** argv, FastoObject* out) {
  return impl_->execute(argc, argv, out);
}

common::Error Driver::serverInfo(IServerInfo** info) {
  FastoObjectCommandIPtr cmd = CreateCommandFast<Command>(UNQLITE_INFO_REQUEST, common::Value::C_INNER);
  LOG_COMMAND(fastonosql::Command(cmd->Clone()));
  ServerInfo::Stats cm;
  common::Error err = impl_->info(nullptr, &cm);
  if (err && err->isError()) {
    return err;
  }

  *info = new ServerInfo(cm);
  return common::Error();
}

common::Error Driver::currentDataBaseInfo(IDataBaseInfo** info) {
  if (!info) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  size_t dbkcount = 0;
  common::Error err = impl_->dbkcount(&dbkcount);
  MCHECK(!err);
  *info = new DataBaseInfo("0", true, dbkcount);
  return common::Error();
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
      FastoObjectCommand* cmd = CreateCommand<Command>(obj, command, common::Value::C_USER);
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
  FastoObjectCommand* cmd = CreateCommand<Command>(obj, cmdtext, common::Value::C_INNER);
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
  std::string patternResult = common::MemSPrintf(UNQLITE_GET_KEYS_PATTERN_1ARGS_I, res.count_keys);
  FastoObjectIPtr root = FastoObject::createRoot(patternResult);
  notifyProgress(sender, 50);
  FastoObjectCommand* cmd = CreateCommand<Command>(root, patternResult,
                                                          common::Value::C_INNER);
  common::Error err = execute(cmd);
  if (err && err->isError()) {
    res.setErrorInfo(err);
  } else {
    FastoObject::childs_t rchildrens = cmd->childrens();
    if (rchildrens.size()) {
      DCHECK_EQ(rchildrens.size(), 1);
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

       err = impl_->dbkcount(&res.db_keys_count);
       MCHECK(!err);
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
  IServerInfoSPtr res(makeUnqliteServerInfo(val));
  return res;
}

}  // namespace unqlite
}  // namespace core
}  // namespace fastonosql
