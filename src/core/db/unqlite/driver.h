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

#include <common/error.h>   // for Error
#include <common/macros.h>  // for WARN_UNUSED_RESULT
#include <common/value.h>   // for Value, etc

#include "core/connection_settings/iconnection_settings.h"  // for IConnectionSettingsBaseSPtr
#include "core/events/events.h"                             // for ClearDatabaseRequestEvent, etc
#include "core/driver/idriver_local.h"                      // for IDriverLocal
#include "core/icommand_translator.h"                       // for translator_t
#include "core/server/iserver_info.h"                       // for IServerInfo (ptr only), etc

#include "global/global.h"  // for FastoObject (ptr only), etc

namespace fastonosql {
namespace core {
class IDataBaseInfo;
}
}

namespace fastonosql {
namespace core {
namespace unqlite {

class DBConnection;

class Driver : public IDriverLocal {
  Q_OBJECT
 public:
  explicit Driver(IConnectionSettingsBaseSPtr settings);
  virtual ~Driver();

  virtual bool IsInterrupted() const override;
  virtual void SetInterrupted(bool interrupted) override;

  virtual translator_t Translator() const override;

  virtual bool IsConnected() const override;
  virtual bool IsAuthenticated() const override;
  virtual std::string Path() const override;
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

  virtual void HandleProcessCommandLineArgsEvent(
      events::ProcessConfigArgsRequestEvent* ev) override;

  virtual void HandleLoadDatabaseContentEvent(events::LoadDatabaseContentRequestEvent* ev) override;

  virtual IServerInfoSPtr MakeServerInfoFromString(const std::string& val) override;

 private:
  DBConnection* const impl_;
};

}  // namespace unqlite
}  // namespace core
}  // namespace fastonosql
