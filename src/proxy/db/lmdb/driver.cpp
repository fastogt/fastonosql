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
    along with FastoNoSQL.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "proxy/db/lmdb/driver.h"

#include <fastonosql/core/db/lmdb/database_info.h>
#include <fastonosql/core/db/lmdb/db_connection.h>  // for DBConnection

#include "proxy/command/command.h"              // for CreateCommand, etc
#include "proxy/command/command_logger.h"       // for LOG_COMMAND
#include "proxy/db/lmdb/command.h"              // for Command
#include "proxy/db/lmdb/connection_settings.h"  // for ConnectionSettings

namespace fastonosql {
namespace proxy {
namespace lmdb {

Driver::Driver(IConnectionSettingsBaseSPtr settings)
    : IDriverLocal(settings), impl_(new core::lmdb::DBConnection(this)) {
  COMPILE_ASSERT(core::lmdb::DBConnection::connection_t == core::LMDB, "DBConnection must be the same type as Driver!");
  CHECK(GetType() == core::LMDB);
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
  return impl_->IsAuthenticated();
}

void Driver::InitImpl() {}

void Driver::ClearImpl() {}

core::FastoObjectCommandIPtr Driver::CreateCommand(core::FastoObject* parent,
                                                   const core::command_buffer_t& input,
                                                   core::CmdLoggingType logging_type) {
  return proxy::CreateCommand<lmdb::Command>(parent, input, logging_type);
}

core::FastoObjectCommandIPtr Driver::CreateCommandFast(const core::command_buffer_t& input,
                                                       core::CmdLoggingType logging_type) {
  return proxy::CreateCommandFast<lmdb::Command>(input, logging_type);
}

core::IDataBaseInfoSPtr Driver::CreateDatabaseInfo(const std::string& name, bool is_default, size_t size) {
  return std::make_shared<core::lmdb::DataBaseInfo>(name, is_default, size);
}

common::Error Driver::SyncConnect() {
  auto lmdb_settings = GetSpecificSettings<ConnectionSettings>();
  return impl_->Connect(lmdb_settings->GetInfo());
}

common::Error Driver::SyncDisconnect() {
  return impl_->Disconnect();
}

common::Error Driver::ExecuteImpl(const core::command_buffer_t& command, core::FastoObject* out) {
  return impl_->Execute(command, out);
}

common::Error Driver::DBkcountImpl(core::keys_limit_t* size) {
  return impl_->DBkcount(size);
}

common::Error Driver::GetCurrentServerInfo(core::IServerInfo** info) {
  core::FastoObjectCommandIPtr cmd = CreateCommandFast(DB_INFO_COMMAND, core::C_INNER);
  LOG_COMMAND(cmd);
  core::lmdb::ServerInfo::Stats cm;
  common::Error err = impl_->Info(std::string(), &cm);
  if (err) {
    return err;
  }

  *info = new core::lmdb::ServerInfo(cm);
  return common::Error();
}

common::Error Driver::GetServerCommands(std::vector<const core::CommandInfo*>* commands) {
  std::vector<const core::CommandInfo*> lcommands;
  const core::ConstantCommandsArray& origin = core::lmdb::DBConnection::GetCommands();
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

core::IServerInfoSPtr Driver::MakeServerInfoFromString(const std::string& val) {
  core::IServerInfoSPtr res(core::lmdb::MakeLmdbServerInfo(val));
  return res;
}

}  // namespace lmdb
}  // namespace proxy
}  // namespace fastonosql
