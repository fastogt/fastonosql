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

#pragma once

#include "proxy/driver/idriver_local.h"  // for IDriverLocal

namespace fastonosql {
namespace core {
namespace forestdb {
class DBConnection;
}
}  // namespace core
namespace proxy {
namespace forestdb {

class Driver : public IDriverLocal {
  Q_OBJECT
 public:
  explicit Driver(IConnectionSettingsBaseSPtr settings);
  virtual ~Driver();

  virtual bool IsInterrupted() const override;
  virtual void SetInterrupted(bool interrupted) override;

  virtual core::translator_t GetTranslator() const override;

  virtual bool IsConnected() const override;
  virtual bool IsAuthenticated() const override;

 private:
  virtual void InitImpl() override;
  virtual void ClearImpl() override;
  virtual core::FastoObjectCommandIPtr CreateCommand(core::FastoObject* parent,
                                                     const core::command_buffer_t& input,
                                                     core::CmdLoggingType ct) override;

  virtual core::FastoObjectCommandIPtr CreateCommandFast(const core::command_buffer_t& input,
                                                         core::CmdLoggingType ct) override;

  virtual common::Error SyncConnect() override WARN_UNUSED_RESULT;
  virtual common::Error SyncDisconnect() override WARN_UNUSED_RESULT;

  virtual common::Error ExecuteImpl(const core::command_buffer_t& command, core::FastoObject* out) override;

  virtual common::Error GetCurrentServerInfo(core::IServerInfo** info) override;
  virtual common::Error GetServerCommands(std::vector<const core::CommandInfo*>* commands) override;
  virtual common::Error GetServerLoadedModules(std::vector<core::ModuleInfo>* modules) override;
  virtual common::Error GetCurrentDataBaseInfo(core::IDataBaseInfo** info) override;

  virtual void HandleLoadDatabaseInfosEvent(events::LoadDatabasesInfoRequestEvent* ev) override;
  virtual void HandleLoadDatabaseContentEvent(events::LoadDatabaseContentRequestEvent* ev) override;

  virtual core::IServerInfoSPtr MakeServerInfoFromString(const std::string& val) override;

  core::forestdb::DBConnection* const impl_;
};

}  // namespace forestdb
}  // namespace proxy
}  // namespace fastonosql
