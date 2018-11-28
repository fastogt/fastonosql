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

#include "proxy/db/redis/driver.h"

#include <algorithm>
#include <string>
#include <vector>

#include <common/convert2string.h>           // for ConvertFromString, etc
#include <common/file_system/file_system.h>  // for copy_file

#include <fastonosql/core/db/redis/db_connection.h>  // for DBConnection, INFO_REQUEST, etc
#include <fastonosql/core/db/redis/server_info.h>
#include <fastonosql/core/db/redis_compatible/database_info.h>

#include <fastonosql/core/value.h>

#include "proxy/command/command.h"  // for CreateCommand, etc
#include "proxy/command/command_logger.h"
#include "proxy/db/redis/command.h"              // for Command
#include "proxy/db/redis/connection_settings.h"  // for ConnectionSettings

#define REDIS_TYPE_COMMAND "TYPE"
#define REDIS_SHUTDOWN_COMMAND "SHUTDOWN"
#define REDIS_BACKUP_COMMAND "SAVE"
#define REDIS_SET_PASSWORD_COMMAND "CONFIG SET requirepass"
#define REDIS_SET_MAX_CONNECTIONS_COMMAND "CONFIG SET maxclients"
#define REDIS_GET_PROPERTY_SERVER_COMMAND "CONFIG GET *"
#define REDIS_PUBSUB_CHANNELS_COMMAND "PUBSUB CHANNELS"
#define REDIS_PUBSUB_NUMSUB_COMMAND "PUBSUB NUMSUB"
#define REDIS_GET_COMMANDS "COMMAND"

#if defined(PRO_VERSION)
#include <fastonosql/core/imodule_connection_client.h>
#define REDIS_GET_LOADED_MODULES_COMMANDS "MODULE LIST"
#endif

#define REDIS_SET_DEFAULT_DATABASE_COMMAND_1ARGS_S "SELECT %s"

#define BACKUP_DEFAULT_PATH "/var/lib/redis/dump.rdb"
#define EXPORT_DEFAULT_PATH "/var/lib/redis/dump.rdb"

namespace fastonosql {
namespace core {
namespace {

command_buffer_t GetKeysOldPattern(const pattern_t& pattern) {
  command_buffer_writer_t wr;
  wr << "KEYS " << pattern;
  return wr.str();
}
}  // namespace
}  // namespace core
namespace proxy {
namespace redis {
#if defined(PRO_VERSION)
namespace {
const struct RedisRegisterTypes {
  RedisRegisterTypes() { qRegisterMetaType<core::ModuleInfo>("core::ModuleInfo"); }
} reg_type;

class ProxyModuleClient : public core::IModuleConnectionClient {
  Driver* drv_;

 public:
  explicit ProxyModuleClient(Driver* drv) : drv_(drv) {}
  void OnUnLoadedModule(const core::ModuleInfo& module) override { emit drv_->ModuleUnLoaded(module); }

