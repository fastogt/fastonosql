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

#include "core/rocksdb/driver.h"

#include <stddef.h>  // for size_t

#include <memory>  // for __shared_ptr
#include <string>  // for string

#include "common/log_levels.h"   // for LEVEL_LOG::L_WARNING
#include "common/qt/utils_qt.h"  // for Event<>::value_type
#include "common/sprintf.h"      // for MemSPrintf
#include "common/value.h"        // for ErrorValue, etc

#include "core/command.h"           // for createCommand, etc
#include "core/command_logger.h"    // for LOG_COMMAND
#include "core/connection_types.h"  // for ConvertToString, etc
#include "core/db_key.h"            // for NDbKValue, NValue, NKey

#include "core/events/events_info.h"

#include "core/rocksdb/command.h"              // for Command
#include "core/rocksdb/config.h"               // for Config
#include "core/rocksdb/connection_settings.h"  // for ConnectionSettings
#include "core/rocksdb/database.h"             // for DataBaseInfo
#include "core/rocksdb/db_connection.h"        // for DBConnection
#include "core/rocksdb/server_info.h"          // for ServerInfo, etc

#include "global/global.h"  // for FastoObject::childs_t, etc
#include "global/types.h"   // for Command

#define ROCKSDB_INFO_REQUEST "INFO"
#define ROCKSDB_GET_KEY_PATTERN_1ARGS_S "GET %s"
#define ROCKSDB_SET_KEY_PATTERN_2ARGS_SS "SET %s %s"

#define ROCKSDB_GET_KEYS_PATTERN_1ARGS_I "KEYS a z %d"
#define ROCKSDB_DELETE_KEY_PATTERN_1ARGS_S "DEL %s"

namespace fastonosql {
namespace core {
namespace rocksdb {

Driver::Driver(IConnectionSettingsBaseSPtr settings)
    : IDriverLocal(settings), impl_(new DBConnection(this)) {
  COMPILE_ASSERT(DBConnection::connection_t == ROCKSDB,
                 "DBConnection must be the same type as Driver!");
  CHECK(type() == ROCKSDB);
}

Driver::~Driver() {
  delete impl_;
}

bool Driver::isInterrupted() const {
  return impl_->isInterrupted();
}

void Driver::setInterrupted(bool interrupted) {
  return impl_->setInterrupted(interrupted);
}

bool Driver::isConnected() const {
  return impl_->isConnected();
}

bool Driver::isAuthenticated() const {
  return impl_->isConnected();
}

// ============== commands =============//
common::Error Driver::commandDeleteImpl(CommandDeleteKey* command, std::string* cmdstring) const {
  if (!command || !cmdstring) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  NDbKValue key = command->key();
  std::string key_str = key.keyString();
  *cmdstring = common::MemSPrintf(ROCKSDB_DELETE_KEY_PATTERN_1ARGS_S, key_str);
  return common::Error();
}

common::Error Driver::commandLoadImpl(CommandLoadKey* command, std::string* cmdstring) const {
  if (!command || !cmdstring) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  NDbKValue key = command->key();
  std::string key_str = key.keyString();
  *cmdstring = common::MemSPrintf(ROCKSDB_GET_KEY_PATTERN_1ARGS_S, key_str);
  return common::Error();
}

common::Error Driver::commandCreateImpl(CommandCreateKey* command, std::string* cmdstring) const {
  if (!command || !cmdstring) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  NDbKValue key = command->key();
  NValue val = command->value();
  common::Value* rval = val.get();
  std::string key_str = key.keyString();
  std::string value_str = common::ConvertToString(rval, " ");
  *cmdstring = common::MemSPrintf(ROCKSDB_SET_KEY_PATTERN_2ARGS_SS, key_str, value_str);
  return common::Error();
}

common::Error Driver::commandChangeTTLImpl(CommandChangeTTL* command,
                                           std::string* cmdstring) const {
  if (!command || !cmdstring) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  std::string errorMsg = common::MemSPrintf("Sorry, but now " PROJECT_NAME_TITLE
                                            " not supported change ttl command for %s.",
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

void Driver::initImpl() {}

void Driver::clearImpl() {}

FastoObjectCommandIPtr Driver::createCommand(FastoObject* parent,
                                             const std::string& input,
                                             common::Value::CommandLoggingType ct) {
  return CreateCommand<Command>(parent, input, ct);
}

FastoObjectCommandIPtr Driver::createCommandFast(const std::string& input,
                                                 common::Value::CommandLoggingType ct) {
  return CreateCommandFast<Command>(input, ct);
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
  FastoObjectCommandIPtr cmd = createCommandFast(ROCKSDB_INFO_REQUEST, common::Value::C_INNER);
  LOG_COMMAND(cmd);
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

  std::string name = impl_->currentDbName();
  size_t dbkcount = 0;
  common::Error err = impl_->dbkcount(&dbkcount);
  MCHECK(!err);
  *info = new DataBaseInfo(name, true, dbkcount);
  return common::Error();
}

void Driver::handleLoadDatabaseContentEvent(events::LoadDatabaseContentRequestEvent* ev) {
  QObject* sender = ev->sender();
  notifyProgress(sender, 0);
  events::LoadDatabaseContentResponceEvent::value_type res(ev->value());
  std::string patternResult = common::MemSPrintf(ROCKSDB_GET_KEYS_PATTERN_1ARGS_I, res.count_keys);
  FastoObjectCommandIPtr cmd = createCommandFast(patternResult, common::Value::C_INNER);
  notifyProgress(sender, 50);
  common::Error err = execute(cmd);
  if (err && err->isError()) {
    res.setErrorInfo(err);
  } else {
    FastoObject::childs_t rchildrens = cmd->childrens();
    if (rchildrens.size()) {
      CHECK_EQ(rchildrens.size(), 1);
      FastoObjectArray* array = dynamic_cast<FastoObjectArray*>(rchildrens[0].get());  // +
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
  IServerInfoSPtr res(makeRocksdbServerInfo(val));
  return res;
}

}  // namespace rocksdb
}  // namespace core
}  // namespace fastonosql
