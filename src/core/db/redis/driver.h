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

#pragma once

#include <string>  // for string

#include <common/error.h>      // for Error
#include <common/macros.h>     // for WARN_UNUSED_RESULT
#include <common/net/types.h>  // for HostAndPort
#include <common/value.h>      // for Value, etc

#include "core/icommand_translator.h"  // for translator_t

#include "core/connection_settings/iconnection_settings.h"
#include "core/driver/idriver_remote.h"  // for IDriverRemote
#include "core/events/events.h"
#include "core/server/iserver_info.h"  // for IServerInfo (ptr only), etc

#include "global/global.h"  // for FastoObject (ptr only), etc

namespace fastonosql {
namespace core {
class IDataBaseInfo;
}
}
namespace fastonosql {
namespace core {
namespace redis {
class DBConnection;
}
}
}

namespace fastonosql {
namespace core {
namespace redis {

class Driver : public IDriverRemote {
  Q_OBJECT
 public:
  explicit Driver(IConnectionSettingsBaseSPtr settings);
  virtual ~Driver();

  virtual bool IsInterrupted() const override;
  virtual void SetInterrupted(bool interrupted) override;

  virtual translator_t Translator() const override;

  virtual bool IsConnected() const override;
  virtual bool IsAuthenticated() const override;
  virtual common::net::HostAndPort Host() const override;
  virtual std::string NsSeparator() const override;
  virtual std::string Delimiter() const override;

 private:
  virtual void InitImpl() override;
  virtual void ClearImpl() override;

  virtual FastoObjectCommandIPtr CreateCommand(FastoObject* parent,
                                               const std::string& input,
                                               common::Value::CommandLoggingType ct) override;

  virtual FastoObjectCommandIPtr CreateCommandFast(const std::string& input,
                                                   common::Value::CommandLoggingType ct) override;

  virtual common::Error SyncConnect() override WARN_UNUSED_RESULT;
  virtual common::Error SyncDisconnect() override WARN_UNUSED_RESULT;

  virtual common::Error ExecuteImpl(int argc, const char** argv, FastoObject* out) override;

  virtual common::Error CurrentServerInfo(IServerInfo** info) override;
  virtual common::Error CurrentDataBaseInfo(IDataBaseInfo** info) override;

  virtual void HandleLoadDatabaseInfosEvent(events::LoadDatabasesInfoRequestEvent* ev) override;
  virtual void HandleLoadServerPropertyEvent(events::ServerPropertyInfoRequestEvent* ev) override;
  virtual void HandleServerPropertyChangeEvent(
      events::ChangeServerPropertyInfoRequestEvent* ev) override;
  virtual void HandleProcessCommandLineArgsEvent(
      events::ProcessConfigArgsRequestEvent* ev) override;
  virtual void HandleShutdownEvent(events::ShutDownRequestEvent* ev) override;
  virtual void HandleBackupEvent(events::BackupRequestEvent* ev) override;
  virtual void HandleExportEvent(events::ExportRequestEvent* ev) override;
  virtual void HandleChangePasswordEvent(events::ChangePasswordRequestEvent* ev) override;
  virtual void HandleChangeMaxConnectionEvent(events::ChangeMaxConnectionRequestEvent* ev) override;

  virtual void HandleLoadDatabaseContentEvent(events::LoadDatabaseContentRequestEvent* ev) override;
  virtual void HandleSetDefaultDatabaseEvent(events::SetDefaultDatabaseRequestEvent* ev) override;

  virtual IServerInfoSPtr MakeServerInfoFromString(const std::string& val) override;

  DBConnection* const impl_;

  common::Error InteracteveMode(events::ProcessConfigArgsRequestEvent* ev);
  common::Error LatencyMode(events::ProcessConfigArgsRequestEvent* ev);
  common::Error SlaveMode(events::ProcessConfigArgsRequestEvent* ev);
  common::Error GetRDBMode(events::ProcessConfigArgsRequestEvent* ev);
  common::Error FindBigKeysMode(events::ProcessConfigArgsRequestEvent* ev);
  common::Error StatMode(events::ProcessConfigArgsRequestEvent* ev);
  common::Error ScanMode(events::ProcessConfigArgsRequestEvent* ev);
};

}  // namespace redis
}  // namespace core
}  // namespace fastonosql
