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

#include "proxy/db/memcached/driver.h"

#include <stddef.h>  // for size_t
#include <memory>    // for __shared_ptr
#include <string>    // for string

#include <common/convert2string.h>
#include <common/log_levels.h>   // for LEVEL_LOG::L_WARNING
#include <common/qt/utils_qt.h>  // for Event<>::value_type
#include <common/sprintf.h>      // for MemSPrintf
#include <common/value.h>        // for ErrorValue, etc

#include "core/connection_types.h"
#include "core/db_key.h"                   // for NDbKValue, NValue, NKey
#include "proxy/command/command.h"         // for CreateCommand, etc
#include "proxy/command/command_logger.h"  // for LOG_COMMAND
#include "proxy/events/events_info.h"

#include "core/db/memcached/config.h"                // for Config
#include "core/db/memcached/db_connection.h"         // for DBConnection
#include "core/db/memcached/server_info.h"           // for ServerInfo, etc
#include "proxy/db/memcached/command.h"              // for Command
#include "proxy/db/memcached/connection_settings.h"  // for ConnectionSettings
#include "proxy/db/memcached/database.h"             // for DataBaseInfo

#include "core/global.h"  // for FastoObject::childs_t, etc

#define MEMCACHED_INFO_REQUEST "STATS"
#define MEMCACHED_GET_KEYS_PATTERN_1ARGS_I "KEYS a z %d"

namespace fastonosql {
namespace proxy {
namespace memcached {

Driver::Driver(IConnectionSettingsBaseSPtr settings)
    : IDriverRemote(settings), impl_(new core::memcached::DBConnection(this)) {
  COMPILE_ASSERT(core::memcached::DBConnection::connection_t == core::MEMCACHED,
                 "DBConnection must be the same type as Driver!");
  CHECK(Type() == core::MEMCACHED);
}

Driver::~Driver() {
  delete impl_;
}

bool Driver::IsInterrupted() const {
  return impl_->IsInterrupted();
}

void Driver::SetInterrupted(bool interrupted) {
  return impl_->SetInterrupted(interrupted);
}

core::translator_t Driver::Translator() const {
  return impl_->Translator();
}

bool Driver::IsConnected() const {
  return impl_->IsConnected();
}

bool Driver::IsAuthenticated() const {
  return impl_->IsConnected();
}

common::net::HostAndPort Driver::Host() const {
  core::memcached::Config conf = impl_->config();
  return conf.host;
}

std::string Driver::NsSeparator() const {
  return impl_->NsSeparator();
}

std::string Driver::Delimiter() const {
  return impl_->Delimiter();
}

void Driver::InitImpl() {}

void Driver::ClearImpl() {}

core::FastoObjectCommandIPtr Driver::CreateCommand(core::FastoObject* parent,
                                                   const core::command_buffer_t& input,
                                                   core::CmdLoggingType ct) {
  return proxy::CreateCommand<memcached::Command>(parent, input, ct);
}

core::FastoObjectCommandIPtr Driver::CreateCommandFast(const core::command_buffer_t& input, core::CmdLoggingType ct) {
  return proxy::CreateCommandFast<memcached::Command>(input, ct);
}

common::Error Driver::SyncConnect() {
  ConnectionSettings* set = dynamic_cast<ConnectionSettings*>(settings_.get());  // +
  CHECK(set);
  return impl_->Connect(set->Info());
}

common::Error Driver::SyncDisconnect() {
  return impl_->Disconnect();
}

common::Error Driver::ExecuteImpl(const core::command_buffer_t& command, core::FastoObject* out) {
  return impl_->Execute(command, out);
}

common::Error Driver::CurrentServerInfo(core::IServerInfo** info) {
  core::FastoObjectCommandIPtr cmd = CreateCommandFast(MAKE_COMMAND_BUFFER(MEMCACHED_INFO_REQUEST), core::C_INNER);
  LOG_COMMAND(cmd);
  core::memcached::ServerInfo::Stats cm;
  common::Error err = impl_->Info(std::string(), &cm);
  if (err && err->IsError()) {
    return err;
  }

  *info = new core::memcached::ServerInfo(cm);
  return common::Error();
}

common::Error Driver::CurrentDataBaseInfo(core::IDataBaseInfo** info) {
  if (!info) {
    DNOTREACHED();
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  return impl_->Select(impl_->CurrentDBName(), info);
}

void Driver::HandleLoadDatabaseContentEvent(events::LoadDatabaseContentRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::LoadDatabaseContentResponceEvent::value_type res(ev->value());
  const core::command_buffer_t pattern_result =
      core::internal::GetKeysPattern(res.cursor_in, res.pattern, res.count_keys);
  core::FastoObjectCommandIPtr cmd = CreateCommandFast(pattern_result, core::C_INNER);
  NotifyProgress(sender, 50);
  common::Error er = Execute(cmd);
  if (er && er->IsError()) {
    res.setErrorInfo(er);
  } else {
    core::FastoObject::childs_t rchildrens = cmd->Childrens();
    if (rchildrens.size()) {
      CHECK_EQ(rchildrens.size(), 1);
      core::FastoObjectArray* array = dynamic_cast<core::FastoObjectArray*>(rchildrens[0].get());  // +
      if (!array) {
        goto done;
      }

      common::ArrayValue* arm = array->Array();
      if (!arm->GetSize()) {
        goto done;
      }

      std::string cursor;
      bool isok = arm->GetString(0, &cursor);
      if (!isok) {
        goto done;
      }

      uint64_t lcursor;
      if (common::ConvertFromString(cursor, &lcursor)) {
        res.cursor_out = lcursor;
      }

      rchildrens = array->Childrens();
      if (!rchildrens.size()) {
        goto done;
      }

      core::FastoObject* obj = rchildrens[0].get();
      core::FastoObjectArray* arr = dynamic_cast<core::FastoObjectArray*>(obj);  // +
      if (!arr) {
        goto done;
      }

      common::ArrayValue* ar = arr->Array();
      if (ar->IsEmpty()) {
        goto done;
      }

      for (size_t i = 0; i < ar->GetSize(); ++i) {
        std::string key_str;
        if (ar->GetString(i, &key_str)) {
          core::key_t key(key_str);
          core::NKey k(key);
          core::command_buffer_writer_t wr;
          wr << MAKE_COMMAND_BUFFER("TTL ") << key.ToString();
          core::FastoObjectCommandIPtr cmd_ttl = CreateCommandFast(wr.str(), core::C_INNER);
          LOG_COMMAND(cmd_ttl);
          core::ttl_t ttl = NO_TTL;
          common::Error err = impl_->TTL(key, &ttl);
          if (err && err->IsError()) {
            k.SetTTL(NO_TTL);
          } else {
            k.SetTTL(ttl);
          }
          core::NValue empty_val(common::Value::CreateEmptyValueFromType(common::Value::TYPE_STRING));
          core::NDbKValue ress(k, empty_val);
          res.keys.push_back(ress);
        }
      }

      common::Error err = impl_->DBkcount(&res.db_keys_count);
      DCHECK(!err);
    }
  }
done:
  NotifyProgress(sender, 75);
  Reply(sender, new events::LoadDatabaseContentResponceEvent(this, res));
  NotifyProgress(sender, 100);
}

core::IServerInfoSPtr Driver::MakeServerInfoFromString(const std::string& val) {
  core::IServerInfoSPtr res(core::memcached::MakeMemcachedServerInfo(val));
  return res;
}

}  // namespace memcached
}  // namespace proxy
}  // namespace fastonosql
