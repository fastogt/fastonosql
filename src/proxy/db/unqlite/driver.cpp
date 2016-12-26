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

#include "proxy/db/unqlite/driver.h"

#include <stddef.h>  // for size_t
#include <stdint.h>  // for uint64_t

#include <memory>  // for __shared_ptr
#include <string>  // for string

#include <common/intrusive_ptr.h>  // for intrusive_ptr
#include <common/qt/utils_qt.h>    // for Event<>::value_type
#include <common/sprintf.h>        // for MemSPrintf
#include <common/value.h>          // for ErrorValue, etc
#include <common/convert2string.h>

#include "proxy/command/command.h"         // for CreateCommand, etc
#include "proxy/command/command_logger.h"  // for LOG_COMMAND
#include "core/connection_types.h"         // for ConvertToString, etc
#include "core/db_key.h"                   // for NDbKValue, NValue, NKey

#include "proxy/events/events_info.h"

#include "core/internal/cdb_connection.h"
#include "core/internal/db_connection.h"

#include "proxy/db/unqlite/command.h"              // for Command
#include "core/db/unqlite/config.h"                // for Config
#include "proxy/db/unqlite/connection_settings.h"  // for ConnectionSettings
#include "core/db/unqlite/db_connection.h"         // for DBConnection
#include "core/db/unqlite/server_info.h"           // for ServerInfo, etc

#include "global/global.h"  // for FastoObject::childs_t, etc

#define UNQLITE_INFO_REQUEST "INFO"

namespace fastonosql {
namespace proxy {
namespace unqlite {

Driver::Driver(IConnectionSettingsBaseSPtr settings)
    : IDriverLocal(settings), impl_(new core::unqlite::DBConnection(this)) {
  COMPILE_ASSERT(core::unqlite::DBConnection::connection_t == core::UNQLITE,
                 "DBConnection must be the same type as Driver!");
  CHECK(Type() == core::UNQLITE);
}

Driver::~Driver() {
  delete impl_;
}

bool Driver::IsInterrupted() const {
  return impl_->IsInterrupted();
}

void Driver::SetInterrupted(bool interrupted) {
  impl_->SetInterrupted(interrupted);
}

core::translator_t Driver::Translator() const {
  return impl_->Translator();
}

bool Driver::IsConnected() const {
  return impl_->IsConnected();
}

bool Driver::IsAuthenticated() const {
  return impl_->IsConnected();
}

std::string Driver::Path() const {
  core::unqlite::Config conf = impl_->config();
  return conf.dbname;
}

std::string Driver::NsSeparator() const {
  return impl_->NsSeparator();
}

std::string Driver::Delimiter() const {
  return impl_->Delimiter();
}

void Driver::InitImpl() {}

void Driver::ClearImpl() {}

FastoObjectCommandIPtr Driver::CreateCommand(FastoObject* parent,
                                             const std::string& input,
                                             common::Value::CommandLoggingType ct) {
  return proxy::CreateCommand<unqlite::Command>(parent, input, ct);
}

FastoObjectCommandIPtr Driver::CreateCommandFast(const std::string& input,
                                                 common::Value::CommandLoggingType ct) {
  return proxy::CreateCommandFast<unqlite::Command>(input, ct);
}

common::Error Driver::SyncConnect() {
  ConnectionSettings* set = dynamic_cast<ConnectionSettings*>(settings_.get());  // +
  CHECK(set);
  return impl_->Connect(set->Info());
}

common::Error Driver::SyncDisconnect() {
  return impl_->Disconnect();
}

common::Error Driver::ExecuteImpl(const std::string& command, FastoObject* out) {
  return impl_->Execute(command, out);
}

common::Error Driver::CurrentServerInfo(core::IServerInfo** info) {
  FastoObjectCommandIPtr cmd = CreateCommandFast(UNQLITE_INFO_REQUEST, common::Value::C_INNER);
  LOG_COMMAND(cmd);
  core::unqlite::ServerInfo::Stats cm;
  common::Error err = impl_->Info(nullptr, &cm);
  if (err && err->isError()) {
    return err;
  }

  *info = new core::unqlite::ServerInfo(cm);
  return common::Error();
}

common::Error Driver::CurrentDataBaseInfo(core::IDataBaseInfo** info) {
  if (!info) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  return impl_->Select(impl_->CurrentDBName(), info);
}

void Driver::HandleLoadDatabaseContentEvent(events::LoadDatabaseContentRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::LoadDatabaseContentResponceEvent::value_type res(ev->value());
  std::string patternResult =
      common::MemSPrintf(GET_KEYS_PATTERN_3ARGS_ISI, res.cursor_in, res.pattern, res.count_keys);
  FastoObjectCommandIPtr cmd = CreateCommandFast(patternResult, common::Value::C_INNER);
  NotifyProgress(sender, 50);
  common::Error er = Execute(cmd);
  if (er && er->isError()) {
    res.setErrorInfo(er);
  } else {
    FastoObject::childs_t rchildrens = cmd->Childrens();
    if (rchildrens.size()) {
      CHECK_EQ(rchildrens.size(), 1);
      FastoObjectArray* array = dynamic_cast<FastoObjectArray*>(rchildrens[0].get());  // +
      if (!array) {
        goto done;
      }

      common::ArrayValue* arm = array->Array();
      if (!arm->size()) {
        goto done;
      }

      std::string cursor;
      bool isok = arm->getString(0, &cursor);
      if (!isok) {
        goto done;
      }

      res.cursor_out = common::ConvertFromString<uint64_t>(cursor);

      rchildrens = array->Childrens();
      if (!rchildrens.size()) {
        goto done;
      }

      FastoObject* obj = rchildrens[0].get();
      FastoObjectArray* arr = dynamic_cast<FastoObjectArray*>(obj);  // +
      if (!arr) {
        goto done;
      }

      common::ArrayValue* ar = arr->Array();
      if (ar->empty()) {
        goto done;
      }

      for (size_t i = 0; i < ar->size(); ++i) {
        std::string key;
        if (ar->getString(i, &key)) {
          core::NKey k(key);
          core::NValue empty_val(
              common::Value::createEmptyValueFromType(common::Value::TYPE_STRING));
          core::NDbKValue ress(k, empty_val);
          res.keys.push_back(ress);
        }
      }

      common::Error err = impl_->DBkcount(&res.db_keys_count);
      DCHECK(!err);
    }
  }
done:
  NotifyProgress(sender, 75);
  Reply(sender, new events::LoadDatabaseContentResponceEvent(this, res));
  NotifyProgress(sender, 100);
}

core::IServerInfoSPtr Driver::MakeServerInfoFromString(const std::string& val) {
  core::IServerInfoSPtr res(core::unqlite::MakeUnqliteServerInfo(val));
  return res;
}

}  // namespace unqlite
}  // namespace proxy
}  // namespace fastonosql