  void OnLoadedModule(const core::ModuleInfo& module) override { emit drv_->ModuleLoaded(module); }
};

}  // namespace
#endif
Driver::Driver(IConnectionSettingsBaseSPtr settings)
    : IDriverRemote(settings),
#if defined(PRO_VERSION)
      proxy_(nullptr),
#endif
      impl_(nullptr) {
#if defined(PRO_VERSION)
  proxy_ = new ProxyModuleClient(this);
  impl_ = new core::redis::DBConnection(this, proxy_);
#else
  impl_ = new core::redis::DBConnection(this);
#endif
  COMPILE_ASSERT(core::redis::DBConnection::GetConnectionType() == core::REDIS,
                 "DBConnection must be the same type as Driver!");
  CHECK(GetType() == core::REDIS);
}

Driver::~Driver() {
  delete impl_;
#if defined(PRO_VERSION)
  delete proxy_;
#endif
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
                                                   core::CmdLoggingType logging_type) {
  return proxy::CreateCommand<Command>(parent, input, logging_type);
}

core::FastoObjectCommandIPtr Driver::CreateCommandFast(const core::command_buffer_t& input,
                                                       core::CmdLoggingType logging_type) {
  return proxy::CreateCommandFast<Command>(input, logging_type);
}

core::IDataBaseInfoSPtr Driver::CreateDatabaseInfo(const core::db_name_t& name, bool is_default, size_t size) {
  return std::make_shared<core::redis_compatible::DataBaseInfo>(name, is_default, size);
}

common::Error Driver::SyncConnect() {
  auto redis_settings = GetSpecificSettings<ConnectionSettings>();
  core::redis::RConfig rconf(redis_settings->GetInfo(), redis_settings->GetSSHInfo());
  common::Error err = impl_->Connect(rconf);
  if (err) {
    return err;
  }

  err = impl_->SetClientName(PROJECT_NAME_LOWERCASE);
  return common::Error();
}

common::Error Driver::SyncDisconnect() {
  return impl_->Disconnect();
}

common::Error Driver::ExecuteImpl(const core::command_buffer_t& command, core::FastoObject* out) {
  return impl_->Execute(command, out);
}

common::Error Driver::DBkcountImpl(core::keys_limit_t* size) {
  return impl_->DBKeysCount(size);
}

common::Error Driver::GetCurrentServerInfo(core::IServerInfo** info) {
  core::FastoObjectCommandIPtr cmd = CreateCommandFast(GEN_CMD_STRING(DB_INFO_COMMAND), core::C_INNER);
  common::Error err = Execute(cmd.get());
  if (err) {
    return err;
  }

  const auto content = common::ConvertToString(cmd.get());
  core::IServerInfo* linfo = core::redis::MakeRedisServerInfo(common::ConvertToString(content));  // #FIXME

  if (!linfo) {
    return common::make_error("Invalid " DB_INFO_COMMAND " command output");
  }

  *info = linfo;
  return common::Error();
}

common::Error Driver::GetServerCommands(std::vector<const core::CommandInfo*>* commands) {
  core::FastoObjectCommandIPtr cmd = CreateCommandFast(GEN_CMD_STRING(REDIS_GET_COMMANDS), core::C_INNER);
  common::Error err = Execute(cmd.get());
  if (err) {
    return err;
  }

  core::translator_t tran = impl_->GetTranslator();

  core::FastoObject::childs_t rchildrens = cmd->GetChildrens();
  CHECK_EQ(rchildrens.size(), 1);
  core::FastoObject* array = rchildrens[0].get();
  CHECK(array);
  std::vector<const core::CommandInfo*> lcommands;
  auto array_value = array->GetValue();
  common::ArrayValue* commands_array = nullptr;
  if (array_value->GetAsList(&commands_array)) {
    for (size_t i = 0; i < commands_array->GetSize(); ++i) {
      const common::ArrayValue* com_value = nullptr;
      if (commands_array->GetList(i, &com_value)) {
        // 0) command name
        // 1) command arity specification
        // 2) nested Array reply of command flags
        // 3) position of first key in argument list
        // 4) position of last key in argument list
        // 5) step count for locating repeating keys
        size_t sz = com_value->GetSize();
        if (sz < 4) {
          return common::make_error("Invalid " REDIS_GET_COMMANDS " command output");
        }

        core::command_buffer_t command_name;
        if (!com_value->GetString(0, &command_name)) {
          return common::make_error("Invalid " REDIS_GET_COMMANDS " command output");
        }

        const core::CommandHolder* cmd = nullptr;
        common::Error err = tran->FindCommand(command_name, &cmd);
        if (err) {
#if defined(PRO_VERSION)
          if (!impl_->IsInternalCommand(command_name)) {
            WARNING_LOG() << "Found not handled command: " << command_name;
          }
#endif
          continue;
        }

        lcommands.push_back(cmd);
      }
    }
  }

  *commands = lcommands;
  return common::Error();
}

#if defined(PRO_VERSION)
common::Error Driver::GetServerLoadedModules(std::vector<core::ModuleInfo>* modules) {
  core::FastoObjectCommandIPtr cmd =
      CreateCommandFast(GEN_CMD_STRING(REDIS_GET_LOADED_MODULES_COMMANDS), core::C_INNER);
  common::Error err = Execute(cmd.get());
  if (err) {
    return err;
  }

  core::FastoObject::childs_t rchildrens = cmd->GetChildrens();
  CHECK_EQ(rchildrens.size(), 1);
  core::FastoObject* array = rchildrens[0].get();
  CHECK(array);
  common::ArrayValue* modules_array = nullptr;
  auto array_value = array->GetValue();
  std::vector<core::ModuleInfo> lmodules;
  if (array_value->GetAsList(&modules_array)) {
    for (size_t i = 0; i < modules_array->GetSize(); ++i) {
      const common::ArrayValue* mod_value = nullptr;
      if (modules_array->GetList(i, &mod_value)) {
        if (mod_value->GetSize() < 4) {
          return common::make_error("Invalid " REDIS_GET_LOADED_MODULES_COMMANDS " command output");
        }

        core::command_buffer_t module_name;
        if (!mod_value->GetString(1, &module_name)) {
          return common::make_error("Invalid " REDIS_GET_LOADED_MODULES_COMMANDS " command output");
        }

        long long version;
        if (!mod_value->GetLongLongInteger(3, &version)) {
          return common::make_error("Invalid " REDIS_GET_LOADED_MODULES_COMMANDS " command output");
        }

        core::ModuleInfo mod;
        mod.name = common::ConvertToString(module_name);  // #FIXME
        mod.version = static_cast<uint32_t>(version);
        lmodules.push_back(mod);
      }
    }
  }

  *modules = lmodules;
  return common::Error();
}
#endif

common::Error Driver::GetCurrentDataBaseInfo(core::IDataBaseInfo** info) {
  if (!info) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  return impl_->Select(impl_->GetCurrentDBName(), info);
}

void Driver::HandleBackupEvent(events::BackupRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::BackupResponceEvent::value_type res(ev->value());
  NotifyProgress(sender, 25);
  core::FastoObjectCommandIPtr cmd = CreateCommandFast(GEN_CMD_STRING(REDIS_BACKUP_COMMAND), core::C_INNER);
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
  Reply(sender, new events::BackupResponceEvent(this, res));
  NotifyProgress(sender, 100);
}

void Driver::HandleRestoreEvent(events::RestoreRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::RestoreResponceEvent::value_type res(ev->value());
  NotifyProgress(sender, 25);
  common::ErrnoError err = common::file_system::copy_file(res.path, EXPORT_DEFAULT_PATH);
  if (err) {
    res.setErrorInfo(common::make_error_from_errno(err));
  }
  NotifyProgress(sender, 75);
  Reply(sender, new events::RestoreResponceEvent(this, res));
  NotifyProgress(sender, 100);
}

void Driver::HandleLoadDatabaseContentEvent(events::LoadDatabaseContentRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::LoadDatabaseContentResponceEvent::value_type res(ev->value());
  core::command_buffer_t pattern_result;
  bool new_behavior = true;
  auto serv = GetCurrentServerInfoIfConnected();
  if (!serv) {
    NotifyProgress(sender, 50);
    res.setErrorInfo(common::make_error("Not connected"));
    NotifyProgress(sender, 75);
    Reply(sender, new events::LoadDatabaseContentResponceEvent(this, res));
    NotifyProgress(sender, 100);
    return;
  }

  uint32_t version = serv->GetVersion();
  new_behavior = version >= PROJECT_VERSION_GENERATE(2, 8, 0);
  core::keys_limit_t keys_count = res.keys_count;
  if (new_behavior) {
    pattern_result = core::GetKeysPattern(res.cursor_in, res.pattern, keys_count);
  } else {
    pattern_result = core::GetKeysOldPattern(res.pattern);
  }
  core::FastoObjectCommandIPtr cmd = CreateCommandFast(pattern_result, core::C_INNER);
  NotifyProgress(sender, 50);
  common::Error err = Execute(cmd);
  if (err) {
    res.setErrorInfo(err);
  } else {
    core::FastoObject::childs_t rchildrens = cmd->GetChildrens();
    if (rchildrens.size()) {
      CHECK_EQ(rchildrens.size(), 1);

      core::FastoObject* array = rchildrens[0].get();
      CHECK(array);
      auto array_value = array->GetValue();
      common::ArrayValue* arm = nullptr;
      if (!array_value->GetAsList(&arm)) {
        goto done;
      }

      std::vector<core::FastoObjectCommandIPtr> cmds;
      if (new_behavior) {
        CHECK_EQ(arm->GetSize(), 2);
        core::cursor_t cursor;
        bool isok = arm->GetUInteger(0, &cursor);
        if (!isok) {
          goto done;
        }
        res.cursor_out = cursor;

        common::ArrayValue* ar = nullptr;
        isok = arm->GetList(1, &ar);
        if (!isok) {
          goto done;
        }

        cmds.reserve(ar->GetSize() * 2);
        for (size_t i = 0; i < ar->GetSize(); ++i) {
          common::Value::string_t key;
          bool isok = ar->GetString(i, &key);
          if (isok) {
            const core::nkey_t key_str(key);
            const core::NKey k(key_str);
            const core::NDbKValue dbv(k, core::NValue());
            core::command_buffer_writer_t wr_type;
            wr_type << REDIS_TYPE_COMMAND " " << key_str.GetForCommandLine();
            cmds.push_back(CreateCommandFast(wr_type.str(), core::C_INNER));

            core::command_buffer_writer_t wr_ttl;
            wr_ttl << DB_GET_TTL_COMMAND " " << key_str.GetForCommandLine();
            cmds.push_back(CreateCommandFast(wr_ttl.str(), core::C_INNER));
            res.keys.push_back(dbv);
          }
        }
      } else {
        keys_count = std::min<core::keys_limit_t>(keys_count, arm->GetSize());
        cmds.reserve(keys_count * 2);
        for (size_t i = 0; i < keys_count; ++i) {
          common::Value::string_t key;
          bool isok = arm->GetString(i, &key);
          if (isok) {
            const core::nkey_t key_str(key);
            const core::NKey k(key_str);
            const core::NDbKValue dbv(k, core::NValue());
            core::command_buffer_writer_t wr_type;
            wr_type << REDIS_TYPE_COMMAND " " << key_str.GetForCommandLine();
            cmds.push_back(CreateCommandFast(wr_type.str(), core::C_INNER));

            core::command_buffer_writer_t wr_ttl;
            wr_ttl << DB_GET_TTL_COMMAND " " << key_str.GetForCommandLine();
            cmds.push_back(CreateCommandFast(wr_ttl.str(), core::C_INNER));
            res.keys.push_back(dbv);
          }
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
            common::Value::string_t type_redis_str = tchildrens[0]->ToString();
            common::Value::Type ctype;
            core::redis_compatible::ConvertFromString(type_redis_str, &ctype);
            core::NValue empty_val(core::CreateEmptyValueFromType(ctype));
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

      err = DBkcountImpl(&res.db_keys_count);
      DCHECK(!err);
    }
  }
done:
  NotifyProgress(sender, 75);
  Reply(sender, new events::LoadDatabaseContentResponceEvent(this, res));
  NotifyProgress(sender, 100);
}

void Driver::HandleDiscoveryInfoEvent(events::DiscoveryInfoRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::DiscoveryInfoResponceEvent::value_type res(ev->value());
  NotifyProgress(sender, 50);

  if (IsConnected()) {
    core::IDataBaseInfo* db = nullptr;
    std::vector<const core::CommandInfo*> cmds;
    common::Error err = GetServerDiscoveryInfo(&db, &cmds);
    if (err) {
      res.setErrorInfo(err);
    } else {
      DCHECK(db);

      core::IDataBaseInfoSPtr current_database_info(db);

      res.dbinfo = current_database_info;
      res.commands = cmds;
#if defined(PRO_VERSION)
      std::vector<core::ModuleInfo> lmodules;
      GetServerLoadedModules(&lmodules);  // can be failed
      res.loaded_modules = lmodules;
#endif
    }
  } else {
    res.setErrorInfo(common::make_error("Not connected to server, impossible to get discovery info!"));
  }

  NotifyProgress(sender, 75);
  Reply(sender, new events::DiscoveryInfoResponceEvent(this, res));
  NotifyProgress(sender, 100);
}

void Driver::HandleLoadServerPropertyEvent(events::ServerPropertyInfoRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::ServerPropertyInfoResponceEvent::value_type res(ev->value());
  core::FastoObjectCommandIPtr cmd =
      CreateCommandFast(GEN_CMD_STRING(REDIS_GET_PROPERTY_SERVER_COMMAND), core::C_INNER);
  NotifyProgress(sender, 50);
  common::Error err = Execute(cmd);
  if (err) {
    res.setErrorInfo(err);
  } else {
    core::FastoObject::childs_t ch = cmd->GetChildrens();
    if (ch.size()) {
      CHECK_EQ(ch.size(), 1);
      core::FastoObject* array = ch[0].get();
      auto array_value = array->GetValue();
      common::ArrayValue* arr = nullptr;
      if (array_value->GetAsList(&arr)) {
        res.info = core::MakeServerProperty(arr);
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
  wr << REDIS_PUBSUB_CHANNELS_COMMAND " " << res.pattern;
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
      core::FastoObject* array = rchildrens[0].get();
      CHECK(array);
      auto array_value = array->GetValue();
      common::ArrayValue* arm = nullptr;
      if (!array_value->GetAsList(&arm) || !arm->GetSize()) {
        goto done;
      }

      std::vector<core::FastoObjectCommandIPtr> cmds;
      cmds.reserve(arm->GetSize());
      for (size_t i = 0; i < arm->GetSize(); ++i) {
        common::Value::string_t channel;
        bool isok = arm->GetString(i, &channel);
        if (isok) {
          core::command_buffer_writer_t wr2;
          wr2 << REDIS_PUBSUB_NUMSUB_COMMAND " " << channel;
          core::NDbPSChannel ch(common::ConvertToString(channel), 0);  // #FIXME
          cmds.push_back(CreateCommandFast(wr2.str(), core::C_INNER));
          res.channels.push_back(ch);
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
            core::FastoObject* array_sub = tchildrens[0].get();
            auto arr_value = array_sub->GetValue();
            common::ArrayValue* array_sub_inner = nullptr;
            if (arr_value->GetAsList(&array_sub_inner)) {
              common::Value* fund_sub = nullptr;
              if (array_sub_inner->Get(1, &fund_sub)) {
                common::Value::Type t = fund_sub->GetType();
                if (t == common::Value::TYPE_LONG_LONG_INTEGER) {
                  long long lsub;
                  if (fund_sub->GetAsLongLongInteger(&lsub)) {
                    res.channels[i].SetNumberOfSubscribers(lsub);
                  }
                } else if (t == common::Value::TYPE_STRING) {
                  common::Value::string_t lsub_str;
                  size_t lsub;
                  if (fund_sub->GetAsString(&lsub_str) && common::ConvertFromBytes(lsub_str, &lsub)) {
                    res.channels[i].SetNumberOfSubscribers(lsub);
                  }
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
