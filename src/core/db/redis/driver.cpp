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

#include "core/db/redis/driver.h"

#include <stddef.h>  // for size_t
#include <stdint.h>  // for uint32_t

#include <memory>  // for __shared_ptr, shared_ptr
#include <vector>  // for vector

#include <common/convert2string.h>  // for ConvertFromString, etc
#include <common/file_system.h>     // for copy_file
#include <common/intrusive_ptr.h>   // for intrusive_ptr
#include <common/qt/utils_qt.h>     // for Event<>::value_type
#include <common/sprintf.h>         // for MemSPrintf
#include <common/value.h>           // for Value, ErrorValue, etc

#include "core/command/command.h"  // for CreateCommand, etc
#include "core/connection_types.h"
#include "core/db_key.h"  // for NDbKValue, NValue, ttl_t, etc
#include "core/events/events_info.h"
#include "core/server_property_info.h"     // for MakeServerProperty, etc
#include "core/database/idatabase_info.h"  // for IDataBaseInfoSPtr, etc
#include "core/driver/root_locker.h"       // for RootLocker

#include "core/db/redis/db_connection.h"        // for DBConnection, INFO_REQUEST, etc
#include "core/db/redis/command.h"              // for Command
#include "core/db/redis/config.h"               // for Config
#include "core/db/redis/connection_settings.h"  // for ConnectionSettings
#include "core/db/redis/database.h"             // for DataBaseInfo
#include "core/db/redis/server_info.h"          // for ServerInfo, etc

#include "global/global.h"  // for FastoObjectCommandIPtr, etc

#define REDIS_SHUTDOWN "SHUTDOWN"
#define REDIS_BACKUP "SAVE"
#define REDIS_SET_PASSWORD_1ARGS_S "CONFIG SET requirepass %s"
#define REDIS_SET_MAX_CONNECTIONS_1ARGS_I "CONFIG SET maxclients %d"
#define REDIS_GET_DATABASES "CONFIG GET databases"
#define REDIS_GET_PROPERTY_SERVER "CONFIG GET *"

#define REDIS_SET_DEFAULT_DATABASE_PATTERN_1ARGS_S "SELECT %s"
#define REDIS_FLUSHDB "FLUSHDB"

namespace {

common::Value::Type convertFromStringRType(const std::string& type) {
  if (type.empty()) {
    return common::Value::TYPE_NULL;
  }

  if (type == "string") {
    return common::Value::TYPE_STRING;
  } else if (type == "list") {
    return common::Value::TYPE_ARRAY;
  } else if (type == "set") {
    return common::Value::TYPE_SET;
  } else if (type == "hash") {
    return common::Value::TYPE_HASH;
  } else if (type == "zset") {
    return common::Value::TYPE_ZSET;
  } else {
    return common::Value::TYPE_NULL;
  }
}

}  // namespace

