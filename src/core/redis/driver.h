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

#include "core/connection_settings/connection_settings.h"
#include "core/driver/idriver_remote.h"  // for IDriverRemote
#include "core/events/events.h"
#include "core/server/iserver_info.h"             // for IServerInfo (ptr only), etc
#include "core/translator/icommand_translator.h"  // for translator_t

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

  virtual translator_t Translator() const;

  virtual bool IsConnected() const;
  virtual bool IsAuthenticated() const;
  virtual common::net::HostAndPort host() const;
  virtual std::string NsSeparator() const;
  virtual std::string Delimiter() const;

 private:
  virtual void InitImpl();
  virtual void ClearImpl();

  virtual FastoObjectCommandIPtr createCommand(FastoObject* parent,
                                               const std::string& input,
                                               common::Value::CommandLoggingType ct) override;

  virtual FastoObjectCommandIPtr createCommandFast(const std::string& input,
                                                   common::Value::CommandLoggingType ct) override;

  virtual common::Error SyncConnect() override WARN_UNUSED_RESULT;
  virtual common::Error SyncDisconnect() override WARN_UNUSED_RESULT;

  virtual common::Error ExecuteImpl(int argc, const char** argv, FastoObject* out);

  virtual common::Error CurrentServerInfo(IServerInfo** info);
  virtual common::Error CurrentDataBaseInfo(IDataBaseInfo** info);

  virtual void HandleLoadDatabaseInfosEvent(events::LoadDatabasesInfoRequestEvent* ev);
  virtual void HandleLoadServerPropertyEvent(events::ServerPropertyInfoRequestEvent* ev);
  virtual void HandleServerPropertyChangeEvent(events::ChangeServerPropertyInfoRequestEvent* ev);
  virtual void HandleProcessCommandLineArgsEvent(events::ProcessConfigArgsRequestEvent* ev);
  virtual void HandleShutdownEvent(events::ShutDownRequestEvent* ev);
  virtual void HandleBackupEvent(events::BackupRequestEvent* ev);
  virtual void HandleExportEvent(events::ExportRequestEvent* ev);
  virtual void HandleChangePasswordEvent(events::ChangePasswordRequestEvent* ev);
  virtual void HandleChangeMaxConnectionEvent(events::ChangeMaxConnectionRequestEvent* ev);

  virtual void HandleLoadDatabaseContentEvent(events::LoadDatabaseContentRequestEvent* ev);
  virtual void HandleClearDatabaseEvent(events::ClearDatabaseRequestEvent* ev);
  virtual void HandleSetDefaultDatabaseEvent(events::SetDefaultDatabaseRequestEvent* ev);

  IServerInfoSPtr MakeServerInfoFromString(const std::string& val);

  DBConnection* const impl_;

  common::Error interacteveMode(events::ProcessConfigArgsRequestEvent* ev);
  common::Error latencyMode(events::ProcessConfigArgsRequestEvent* ev);
  common::Error slaveMode(events::ProcessConfigArgsRequestEvent* ev);
  common::Error getRDBMode(events::ProcessConfigArgsRequestEvent* ev);
  common::Error findBigKeysMode(events::ProcessConfigArgsRequestEvent* ev);
  common::Error statMode(events::ProcessConfigArgsRequestEvent* ev);
  common::Error scanMode(events::ProcessConfigArgsRequestEvent* ev);
};

}  // namespace redis
}  // namespace core
}  // namespace fastonosql
