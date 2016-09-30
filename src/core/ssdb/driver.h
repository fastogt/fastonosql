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

#include "core/connection_settings.h"  // for IConnectionSettingsBaseSPtr
#include "core/events/events.h"        // for ClearDatabaseRequestEvent, etc
#include "core/idriver.h"              // for IDriverRemote
#include "core/types.h"                // for IDataBaseInfo (ptr only), etc

namespace fastonosql {
namespace core {
namespace ssdb {

class DBConnection;

class Driver : public IDriverRemote {
  Q_OBJECT
 public:
  explicit Driver(IConnectionSettingsBaseSPtr settings);
  virtual ~Driver();

  virtual bool isInterrupted() const override;
  virtual void setInterrupted(bool interrupted) override;

  virtual translator_t translator() const;

  virtual bool isConnected() const;
  virtual bool isAuthenticated() const;
  virtual common::net::HostAndPort host() const;
  virtual std::string nsSeparator() const;
  virtual std::string delimiter() const;

 private:
  virtual void initImpl();
  virtual void clearImpl();

  virtual FastoObjectCommandIPtr createCommand(FastoObject* parent,
                                               const std::string& input,
                                               common::Value::CommandLoggingType ct) override;

  virtual FastoObjectCommandIPtr createCommandFast(const std::string& input,
                                                   common::Value::CommandLoggingType ct) override;

  virtual common::Error syncConnect() override WARN_UNUSED_RESULT;
  virtual common::Error syncDisconnect() override WARN_UNUSED_RESULT;

  virtual common::Error executeImpl(int argc, const char** argv, FastoObject* out);
  virtual common::Error serverInfo(IServerInfo** info);
  virtual common::Error currentDataBaseInfo(IDataBaseInfo** info);

  virtual void handleProcessCommandLineArgs(events::ProcessConfigArgsRequestEvent* ev);

  virtual void handleLoadDatabaseContentEvent(events::LoadDatabaseContentRequestEvent* ev);
  virtual void handleClearDatabaseEvent(events::ClearDatabaseRequestEvent* ev);

  IServerInfoSPtr makeServerInfoFromString(const std::string& val);

 private:
  DBConnection* const impl_;
};

}  // namespace ssdb
}  // namespace core
}  // namespace fastonosql
