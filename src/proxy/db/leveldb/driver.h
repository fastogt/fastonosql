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
namespace leveldb {
class DBConnection;
}
}  // namespace core
}  // namespace fastonosql

namespace fastonosql {
namespace proxy {
namespace leveldb {

class Driver : public IDriverLocal {
  Q_OBJECT
 public:
  explicit Driver(IConnectionSettingsBaseSPtr settings);
  virtual ~Driver() override;

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
                                                     core::CmdLoggingType logging_type) override;

  virtual core::FastoObjectCommandIPtr CreateCommandFast(const core::command_buffer_t& input,
                                                         core::CmdLoggingType logging_type) override;

  virtual core::IDataBaseInfoSPtr CreateDatabaseInfo(const core::db_name_t& name,
                                                     bool is_default,
                                                     size_t size) override;

  virtual common::Error SyncConnect() override WARN_UNUSED_RESULT;
  virtual common::Error SyncDisconnect() override WARN_UNUSED_RESULT;

  virtual common::Error ExecuteImpl(const core::command_buffer_t& command,
                                    core::FastoObject* out) override WARN_UNUSED_RESULT;
  virtual common::Error DBkcountImpl(core::keys_limit_t* size) override WARN_UNUSED_RESULT;

  virtual common::Error GetCurrentServerInfo(core::IServerInfo** info) override;
  virtual common::Error GetServerCommands(std::vector<const core::CommandInfo*>* commands) override;
  virtual common::Error GetCurrentDataBaseInfo(core::IDataBaseInfo** info) override;

  virtual core::IServerInfoSPtr MakeServerInfoFromString(const std::string& val) override;

  core::leveldb::DBConnection* const impl_;
};

}  // namespace leveldb
}  // namespace proxy
}  // namespace fastonosql
