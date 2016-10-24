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

#include "core/connection_settings/connection_settings.h"  // for IConnectionSettingsBaseSPtr
#include "core/events/events.h"                            // for ClearDatabaseRequestEvent, etc
#include "core/driver/idriver_local.h"                     // for IDriverLocal

namespace fastonosql {
namespace core {
namespace lmdb {

class DBConnection;

class Driver : public IDriverLocal {
  Q_OBJECT
 public:
  explicit Driver(IConnectionSettingsBaseSPtr settings);
  virtual ~Driver();

  virtual bool IsInterrupted() const override;
  virtual void SetInterrupted(bool interrupted) override;

  virtual translator_t Translator() const;

  virtual bool IsConnected() const;
  virtual bool IsAuthenticated() const;
  virtual std::string path() const;
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

  virtual void HandleProcessCommandLineArgsEvent(events::ProcessConfigArgsRequestEvent* ev);

  virtual void HandleLoadDatabaseContentEvent(events::LoadDatabaseContentRequestEvent* ev);
  virtual void HandleClearDatabaseEvent(events::ClearDatabaseRequestEvent* ev);

  IServerInfoSPtr MakeServerInfoFromString(const std::string& val);

  DBConnection* const impl_;
};

}  // namespace lmdb
}  // namespace core
}  // namespace fastonosql
