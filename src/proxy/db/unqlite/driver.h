/*  Copyright (C) 2014-2022 FastoGT. All right reserved.

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

#pragma once

#include <string>
#include <vector>

#include "proxy/driver/idriver_local.h"

namespace fastonosql {
namespace core {
namespace unqlite {
class DBConnection;
}
}  // namespace core
}  // namespace fastonosql

namespace fastonosql {
namespace proxy {
namespace unqlite {

class Driver : public IDriverLocal {
  Q_OBJECT

 public:
  explicit Driver(IConnectionSettingsBaseSPtr settings);
  ~Driver() override;

  bool IsInterrupted() const override;
  void SetInterrupted(bool interrupted) override;

  core::translator_t GetTranslator() const override;

  bool IsConnected() const override;
  bool IsAuthenticated() const override;

 private:
  void InitImpl() override;
  void ClearImpl() override;

  core::FastoObjectCommandIPtr CreateCommand(core::FastoObject* parent,
                                             const core::command_buffer_t& input,
                                             core::CmdLoggingType logging_type) override;

  core::FastoObjectCommandIPtr CreateCommandFast(const core::command_buffer_t& input,
                                                 core::CmdLoggingType logging_type) override;

  core::IDataBaseInfoSPtr CreateDatabaseInfo(const core::db_name_t& name, bool is_default, size_t size) override;

  common::Error SyncConnect() override WARN_UNUSED_RESULT;
  common::Error SyncDisconnect() override WARN_UNUSED_RESULT;

  common::Error ExecuteImpl(const core::command_buffer_t& command, core::FastoObject* out) override WARN_UNUSED_RESULT;
  common::Error DBkcountImpl(core::keys_limit_t* size) override WARN_UNUSED_RESULT;

  common::Error GetCurrentServerInfo(core::IServerInfo** info) override;
  common::Error GetServerCommands(std::vector<const core::CommandInfo*>* commands) override;
  common::Error GetCurrentDataBaseInfo(core::IDataBaseInfo** info) override;

  core::IServerInfoSPtr MakeServerInfoFromString(const std::string& val) override;

 private:
  core::unqlite::DBConnection* const impl_;
};

}  // namespace unqlite
}  // namespace proxy
}  // namespace fastonosql
