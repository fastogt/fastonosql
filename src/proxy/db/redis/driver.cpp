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

#include "proxy/db/redis/driver.h"

#include <sstream>

#include <common/convert2string.h>           // for ConvertFromString, etc
#include <common/file_system/file_system.h>  // for copy_file

#include "core/db/redis/database_info.h"  // for DataBaseInfo
#include "core/db/redis/db_connection.h"  // for DBConnection, INFO_REQUEST, etc

#include "proxy/command/command.h"  // for CreateCommand, etc
#include "proxy/command/command_logger.h"
#include "proxy/db/redis/command.h"              // for Command
#include "proxy/db/redis/connection_settings.h"  // for ConnectionSettings

#define REDIS_TYPE_COMMAND "TYPE"
#define REDIS_SHUTDOWN_COMMAND "SHUTDOWN"
#define REDIS_BACKUP_COMMAND "SAVE"
#define REDIS_SET_PASSWORD_COMMAND "CONFIG SET requirepass"
#define REDIS_SET_MAX_CONNECTIONS_COMMAND "CONFIG SET maxclients"
#define REDIS_GET_DATABASES_COMMAND "CONFIG GET databases"
#define REDIS_GET_PROPERTY_SERVER_COMMAND "CONFIG GET *"
#define REDIS_PUBSUB_CHANNELS_COMMAND "PUBSUB CHANNELS"
#define REDIS_PUBSUB_NUMSUB_COMMAND "PUBSUB NUMSUB"

#define REDIS_SET_DEFAULT_DATABASE_COMMAND_1ARGS_S "SELECT %s"

