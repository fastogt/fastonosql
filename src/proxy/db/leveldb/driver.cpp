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

#include "proxy/db/leveldb/driver.h"

#include <fastonosql/core/db/leveldb/database_info.h>
#include <fastonosql/core/db/leveldb/db_connection.h>  // for DBConnection

#include "proxy/command/command.h"                 // for CreateCommand, etc
#include "proxy/command/command_logger.h"          // for LOG_COMMAND
#include "proxy/db/leveldb/command.h"              // for Command
#include "proxy/db/leveldb/connection_settings.h"  // for ConnectionSettings
#include "proxy/db/leveldb/database.h"             // for DataBaseInfo

namespace fastonosql {
namespace proxy {
namespace leveldb {

Driver::Driver(IConnectionSettingsBaseSPtr settings)
    : IDriverLocal(settings), impl_(new core::leveldb::DBConnection(this)) {
  COMPILE_ASSERT(core::leveldb::DBConnection::GetConnectionType() == core::LEVELDB,
                 "DBConnection must be the same type as Driver!");
  CHECK(GetType() == core::LEVELDB);
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
  return proxy::CreateCommand<Command>(parent, input, logging_type);
}

core::FastoObjectCommandIPtr Driver::CreateCommandFast(const core::command_buffer_t& input,
                                                       core::CmdLoggingType logging_type) {
  return proxy::CreateCommandFast<Command>(input, logging_type);
}

core::IDataBaseInfoSPtr Driver::CreateDatabaseInfo(const core::db_name_t& name, bool is_default, size_t size) {
  return std::make_shared<core::leveldb::DataBaseInfo>(name, is_default, size);
}

common::Error Driver::SyncConnect() {
  auto leveldb_settings = GetSpecificSettings<ConnectionSettings>();
  return impl_->Connect(leveldb_settings->GetInfo());
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
  core::leveldb::ServerInfo::Stats cm;
  common::Error err = impl_->Info(&cm);
  if (err) {
    return err;
  }

  *info = new core::leveldb::ServerInfo(cm);
  return common::Error();
}

common::Error Driver::GetServerCommands(std::vector<const core::CommandInfo*>* commands) {
  std::vector<const core::CommandInfo*> lcommands;
  const core::ConstantCommandsArray& origin = core::leveldb::DBConnection::GetCommands();
  for (size_t i = 0; i < origin.size(); ++i) {
    lcommands.push_back(&origin[i]);
  }
  *commands = lcommands;
  return common::Error();
}

common::Error Driver::GetCurrentDataBaseInfo(core::IDataBaseInfo** info) {
  if (!info) {
    return common::make_error_inval();
  }

  return impl_->Select(impl_->GetCurrentDBName(), info);
}

core::IServerInfoSPtr Driver::MakeServerInfoFromString(const std::string& val) {
  core::IServerInfoSPtr res(core::leveldb::MakeLeveldbServerInfo(val));
  return res;
}

}  // namespace leveldb
}  // namespace proxy
}  // namespace fastonosql
