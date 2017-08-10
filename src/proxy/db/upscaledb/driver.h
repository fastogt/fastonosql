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

#pragma once

#include <string>  // for string

#include <common/error.h>   // for Error
#include <common/macros.h>  // for WARN_UNUSED_RESULT
#include <common/value.h>   // for Value, etc

#include "core/icommand_translator.h"                        // for translator_t
#include "core/server/iserver_info.h"                        // for IServerInfo (ptr only), etc
#include "proxy/connection_settings/iconnection_settings.h"  // for IConnectionSettingsBaseSPtr
#include "proxy/driver/idriver_local.h"                      // for IDriverLocal
#include "proxy/events/events.h"                             // for ClearDatabaseRequestEvent, etc

#include "core/global.h"  // for FastoObject (ptr only), etc

namespace fastonosql {
namespace core {
namespace upscaledb {
class DBConnection;
}
}  // namespace core
}  // namespace fastonosql

namespace fastonosql {
namespace proxy {
namespace upscaledb {

class Driver : public IDriverLocal {
  Q_OBJECT
 public:
  explicit Driver(IConnectionSettingsBaseSPtr settings);
  virtual ~Driver();

  virtual bool IsInterrupted() const override;
  virtual void SetInterrupted(bool interrupted) override;

  virtual core::translator_t Translator() const override;

  virtual bool IsConnected() const override;
  virtual bool IsAuthenticated() const override;
  virtual std::string Path() const override;
  virtual std::string NsSeparator() const override;
  virtual std::string Delimiter() const override;

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
  virtual common::Error CurrentServerInfo(core::IServerInfo** info) override;
  virtual common::Error CurrentDataBaseInfo(core::IDataBaseInfo** info) override;

  virtual void HandleLoadDatabaseContentEvent(events::LoadDatabaseContentRequestEvent* ev) override;

  virtual core::IServerInfoSPtr MakeServerInfoFromString(const std::string& val) override;

  core::upscaledb::DBConnection* const impl_;
};

}  // namespace upscaledb
}  // namespace proxy
}  // namespace fastonosql