#define BACKUP_DEFAULT_PATH "/var/lib/redis/dump.rdb"
#define EXPORT_DEFAULT_PATH "/var/lib/redis/dump.rdb"

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
namespace proxy {
namespace redis {

Driver::Driver(IConnectionSettingsBaseSPtr settings)
    : IDriverRemote(settings), impl_(new core::redis::DBConnection(this)) {
  COMPILE_ASSERT(core::redis::DBConnection::connection_t == core::REDIS,
                 "DBConnection must be the same type as Driver!");
  CHECK(GetType() == core::REDIS);
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

core::translator_t Driver::GetTranslator() const {
  return impl_->GetTranslator();
}

bool Driver::IsConnected() const {
  return impl_->IsConnected();
}

bool Driver::IsAuthenticated() const {
  return impl_->IsAuthenticated();
}

void Driver::InitImpl() {}

void Driver::ClearImpl() {}

core::FastoObjectCommandIPtr Driver::CreateCommand(core::FastoObject* parent,
                                                   const core::command_buffer_t& input,
                                                   core::CmdLoggingType ct) {
  return proxy::CreateCommand<Command>(parent, input, ct);
}

core::FastoObjectCommandIPtr Driver::CreateCommandFast(const core::command_buffer_t& input, core::CmdLoggingType ct) {
  return proxy::CreateCommandFast<Command>(input, ct);
}

common::Error Driver::SyncConnect() {
  auto redis_settings = GetSpecificSettings<ConnectionSettings>();
  core::redis::RConfig rconf(redis_settings->GetInfo(), redis_settings->GetSSHInfo());
  return impl_->Connect(rconf);
}

common::Error Driver::SyncDisconnect() {
  return impl_->Disconnect();
}

common::Error Driver::ExecuteImpl(const core::command_buffer_t& command, core::FastoObject* out) {
  return impl_->Execute(command, out);
}

common::Error Driver::CurrentServerInfo(core::IServerInfo** info) {
  core::FastoObjectCommandIPtr cmd = CreateCommandFast(DB_INFO_COMMAND, core::C_INNER);
  common::Error err = Execute(cmd.get());
  if (err) {
    return err;
  }

  std::string content = common::ConvertToString(cmd.get());
  core::IServerInfo* linfo = core::redis::MakeRedisServerInfo(content);

  if (!linfo) {
    return common::make_error("Invalid " DB_INFO_COMMAND " command output");
  }

  *info = linfo;
  return common::Error();
}

common::Error Driver::CurrentDataBaseInfo(core::IDataBaseInfo** info) {
  if (!info) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  return impl_->Select(impl_->GetCurrentDBName(), info);
}

void Driver::HandleImportEvent(events::ImportRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::ImportResponceEvent::value_type res(ev->value());
  NotifyProgress(sender, 25);
  core::FastoObjectCommandIPtr cmd = CreateCommandFast(REDIS_BACKUP_COMMAND, core::C_INNER);
  common::Error err = Execute(cmd);
  if (err) {
    res.setErrorInfo(err);
  } else {
    common::ErrnoError err = common::file_system::copy_file(BACKUP_DEFAULT_PATH, res.path);
    if (err) {
      res.setErrorInfo(common::make_error_from_errno(err));
    }
  }
  NotifyProgress(sender, 75);
  Reply(sender, new events::ImportResponceEvent(this, res));
  NotifyProgress(sender, 100);
}

void Driver::HandleExportEvent(events::ExportRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::ExportResponceEvent::value_type res(ev->value());
  NotifyProgress(sender, 25);
  common::ErrnoError err = common::file_system::copy_file(res.path, EXPORT_DEFAULT_PATH);
  if (err) {
    res.setErrorInfo(common::make_error_from_errno(err));
  }
  NotifyProgress(sender, 75);
  Reply(sender, new events::ExportResponceEvent(this, res));
  NotifyProgress(sender, 100);
}

void Driver::HandleLoadDatabaseInfosEvent(events::LoadDatabasesInfoRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::LoadDatabasesInfoResponceEvent::value_type res(ev->value());
  core::FastoObjectCommandIPtr cmd = CreateCommandFast(REDIS_GET_DATABASES_COMMAND, core::C_INNER);
  NotifyProgress(sender, 50);

  core::IDataBaseInfo* info = nullptr;
  common::Error err = CurrentDataBaseInfo(&info);
  if (err) {
    res.setErrorInfo(err);
    NotifyProgress(sender, 75);
    Reply(sender, new events::LoadDatabasesInfoResponceEvent(this, res));
    NotifyProgress(sender, 100);
    return;
  }

  err = Execute(cmd.get());
  if (err) {
    res.setErrorInfo(err);
    NotifyProgress(sender, 75);
    Reply(sender, new events::LoadDatabasesInfoResponceEvent(this, res));
    NotifyProgress(sender, 100);
    return;
  }

  core::FastoObject::childs_t rchildrens = cmd->GetChildrens();
  CHECK_EQ(rchildrens.size(), 1);
  core::FastoObjectArray* array = dynamic_cast<core::FastoObjectArray*>(rchildrens[0].get());  // +
  CHECK(array);
  common::ArrayValue* ar = array->GetArray();
  CHECK(ar);

  core::IDataBaseInfoSPtr curdb(info);
  std::string scountDb;
  if (ar->GetString(1, &scountDb)) {
    size_t countDb;
    if (common::ConvertFromString(scountDb, &countDb) && countDb > 0) {
      for (size_t i = 0; i < countDb; ++i) {
        core::IDataBaseInfoSPtr dbInf(new core::redis::DataBaseInfo(common::ConvertToString(i), false, 0));
        if (dbInf->GetName() == curdb->GetName()) {
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
  const core::command_buffer_t pattern_result =
      core::internal::GetKeysPattern(res.cursor_in, res.pattern, res.count_keys);
  core::FastoObjectCommandIPtr cmd = CreateCommandFast(pattern_result, core::C_INNER);
  NotifyProgress(sender, 50);
  common::Error err = Execute(cmd);
  if (err) {
    res.setErrorInfo(err);
  } else {
    core::FastoObject::childs_t rchildrens = cmd->GetChildrens();
    if (rchildrens.size()) {
      CHECK_EQ(rchildrens.size(), 1);
      core::FastoObjectArray* array = dynamic_cast<core::FastoObjectArray*>(rchildrens[0].get());  // +
      if (!array) {
        goto done;
      }

      common::ArrayValue* arm = array->GetArray();
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

      rchildrens = array->GetChildrens();
      if (!rchildrens.size()) {
        goto done;
      }

      core::FastoObject* obj = rchildrens[0].get();
      core::FastoObjectArray* arr = dynamic_cast<core::FastoObjectArray*>(obj);  // +
      if (!arr) {
        goto done;
      }

      common::ArrayValue* ar = arr->GetArray();
      if (ar->IsEmpty()) {
        goto done;
      }

      std::vector<core::FastoObjectCommandIPtr> cmds;
      cmds.reserve(ar->GetSize() * 2);
      for (size_t i = 0; i < ar->GetSize(); ++i) {
        std::string key;
        bool isok = ar->GetString(i, &key);
        if (isok) {
          core::key_t key_str(key);
          core::NKey k(key_str);
          core::NDbKValue dbv(k, core::NValue());
          core::command_buffer_writer_t wr_type;
          wr_type << REDIS_TYPE_COMMAND << " " << key_str.GetKeyForCommandLine();
          cmds.push_back(CreateCommandFast(wr_type.str(), core::C_INNER));

          core::command_buffer_writer_t wr_ttl;
          wr_ttl << DB_GET_TTL_COMMAND " " << key_str.GetKeyForCommandLine();
          cmds.push_back(CreateCommandFast(wr_ttl.str(), core::C_INNER));
          res.keys.push_back(dbv);
        }
      }

      err = impl_->ExecuteAsPipeline(cmds, &LOG_COMMAND);
      if (err) {
        goto done;
      }

      for (size_t i = 0; i < res.keys.size(); ++i) {
        core::FastoObjectIPtr cmdType = cmds[i * 2];
        core::FastoObject::childs_t tchildrens = cmdType->GetChildrens();
        if (tchildrens.size()) {
          DCHECK_EQ(tchildrens.size(), 1);
          if (tchildrens.size() == 1) {
            std::string typeRedis = tchildrens[0]->ToString();
            common::Value::Type ctype = convertFromStringRType(typeRedis);
            common::ValueSPtr empty_val(common::Value::CreateEmptyValueFromType(ctype));
            res.keys[i].SetValue(empty_val);
          }
        }

        core::FastoObjectIPtr cmdType2 = cmds[i * 2 + 1];
        tchildrens = cmdType2->GetChildrens();
        if (tchildrens.size()) {
          DCHECK_EQ(tchildrens.size(), 1);
          if (tchildrens.size() == 1) {
            auto vttl = tchildrens[0]->GetValue();
            core::ttl_t ttl = 0;
            if (vttl->GetAsLongLongInteger(&ttl)) {
              core::NKey key = res.keys[i].GetKey();
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
  core::FastoObjectCommandIPtr cmd = CreateCommandFast(REDIS_GET_PROPERTY_SERVER_COMMAND, core::C_INNER);
  NotifyProgress(sender, 50);
  common::Error err = Execute(cmd);
  if (err) {
    res.setErrorInfo(err);
  } else {
    core::FastoObject::childs_t ch = cmd->GetChildrens();
    if (ch.size()) {
      CHECK_EQ(ch.size(), 1);
      core::FastoObjectArray* array = dynamic_cast<core::FastoObjectArray*>(ch[0].get());  // +
      if (array) {
        res.info = core::MakeServerProperty(array->GetArray());
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
  core::command_buffer_writer_t wr;
  wr << "CONFIG SET " << res.new_item.first << " " << res.new_item.second;
  core::command_buffer_t change_request = wr.str();
  core::FastoObjectCommandIPtr cmd = CreateCommandFast(change_request, core::C_INNER);
  common::Error err = Execute(cmd);
  if (err) {
    res.setErrorInfo(err);
  } else {
    res.is_change = true;
  }
  NotifyProgress(sender, 75);
  Reply(sender, new events::ChangeServerPropertyInfoResponceEvent(this, res));
  NotifyProgress(sender, 100);
}

void Driver::HandleLoadServerChannelsRequestEvent(events::LoadServerChannelsRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::LoadServerChannelsResponceEvent::value_type res(ev->value());

  NotifyProgress(sender, 50);
  core::command_buffer_writer_t wr;
  wr << REDIS_PUBSUB_CHANNELS_COMMAND << " " << res.pattern;
  const core::command_buffer_t load_channels_request = wr.str();
  core::FastoObjectCommandIPtr cmd = CreateCommandFast(load_channels_request, core::C_INNER);
  common::Error err = Execute(cmd);
  if (err) {
    res.setErrorInfo(err);
    goto done;
  } else {
    core::FastoObject::childs_t rchildrens = cmd->GetChildrens();
    if (rchildrens.size()) {
      CHECK_EQ(rchildrens.size(), 1);
      core::FastoObjectArray* array = dynamic_cast<core::FastoObjectArray*>(rchildrens[0].get());  // +
      if (!array) {
        goto done;
      }

      common::ArrayValue* arm = array->GetArray();
      if (!arm->GetSize()) {
        goto done;
      }

      std::vector<core::FastoObjectCommandIPtr> cmds;
      cmds.reserve(arm->GetSize());
      for (size_t i = 0; i < arm->GetSize(); ++i) {
        std::string channel;
        bool isok = arm->GetString(i, &channel);
        if (isok) {
          core::command_buffer_writer_t wr2;
          wr2 << REDIS_PUBSUB_NUMSUB_COMMAND << " " << channel;
          core::NDbPSChannel c(channel, 0);
          cmds.push_back(CreateCommandFast(wr2.str(), core::C_INNER));
          res.channels.push_back(c);
        }
      }

      err = impl_->ExecuteAsPipeline(cmds, &LOG_COMMAND);
      if (err) {
        res.setErrorInfo(err);
        goto done;
      }

      for (size_t i = 0; i < res.channels.size(); ++i) {
        core::FastoObjectIPtr subCount = cmds[i];
        core::FastoObject::childs_t tchildrens = subCount->GetChildrens();
        if (tchildrens.size()) {
          DCHECK_EQ(tchildrens.size(), 1);
          if (tchildrens.size() == 1) {
            core::FastoObjectArray* array_sub = dynamic_cast<core::FastoObjectArray*>(tchildrens[0].get());  // +
            if (array_sub) {
              common::ArrayValue* array_sub_inner = array_sub->GetArray();
              common::Value* fund_sub = nullptr;
              if (array_sub_inner->Get(1, &fund_sub)) {
                std::string lsub_str;
                uint32_t lsub;
                if (fund_sub->GetAsString(&lsub_str) && common::ConvertFromString(lsub_str, &lsub)) {
                  res.channels[i].SetNumberOfSubscribers(lsub);
                }
              }
            }
          }
        }
      }
    }
  }

done:
  NotifyProgress(sender, 75);
  Reply(sender, new events::LoadServerChannelsResponceEvent(this, res));
  NotifyProgress(sender, 100);
}

core::IServerInfoSPtr Driver::MakeServerInfoFromString(const std::string& val) {
  core::IServerInfoSPtr res(core::redis::MakeRedisServerInfo(val));
  return res;
}

}  // namespace redis
}  // namespace proxy
}  // namespace fastonosql
