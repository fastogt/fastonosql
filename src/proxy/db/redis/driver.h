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

#include <string>
#include <vector>

#include "proxy/driver/idriver_remote.h"  // for IDriverRemote

namespace fastonosql {
namespace core {
#if defined(PRO_VERSION)
class IModuleConnectionClient;
#endif
namespace redis {
class DBConnection;
}
}  // namespace core
}  // namespace fastonosql

namespace fastonosql {
namespace proxy {
namespace redis {

class Driver : public IDriverRemote {
  Q_OBJECT

 public:
  explicit Driver(IConnectionSettingsBaseSPtr settings);
  ~Driver() override;

  bool IsInterrupted() const override;
  void SetInterrupted(bool interrupted) override;

  core::translator_t GetTranslator() const override;

  bool IsConnected() const override;
  bool IsAuthenticated() const override;

#if defined(PRO_VERSION)
 Q_SIGNALS:
  void ModuleLoaded(core::ModuleInfo module);
  void ModuleUnLoaded(core::ModuleInfo module);
#endif

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
#if defined(PRO_VERSION)
  common::Error GetServerLoadedModules(std::vector<core::ModuleInfo>* modules);
#endif
  common::Error GetCurrentDataBaseInfo(core::IDataBaseInfo** info) override;

  void HandleDiscoveryInfoEvent(events::DiscoveryInfoRequestEvent* ev) override;
  void HandleLoadServerPropertyEvent(events::ServerPropertyInfoRequestEvent* ev) override;
  void HandleServerPropertyChangeEvent(events::ChangeServerPropertyInfoRequestEvent* ev) override;
  void HandleLoadServerChannelsRequestEvent(events::LoadServerChannelsRequestEvent* ev) override;
  void HandleBackupEvent(events::BackupRequestEvent* ev) override;
  void HandleRestoreEvent(events::RestoreRequestEvent* ev) override;

  void HandleLoadDatabaseContentEvent(events::LoadDatabaseContentRequestEvent* ev) override;

  core::IServerInfoSPtr MakeServerInfoFromString(const std::string& val) override;

#if defined(PRO_VERSION)
  core::IModuleConnectionClient* proxy_;
#endif
  core::redis::DBConnection* impl_;
};

}  // namespace redis
}  // namespace proxy
}  // namespace fastonosql