namespace fastonosql {
namespace core {
namespace redis {

Driver::Driver(IConnectionSettingsBaseSPtr settings)
    : IDriverRemote(settings), impl_(new DBConnection(this)) {
  COMPILE_ASSERT(DBConnection::connection_t == REDIS,
                 "DBConnection must be the same type as Driver!");
  CHECK(Type() == REDIS);
}

Driver::~Driver() {
  delete impl_;
}

common::net::HostAndPort Driver::Host() const {
  Config conf = impl_->config();
  return conf.host;
}

std::string Driver::NsSeparator() const {
  return impl_->NsSeparator();
}

std::string Driver::Delimiter() const {
  return impl_->Delimiter();
}

bool Driver::IsInterrupted() const {
  return impl_->IsInterrupted();
}

void Driver::SetInterrupted(bool interrupted) {
  return impl_->SetInterrupted(interrupted);
}

translator_t Driver::Translator() const {
  return impl_->Translator();
}

bool Driver::IsConnected() const {
  return impl_->IsConnected();
}

bool Driver::IsAuthenticated() const {
  return impl_->IsAuthenticated();
}

void Driver::InitImpl() {}

void Driver::ClearImpl() {}

FastoObjectCommandIPtr Driver::CreateCommand(FastoObject* parent,
                                             const std::string& input,
                                             common::Value::CommandLoggingType ct) {
  return fastonosql::core::CreateCommand<Command>(parent, input, ct);
}

FastoObjectCommandIPtr Driver::CreateCommandFast(const std::string& input,
                                                 common::Value::CommandLoggingType ct) {
  return fastonosql::core::CreateCommandFast<Command>(input, ct);
}

common::Error Driver::SyncConnect() {
  ConnectionSettings* set = dynamic_cast<ConnectionSettings*>(settings_.get());  // +
  CHECK(set);
  RConfig rconf(set->Info(), set->SSHInfo());
  return impl_->Connect(rconf);
}

common::Error Driver::SyncDisconnect() {
  return impl_->Disconnect();
}

common::Error Driver::ExecuteImpl(int argc, const char** argv, FastoObject* out) {
  return impl_->Execute(argc, argv, out);
}

common::Error Driver::CurrentServerInfo(IServerInfo** info) {
  FastoObjectCommandIPtr cmd = CreateCommandFast(INFO_REQUEST, common::Value::C_INNER);
  common::Error err = Execute(cmd.get());
  if (err && err->isError()) {
    return err;
  }

  std::string content = common::ConvertToString(cmd.get());
  *info = MakeRedisServerInfo(content);

  if (!*info) {
    return common::make_error_value("Invalid " INFO_REQUEST " command output",
                                    common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error Driver::CurrentDataBaseInfo(IDataBaseInfo** info) {
  if (!info) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  return impl_->Select(impl_->CurrentDBName(), info);
}

void Driver::HandleProcessCommandLineArgsEvent(events::ProcessConfigArgsRequestEvent* ev) {
  const Config conf = impl_->config();
  /* Latency mode */
  if (conf.latency_mode) {
    LatencyMode(ev);
  }

  /* Slave mode */
  if (conf.slave_mode) {
    SlaveMode(ev);
  }

  /* Get RDB mode. */
  if (conf.getrdb_mode) {
    GetRDBMode(ev);
  }

  /* Find big keys */
  if (conf.bigkeys) {
    FindBigKeysMode(ev);
  }

  /* Stat mode */
  if (conf.stat_mode) {
    StatMode(ev);
  }

  /* Scan mode */
  if (conf.scan_mode) {
    ScanMode(ev);
  }

  InteracteveMode(ev);

  QObject* sender = ev->sender();
  events::ProcessConfigArgsResponceEvent::value_type res(ev->value());
  Reply(sender, new events::ProcessConfigArgsResponceEvent(this, res));
}

void Driver::HandleShutdownEvent(events::ShutDownRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::ShutDownResponceEvent::value_type res(ev->value());
  NotifyProgress(sender, 25);
  FastoObjectCommandIPtr cmd = CreateCommandFast(REDIS_SHUTDOWN, common::Value::C_INNER);
  common::Error er = Execute(cmd);
  if (er && er->isError()) {
    res.setErrorInfo(er);
  }
  NotifyProgress(sender, 75);
  Reply(sender, new events::ShutDownResponceEvent(this, res));
  NotifyProgress(sender, 100);
}

void Driver::HandleBackupEvent(events::BackupRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::BackupResponceEvent::value_type res(ev->value());
  NotifyProgress(sender, 25);
  FastoObjectCommandIPtr cmd = CreateCommandFast(REDIS_BACKUP, common::Value::C_INNER);
  common::Error er = Execute(cmd);
  if (er && er->isError()) {
    res.setErrorInfo(er);
  } else {
    common::Error err = common::file_system::copy_file("/var/lib/redis/dump.rdb", res.path);
    if (err && err->isError()) {
      res.setErrorInfo(err);
    }
  }
  NotifyProgress(sender, 75);
  Reply(sender, new events::BackupResponceEvent(this, res));
  NotifyProgress(sender, 100);
}

void Driver::HandleExportEvent(events::ExportRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::ExportResponceEvent::value_type res(ev->value());
  NotifyProgress(sender, 25);
  common::Error err = common::file_system::copy_file(res.path, "/var/lib/redis/dump.rdb");
  if (err && err->isError()) {
    res.setErrorInfo(err);
  }
  NotifyProgress(sender, 75);
  Reply(sender, new events::ExportResponceEvent(this, res));
  NotifyProgress(sender, 100);
}

void Driver::HandleChangePasswordEvent(events::ChangePasswordRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::ChangePasswordResponceEvent::value_type res(ev->value());
  NotifyProgress(sender, 25);
  std::string patternResult = common::MemSPrintf(REDIS_SET_PASSWORD_1ARGS_S, res.new_password);
  FastoObjectCommandIPtr cmd = CreateCommandFast(patternResult, common::Value::C_INNER);
  common::Error er = Execute(cmd);
  if (er && er->isError()) {
    res.setErrorInfo(er);
  }

  NotifyProgress(sender, 75);
  Reply(sender, new events::ChangePasswordResponceEvent(this, res));
  NotifyProgress(sender, 100);
}

void Driver::HandleChangeMaxConnectionEvent(events::ChangeMaxConnectionRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::ChangeMaxConnectionResponceEvent::value_type res(ev->value());
  NotifyProgress(sender, 25);
  std::string patternResult =
      common::MemSPrintf(REDIS_SET_MAX_CONNECTIONS_1ARGS_I, res.max_connection);
  FastoObjectCommandIPtr cmd = CreateCommandFast(patternResult, common::Value::C_INNER);
  common::Error er = Execute(cmd);
  if (er && er->isError()) {
    res.setErrorInfo(er);
  }

  NotifyProgress(sender, 75);
  Reply(sender, new events::ChangeMaxConnectionResponceEvent(this, res));
  NotifyProgress(sender, 100);
}

common::Error Driver::InteracteveMode(events::ProcessConfigArgsRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::EnterModeEvent::value_type res(this, InteractiveMode);
  Reply(sender, new events::EnterModeEvent(this, res));

  events::LeaveModeEvent::value_type res2(this, InteractiveMode);
  Reply(sender, new events::LeaveModeEvent(this, res2));
  NotifyProgress(sender, 100);
  return common::Error();
}

common::Error Driver::LatencyMode(events::ProcessConfigArgsRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::EnterModeEvent::value_type resEv(this, core::LatencyMode);
  Reply(sender, new events::EnterModeEvent(this, resEv));

  events::LeaveModeEvent::value_type res(this, core::LatencyMode);
  RootLocker lock(this, sender, LATENCY_REQUEST, false);

  FastoObjectIPtr obj = lock.Root();
  common::Error er = impl_->LatencyMode(obj.get());
  if (er && er->isError()) {
    res.setErrorInfo(er);
  }

  Reply(sender, new events::LeaveModeEvent(this, res));
  NotifyProgress(sender, 100);
  return er;
}

common::Error Driver::SlaveMode(events::ProcessConfigArgsRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::EnterModeEvent::value_type resEv(this, core::SlaveMode);
  Reply(sender, new events::EnterModeEvent(this, resEv));

  events::LeaveModeEvent::value_type res(this, core::SlaveMode);
  RootLocker lock(this, sender, SYNC_REQUEST, false);

  FastoObjectIPtr obj = lock.Root();
  common::Error er = impl_->SlaveMode(obj.get());
  if (er && er->isError()) {
    res.setErrorInfo(er);
  }

  Reply(sender, new events::LeaveModeEvent(this, res));
  NotifyProgress(sender, 100);
  return er;
}

common::Error Driver::GetRDBMode(events::ProcessConfigArgsRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::EnterModeEvent::value_type resEv(this, core::GetRDBMode);
  Reply(sender, new events::EnterModeEvent(this, resEv));

  events::LeaveModeEvent::value_type res(this, core::GetRDBMode);
  RootLocker lock(this, sender, RDM_REQUEST, false);

  FastoObjectIPtr obj = lock.Root();
  common::Error er = impl_->GetRDB(obj.get());
  if (er && er->isError()) {
    res.setErrorInfo(er);
  }

  Reply(sender, new events::LeaveModeEvent(this, res));
  NotifyProgress(sender, 100);
  return er;
}

common::Error Driver::FindBigKeysMode(events::ProcessConfigArgsRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::EnterModeEvent::value_type resEv(this, core::FindBigKeysMode);
  Reply(sender, new events::EnterModeEvent(this, resEv));

  events::LeaveModeEvent::value_type res(this, core::FindBigKeysMode);
  RootLocker lock(this, sender, FIND_BIG_KEYS_REQUEST, false);

  FastoObjectIPtr obj = lock.Root();
  common::Error er = impl_->FindBigKeys(obj.get());
  if (er && er->isError()) {
    res.setErrorInfo(er);
  }

  Reply(sender, new events::LeaveModeEvent(this, res));
  NotifyProgress(sender, 100);
  return er;
}

common::Error Driver::StatMode(events::ProcessConfigArgsRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::EnterModeEvent::value_type resEv(this, core::StatMode);
  Reply(sender, new events::EnterModeEvent(this, resEv));

  events::LeaveModeEvent::value_type res(this, core::StatMode);
  RootLocker lock(this, sender, STAT_MODE_REQUEST, false);

  FastoObjectIPtr obj = lock.Root();
  common::Error er = impl_->StatMode(obj.get());
  if (er && er->isError()) {
    res.setErrorInfo(er);
  }

  Reply(sender, new events::LeaveModeEvent(this, res));
  NotifyProgress(sender, 100);
  return er;
}

common::Error Driver::ScanMode(events::ProcessConfigArgsRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::EnterModeEvent::value_type resEv(this, core::ScanMode);
  Reply(sender, new events::EnterModeEvent(this, resEv));

  events::LeaveModeEvent::value_type res(this, core::ScanMode);
  RootLocker lock(this, sender, SCAN_MODE_REQUEST, false);

  FastoObjectIPtr obj = lock.Root();
  common::Error er = impl_->ScanMode(obj.get());
  if (er && er->isError()) {
    res.setErrorInfo(er);
  }

  Reply(sender, new events::LeaveModeEvent(this, res));
  NotifyProgress(sender, 100);
  return er;
}

void Driver::HandleLoadDatabaseInfosEvent(events::LoadDatabasesInfoRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::LoadDatabasesInfoResponceEvent::value_type res(ev->value());
  FastoObjectCommandIPtr cmd = CreateCommandFast(REDIS_GET_DATABASES, common::Value::C_INNER);
  NotifyProgress(sender, 50);

  IDataBaseInfo* info = nullptr;
  common::Error err = CurrentDataBaseInfo(&info);
  if (err && err->isError()) {
    res.setErrorInfo(err);
    NotifyProgress(sender, 75);
    Reply(sender, new events::LoadDatabasesInfoResponceEvent(this, res));
    NotifyProgress(sender, 100);
    return;
  }

  err = Execute(cmd.get());
  if (err && err->isError()) {
    res.setErrorInfo(err);
    NotifyProgress(sender, 75);
    Reply(sender, new events::LoadDatabasesInfoResponceEvent(this, res));
    NotifyProgress(sender, 100);
    return;
  }

  FastoObject::childs_t rchildrens = cmd->Childrens();
  CHECK_EQ(rchildrens.size(), 1);
  FastoObjectArray* array = dynamic_cast<FastoObjectArray*>(rchildrens[0].get());  // +
  CHECK(array);
  common::ArrayValue* ar = array->Array();
  CHECK(ar);

  IDataBaseInfoSPtr curdb(info);
  std::string scountDb;
  if (ar->getString(1, &scountDb)) {
    size_t countDb = common::ConvertFromString<size_t>(scountDb);
    if (countDb > 0) {
      for (size_t i = 0; i < countDb; ++i) {
        IDataBaseInfoSPtr dbInf(new DataBaseInfo(common::ConvertToString(i), false, 0));
        if (dbInf->Name() == curdb->Name()) {
          res.databases.push_back(curdb);
        } else {
          res.databases.push_back(dbInf);
        }
      }
    }
  } else {
    res.databases.push_back(curdb);
  }
  NotifyProgress(sender, 75);
  Reply(sender, new events::LoadDatabasesInfoResponceEvent(this, res));
  NotifyProgress(sender, 100);
}

void Driver::HandleLoadDatabaseContentEvent(events::LoadDatabaseContentRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::LoadDatabaseContentResponceEvent::value_type res(ev->value());
  std::string patternResult =
      common::MemSPrintf(GET_KEYS_PATTERN_3ARGS_ISI, res.cursor_in, res.pattern, res.count_keys);
  FastoObjectCommandIPtr cmd = CreateCommandFast(patternResult, common::Value::C_INNER);
  NotifyProgress(sender, 50);
  common::Error err = Execute(cmd);
  if (err && err->isError()) {
    res.setErrorInfo(err);
  } else {
    FastoObject::childs_t rchildrens = cmd->Childrens();
    if (rchildrens.size()) {
      CHECK_EQ(rchildrens.size(), 1);
      FastoObjectArray* array = dynamic_cast<FastoObjectArray*>(rchildrens[0].get());  // +
      if (!array) {
        goto done;
      }

      common::ArrayValue* arm = array->Array();
      if (!arm->size()) {
        goto done;
      }

      std::string cursor;
      bool isok = arm->getString(0, &cursor);
      if (!isok) {
        goto done;
      }

      res.cursor_out = common::ConvertFromString<uint32_t>(cursor);

      rchildrens = array->Childrens();
      if (!rchildrens.size()) {
        goto done;
      }

      FastoObject* obj = rchildrens[0].get();
      FastoObjectArray* arr = dynamic_cast<FastoObjectArray*>(obj);  // +
      if (!arr) {
        goto done;
      }

      common::ArrayValue* ar = arr->Array();
      if (ar->empty()) {
        goto done;
      }

      std::vector<FastoObjectCommandIPtr> cmds;
      cmds.reserve(ar->size() * 2);
      for (size_t i = 0; i < ar->size(); ++i) {
        std::string key;
        bool isok = ar->getString(i, &key);
        if (isok) {
          NKey k(key);
          NDbKValue dbv(k, NValue());
          cmds.push_back(CreateCommandFast("TYPE " + key, common::Value::C_INNER));
          cmds.push_back(CreateCommandFast("TTL " + key, common::Value::C_INNER));
          res.keys.push_back(dbv);
        }
      }

      err = impl_->ExecuteAsPipeline(cmds);
      if (err && err->isError()) {
        goto done;
      }

      for (size_t i = 0; i < res.keys.size(); ++i) {
        FastoObjectIPtr cmdType = cmds[i * 2];
        FastoObject::childs_t tchildrens = cmdType->Childrens();
        if (tchildrens.size()) {
          DCHECK_EQ(tchildrens.size(), 1);
          if (tchildrens.size() == 1) {
            std::string typeRedis = tchildrens[0]->ToString();
            common::Value::Type ctype = convertFromStringRType(typeRedis);
            common::ValueSPtr empty_val(common::Value::createEmptyValueFromType(ctype));
            res.keys[i].SetValue(empty_val);
          }
        }

        FastoObjectIPtr cmdType2 = cmds[i * 2 + 1];
        tchildrens = cmdType2->Childrens();
        if (tchildrens.size()) {
          DCHECK_EQ(tchildrens.size(), 1);
          if (tchildrens.size() == 1) {
            auto vttl = tchildrens[0]->Value();
            ttl_t ttl = 0;
            if (vttl->getAsInteger(&ttl)) {
              NKey key = res.keys[i].Key();
              key.SetTTL(ttl);
              res.keys[i].SetKey(key);
            }
          }
        }
      }

      err = impl_->DBkcount(&res.db_keys_count);
      DCHECK(!err);
    }
  }
done:
  NotifyProgress(sender, 75);
  Reply(sender, new events::LoadDatabaseContentResponceEvent(this, res));
  NotifyProgress(sender, 100);
}

void Driver::HandleLoadServerPropertyEvent(events::ServerPropertyInfoRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::ServerPropertyInfoResponceEvent::value_type res(ev->value());
  FastoObjectCommandIPtr cmd = CreateCommandFast(REDIS_GET_PROPERTY_SERVER, common::Value::C_INNER);
  NotifyProgress(sender, 50);
  common::Error er = Execute(cmd);
  if (er && er->isError()) {
    res.setErrorInfo(er);
  } else {
    FastoObject::childs_t ch = cmd->Childrens();
    if (ch.size()) {
      CHECK_EQ(ch.size(), 1);
      FastoObjectArray* array = dynamic_cast<FastoObjectArray*>(ch[0].get());  // +
      if (array) {
        res.info = MakeServerProperty(array);
      }
    }
  }
  NotifyProgress(sender, 75);
  Reply(sender, new events::ServerPropertyInfoResponceEvent(this, res));
  NotifyProgress(sender, 100);
}

void Driver::HandleServerPropertyChangeEvent(events::ChangeServerPropertyInfoRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::ChangeServerPropertyInfoResponceEvent::value_type res(ev->value());

  NotifyProgress(sender, 50);
  std::string changeRequest = "CONFIG SET " + res.new_item.first + " " + res.new_item.second;
  FastoObjectCommandIPtr cmd = CreateCommandFast(changeRequest, common::Value::C_INNER);
  common::Error er = Execute(cmd);
  if (er && er->isError()) {
    res.setErrorInfo(er);
  } else {
    res.is_change = true;
  }
  NotifyProgress(sender, 75);
  Reply(sender, new events::ChangeServerPropertyInfoResponceEvent(this, res));
  NotifyProgress(sender, 100);
}

IServerInfoSPtr Driver::MakeServerInfoFromString(const std::string& val) {
  IServerInfoSPtr res(MakeRedisServerInfo(val));
  return res;
}

}  // namespace redis
}  // namespace core
}  // namespace fastonosql
