/*  Copyright (C) 2014-2018 FastoGT. All right reserved.

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
    along with FastoNoSQL. If not, see <http://www.gnu.org/licenses/>.
*/

#include "proxy/db/ssdb/driver.h"

#include <common/convert2string.h>

#include <fastonosql/core/db/ssdb/db_connection.h>  // for DBConnection
#include <fastonosql/core/value.h>

#include "proxy/command/command.h"              // for CreateCommand, etc
#include "proxy/command/command_logger.h"       // for LOG_COMMAND
#include "proxy/db/ssdb/command.h"              // for Command
#include "proxy/db/ssdb/connection_settings.h"  // for ConnectionSettings

namespace fastonosql {
namespace proxy {
namespace ssdb {

Driver::Driver(IConnectionSettingsBaseSPtr settings)
    : IDriverRemote(settings), impl_(new core::ssdb::DBConnection(this)) {
  COMPILE_ASSERT(core::ssdb::DBConnection::GetConnectionType() == core::SSDB,
                 "DBConnection must be the same type as Driver!");
  CHECK(GetType() == core::SSDB);
}

Driver::~Driver() {
  delete impl_;
}

bool Driver::IsInterrupted() const {
  return impl_->IsInterrupted();
}

void Driver::SetInterrupted(bool interrupted) {
  return impl_->SetInterrupted(interrupted);
}

core::translator_t Driver::GetTranslator() const {
  return impl_->GetTranslator();
}

bool Driver::IsConnected() const {
  return impl_->IsConnected();
}

bool Driver::IsAuthenticated() const {
  return impl_->IsAuthenticated();
}

void Driver::InitImpl() {}

void Driver::ClearImpl() {}

core::FastoObjectCommandIPtr Driver::CreateCommand(core::FastoObject* parent,
                                                   const core::command_buffer_t& input,
                                                   core::CmdLoggingType logging_type) {
  return proxy::CreateCommand<ssdb::Command>(parent, input, logging_type);
}

core::FastoObjectCommandIPtr Driver::CreateCommandFast(const core::command_buffer_t& input,
                                                       core::CmdLoggingType logging_type) {
  return proxy::CreateCommandFast<ssdb::Command>(input, logging_type);
}

core::IDataBaseInfoSPtr Driver::CreateDatabaseInfo(const core::db_name_t& name, bool is_default, size_t size) {
  return core::IDataBaseInfoSPtr(impl_->MakeDatabaseInfo(name, is_default, size));
}

common::Error Driver::SyncConnect() {
  auto ssdb_settings = GetSpecificSettings<ConnectionSettings>();
  return impl_->Connect(ssdb_settings->GetInfo());
}

common::Error Driver::SyncDisconnect() {
  return impl_->Disconnect();
}

common::Error Driver::ExecuteImpl(const core::command_buffer_t& command, core::FastoObject* out) {
  return impl_->Execute(command, out);
}

common::Error Driver::DBkcountImpl(core::keys_limit_t* size) {
  return impl_->DBKeysCount(size);
}

common::Error Driver::GetCurrentServerInfo(core::IServerInfo** info) {
  core::FastoObjectCommandIPtr cmd = CreateCommandFast(GEN_CMD_STRING(DB_INFO_COMMAND), core::C_INNER);
  LOG_COMMAND(cmd);
  core::ssdb::ServerInfo::Stats cm;
  common::Error err = impl_->Info(core::command_buffer_t(), &cm);
  if (err) {
    return err;
  }

  *info = new core::ssdb::ServerInfo(cm);
  return common::Error();
}

common::Error Driver::GetServerCommands(std::vector<const core::CommandInfo*>* commands) {
  std::vector<const core::CommandInfo*> lcommands;
  const core::ConstantCommandsArray& origin = core::ssdb::DBConnection::GetCommands();
  for (size_t i = 0; i < origin.size(); ++i) {
    lcommands.push_back(&origin[i]);
  }
  *commands = lcommands;
  return common::Error();
}

common::Error Driver::GetCurrentDataBaseInfo(core::IDataBaseInfo** info) {
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
  const core::command_buffer_t pattern_result = core::GetKeysPattern(res.cursor_in, res.pattern, res.keys_count);
  core::FastoObjectCommandIPtr cmd = CreateCommandFast(pattern_result, core::C_INNER);
  NotifyProgress(sender, 50);
  common::Error err = Execute(cmd);
  if (err) {
    res.setErrorInfo(err);
  } else {
    core::FastoObject::childs_t rchildrens = cmd->GetChildrens();
    if (rchildrens.size()) {
      CHECK_EQ(rchildrens.size(), 1);
      core::FastoObject* array = rchildrens[0].get();
      CHECK(array);
      auto array_value = array->GetValue();
      common::ArrayValue* arm = nullptr;
      if (!array_value->GetAsList(&arm)) {
        goto done;
      }

      CHECK_EQ(arm->GetSize(), 2);
      core::cursor_t cursor;
      bool isok = arm->GetUInteger(0, &cursor);
      if (!isok) {
        goto done;
      }
      res.cursor_out = cursor;

      common::ArrayValue* ar = nullptr;
      isok = arm->GetList(1, &ar);
      if (!isok) {
        goto done;
      }

      for (size_t i = 0; i < ar->GetSize(); ++i) {
        core::command_buffer_t key_str;
        if (ar->GetString(i, &key_str)) {
          const core::nkey_t key(key_str);
          core::NKey k(key);
          core::command_buffer_writer_t wr;
          wr << DB_GET_TTL_COMMAND " " << key.GetHumanReadable();  // emulate log execution
          core::FastoObjectCommandIPtr cmd_ttl = CreateCommandFast(wr.str(), core::C_INNER);
          LOG_COMMAND(cmd_ttl);
          core::ttl_t ttl = NO_TTL;
          common::Error err = impl_->GetTTL(k, &ttl);
          if (err) {
            k.SetTTL(NO_TTL);
          } else {
            k.SetTTL(ttl);
          }

          core::command_buffer_writer_t wr2;
          wr2 << DB_KEY_TYPE_COMMAND " " << key.GetHumanReadable();  // emulate log execution
          core::FastoObjectCommandIPtr cmd_type = CreateCommandFast(wr2.str(), core::C_INNER);
          LOG_COMMAND(cmd_type);
          core::readable_string_t type_str;
          impl_->GetType(k, &type_str);
          core::NValue empty_val;
          if (type_str == GEN_READABLE_STRING("list")) {
            empty_val.reset(core::CreateEmptyValueFromType(common::Value::TYPE_ARRAY));
          } else {
            empty_val.reset(core::CreateEmptyValueFromType(common::Value::TYPE_STRING));
          }

          core::NDbKValue ress(k, empty_val);
          res.keys.push_back(ress);
        }
      }

      common::Error err = DBkcountImpl(&res.db_keys_count);
      DCHECK(!err);
    }
  }
done:
  NotifyProgress(sender, 75);
  Reply(sender, new events::LoadDatabaseContentResponceEvent(this, res));
  NotifyProgress(sender, 100);
}

core::IServerInfoSPtr Driver::MakeServerInfoFromString(const std::string& val) {
  return core::IServerInfoSPtr(impl_->MakeServerInfo(val));
}

}  // namespace ssdb
}  // namespace proxy
}  // namespace fastonosql
