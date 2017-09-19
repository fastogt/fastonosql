/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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

#include "proxy/db/upscaledb/driver.h"

#include <common/convert2string.h>

#include "core/db/upscaledb/db_connection.h"  // for DBConnection

#include "proxy/command/command.h"                   // for CreateCommand, etc
#include "proxy/command/command_logger.h"            // for LOG_COMMAND
#include "proxy/db/upscaledb/command.h"              // for Command
#include "proxy/db/upscaledb/connection_settings.h"  // for ConnectionSettings

#define UPSCALEDB_INFO_REQUEST "INFO"

#define UPSCALEDB_GET_KEYS_PATTERN_1ARGS_I "KEYS a z %d"

namespace fastonosql {
namespace proxy {
namespace upscaledb {

Driver::Driver(IConnectionSettingsBaseSPtr settings)
    : IDriverLocal(settings), impl_(new core::upscaledb::DBConnection(this)) {
  COMPILE_ASSERT(core::upscaledb::DBConnection::connection_t == core::UPSCALEDB,
                 "DBConnection must be the same type as Driver!");
  CHECK(GetType() == core::UPSCALEDB);
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

core::translator_t Driver::GetTranslator() const {
  return impl_->GetTranslator();
}

bool Driver::IsConnected() const {
  return impl_->IsConnected();
}

bool Driver::IsAuthenticated() const {
  return impl_->IsConnected();
}

std::string Driver::GetPath() const {
  core::upscaledb::Config config = impl_->config();
  return config.db_path;
}

std::string Driver::GetNsSeparator() const {
  return impl_->GetNsSeparator();
}

std::string Driver::GetDelimiter() const {
  return impl_->GetDelimiter();
}

void Driver::InitImpl() {}

void Driver::ClearImpl() {}

core::FastoObjectCommandIPtr Driver::CreateCommand(core::FastoObject* parent,
                                                   const core::command_buffer_t& input,
                                                   core::CmdLoggingType ct) {
  return proxy::CreateCommand<upscaledb::Command>(parent, input, ct);
}

core::FastoObjectCommandIPtr Driver::CreateCommandFast(const core::command_buffer_t& input, core::CmdLoggingType ct) {
  return proxy::CreateCommandFast<upscaledb::Command>(input, ct);
}

common::Error Driver::SyncConnect() {
  ConnectionSettings* set = dynamic_cast<ConnectionSettings*>(settings_.get());  // +
  CHECK(set);
  return impl_->Connect(set->GetInfo());
}

common::Error Driver::SyncDisconnect() {
  return impl_->Disconnect();
}

common::Error Driver::ExecuteImpl(const core::command_buffer_t& command, core::FastoObject* out) {
  return impl_->Execute(command, out);
}

common::Error Driver::CurrentServerInfo(core::IServerInfo** info) {
  core::FastoObjectCommandIPtr cmd = CreateCommandFast(UPSCALEDB_INFO_REQUEST, core::C_INNER);
  LOG_COMMAND(cmd);
  core::upscaledb::ServerInfo::Stats cm;
  common::Error err = impl_->Info(std::string(), &cm);
  if (err) {
    return err;
  }

  *info = new core::upscaledb::ServerInfo(cm);
  return common::Error();
}

common::Error Driver::CurrentDataBaseInfo(core::IDataBaseInfo** info) {
  if (!info) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  return impl_->Select(impl_->GetCurrentDBName(), info);
}

void Driver::HandleLoadDatabaseContentEvent(events::LoadDatabaseContentRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::LoadDatabaseContentResponceEvent::value_type res(ev->value());
  const core::command_buffer_t pattern_result =
      core::internal::GetKeysPattern(res.cursor_in, res.pattern, res.count_keys);
  core::FastoObjectCommandIPtr cmd = CreateCommandFast(pattern_result, core::C_INNER);
  NotifyProgress(sender, 50);
  common::Error err = Execute(cmd);
  if (err) {
    res.setErrorInfo(err);
  } else {
    core::FastoObject::childs_t rchildrens = cmd->GetChildrens();
    if (rchildrens.size()) {
      CHECK_EQ(rchildrens.size(), 1);
      core::FastoObjectArray* array = dynamic_cast<core::FastoObjectArray*>(rchildrens[0].get());  // +
      if (!array) {
        goto done;
      }

      common::ArrayValue* arm = array->GetArray();
      if (!arm->GetSize()) {
        goto done;
      }

      std::string cursor;
      bool isok = arm->GetString(0, &cursor);
      if (!isok) {
        goto done;
      }

      uint64_t lcursor;
      if (common::ConvertFromString(cursor, &lcursor)) {
        res.cursor_out = lcursor;
      }

      rchildrens = array->GetChildrens();
      if (!rchildrens.size()) {
        goto done;
      }

      core::FastoObject* obj = rchildrens[0].get();
      core::FastoObjectArray* arr = dynamic_cast<core::FastoObjectArray*>(obj);  // +
      if (!arr) {
        goto done;
      }

      common::ArrayValue* ar = arr->GetArray();
      if (ar->IsEmpty()) {
        goto done;
      }

      for (size_t i = 0; i < ar->GetSize(); ++i) {
        std::string key_str;
        if (ar->GetString(i, &key_str)) {
          core::key_t key(key_str);
          core::NKey k(key);
          core::NValue empty_val(common::Value::CreateEmptyValueFromType(common::Value::TYPE_STRING));
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
  core::IServerInfoSPtr res(core::upscaledb::MakeUpscaleDBServerInfo(val));
  return res;
}

}  // namespace upscaledb
}  // namespace proxy
}  // namespace fastonosql
