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

#include <common/error.h>      // for Error
#include <common/macros.h>     // for WARN_UNUSED_RESULT
#include <common/net/types.h>  // for HostAndPort

#include "proxy/connection_settings/iconnection_settings.h"  // for IConnectionSettingsBaseSPtr
#include "proxy/driver/idriver_remote.h"                     // for IDriverRemote
#include "proxy/events/events.h"                             // for CommandRequestEvent, etc

namespace fastonosql {
namespace core {
namespace memcached {
class DBConnection;
}
}  // namespace core
}  // namespace fastonosql

namespace fastonosql {
namespace proxy {
namespace memcached {

class Driver : public IDriverRemote {
  Q_OBJECT
 public:
  explicit Driver(IConnectionSettingsBaseSPtr settings);
  virtual ~Driver();

  virtual bool IsInterrupted() const override;
  virtual void SetInterrupted(bool interrupted) override;

  virtual core::translator_t GetTranslator() const override;

  virtual bool IsConnected() const override;
  virtual bool IsAuthenticated() const override;
  virtual common::net::HostAndPort GetHost() const override;
  virtual std::string GetNsSeparator() const override;
  virtual std::string GetDelimiter() const override;

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

  core::memcached::DBConnection* const impl_;
};

}  // namespace memcached
}  // namespace proxy
}  // namespace fastonosql
