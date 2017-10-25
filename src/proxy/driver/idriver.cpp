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

#include "proxy/driver/idriver.h"

#include <QApplication>
#include <QThread>

#include <common/convert2string.h>  // for ConvertToString, etc
#include <common/file_system/file.h>
#include <common/file_system/file_system.h>
#include <common/file_system/string_path_utils.h>
#include <common/sprintf.h>
#include <common/threads/platform_thread.h>
#include <common/time.h>  // for current_mstime

#include "proxy/command/command_logger.h"  // for LOG_COMMAND
#include "proxy/driver/first_child_update_root_locker.h"

namespace {

const char magicNumber = 0x1E;
std::string createStamp(common::time64_t time) {
  return magicNumber + common::ConvertToString(time) + '\n';
}

bool getStamp(common::buffer_t stamp, common::time64_t* time_out) {
  if (stamp.empty()) {
    return false;
  }

  if (stamp[0] != magicNumber) {
    return false;
  }

  stamp.erase(stamp.begin());  // pop_front
  if (stamp.empty()) {
    return false;
  }

  if (stamp[stamp.size() - 1] == '\n') {
    stamp.pop_back();
  }

  common::time64_t ltime_out;
  if (!common::ConvertFromBytes(stamp, &ltime_out)) {
    return false;
  }
  *time_out = ltime_out;
  return ltime_out != 0;
}
}  // namespace

namespace fastonosql {
namespace proxy {
namespace {
const struct RegisterTypes {
  RegisterTypes() {
    qRegisterMetaType<common::ValueSPtr>("common::ValueSPtr");
    qRegisterMetaType<core::FastoObjectIPtr>("core::FastoObjectIPtr");
    qRegisterMetaType<core::NKey>("core::NKey");
    qRegisterMetaType<core::NDbKValue>("core::NDbKValue");
    qRegisterMetaType<core::IDataBaseInfoSPtr>("core::IDataBaseInfoSPtr");
    qRegisterMetaType<core::ttl_t>("core::ttl_t");
    qRegisterMetaType<core::command_buffer_t>("core::command_buffer_t");
    qRegisterMetaType<core::string_key_t>("core::string_key_t");
    qRegisterMetaType<core::ServerInfoSnapShoot>("core::ServerInfoSnapShoot");
  }
} reg_type;

void NotifyProgressImpl(IDriver* sender, QObject* reciver, int value) {
  IDriver::Reply(reciver, new events::ProgressResponceEvent(sender, events::ProgressResponceEvent::value_type(value)));
}

template <typename event_request_type, typename event_responce_type>
void ReplyNotImplementedYet(IDriver* sender, event_request_type* ev, const char* eventCommandText) {
  QObject* esender = ev->sender();
  NotifyProgressImpl(sender, esender, 0);
  typename event_responce_type::value_type res(ev->value());

  std::string patternResult =
      common::MemSPrintf("Sorry, but now " PROJECT_NAME_TITLE " not supported %s command.", eventCommandText);
  common::Error er = common::make_error(patternResult);
  res.setErrorInfo(er);
  event_responce_type* resp = new event_responce_type(sender, res);
  IDriver::Reply(esender, resp);
  NotifyProgressImpl(sender, esender, 100);
}

}  // namespace

IDriver::IDriver(IConnectionSettingsBaseSPtr settings)
    : settings_(settings), thread_(nullptr), timer_info_id_(0), log_file_(nullptr) {
  thread_ = new QThread(this);
  moveToThread(thread_);

  VERIFY(connect(thread_, &QThread::started, this, &IDriver::Init));
  VERIFY(connect(thread_, &QThread::finished, this, &IDriver::Clear));
}

IDriver::~IDriver() {
  if (log_file_) {
    log_file_->Close();
    destroy(&log_file_);
  }
}

common::Error IDriver::Execute(core::FastoObjectCommandIPtr cmd) {
  if (!cmd) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  LOG_COMMAND(cmd);
  common::Error err = ExecuteImpl(cmd->GetInputCommand(), cmd.get());
  return err;
}

void IDriver::Reply(QObject* reciver, QEvent* ev) {
  qApp->postEvent(reciver, ev);
}

core::connectionTypes IDriver::GetType() const {
  return settings_->GetType();
}

connection_path_t IDriver::ConnectionPath() const {
  return settings_->GetPath();
}

std::string IDriver::GetDelimiter() const {
  return settings_->GetDelimiter();
}

std::string IDriver::GetNsSeparator() const {
  return settings_->GetNsSeparator();
}

void IDriver::Start() {
  thread_->start();
}

void IDriver::Stop() {
  thread_->quit();
  thread_->wait();
}

void IDriver::Interrupt() {
  SetInterrupted(true);
}

void IDriver::Init() {
  if (settings_->IsHistoryEnabled()) {
    int interval = settings_->GetLoggingMsTimeInterval();
    timer_info_id_ = startTimer(interval);
    DCHECK(timer_info_id_ != 0);
  }
  InitImpl();
}

void IDriver::Clear() {
  if (timer_info_id_ != 0) {
    killTimer(timer_info_id_);
    timer_info_id_ = 0;
  }
  common::Error err = SyncDisconnect();
  if (err) {
    DNOTREACHED();
  }
  ClearImpl();
}

void IDriver::customEvent(QEvent* event) {
  SetInterrupted(false);

  QEvent::Type type = event->type();
  if (type == static_cast<QEvent::Type>(events::ConnectRequestEvent::EventType)) {
    events::ConnectRequestEvent* ev = static_cast<events::ConnectRequestEvent*>(event);
    HandleConnectEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::DisconnectRequestEvent::EventType)) {
    events::DisconnectRequestEvent* ev = static_cast<events::DisconnectRequestEvent*>(event);
    HandleDisconnectEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::ExecuteRequestEvent::EventType)) {
    events::ExecuteRequestEvent* ev = static_cast<events::ExecuteRequestEvent*>(event);
    HandleExecuteEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::LoadDatabasesInfoRequestEvent::EventType)) {
    events::LoadDatabasesInfoRequestEvent* ev = static_cast<events::LoadDatabasesInfoRequestEvent*>(event);
    HandleLoadDatabaseInfosEvent(ev);  //
  } else if (type == static_cast<QEvent::Type>(events::ServerInfoRequestEvent::EventType)) {
    events::ServerInfoRequestEvent* ev = static_cast<events::ServerInfoRequestEvent*>(event);
    HandleLoadServerInfoEvent(ev);  //
  } else if (type == static_cast<QEvent::Type>(events::ServerInfoHistoryRequestEvent::EventType)) {
    events::ServerInfoHistoryRequestEvent* ev = static_cast<events::ServerInfoHistoryRequestEvent*>(event);
    HandleLoadServerInfoHistoryEvent(ev);  //
  } else if (type == static_cast<QEvent::Type>(events::ClearServerHistoryRequestEvent::EventType)) {
    events::ClearServerHistoryRequestEvent* ev = static_cast<events::ClearServerHistoryRequestEvent*>(event);
    HandleClearServerHistoryEvent(ev);  //
  } else if (type == static_cast<QEvent::Type>(events::ServerPropertyInfoRequestEvent::EventType)) {
    events::ServerPropertyInfoRequestEvent* ev = static_cast<events::ServerPropertyInfoRequestEvent*>(event);
    HandleLoadServerPropertyEvent(ev);  // ni
  } else if (type == static_cast<QEvent::Type>(events::ChangeServerPropertyInfoRequestEvent::EventType)) {
    events::ChangeServerPropertyInfoRequestEvent* ev =
        static_cast<events::ChangeServerPropertyInfoRequestEvent*>(event);
    HandleServerPropertyChangeEvent(ev);  // ni
  } else if (type == static_cast<QEvent::Type>(events::LoadServerChannelsRequestEvent::EventType)) {
    events::LoadServerChannelsRequestEvent* ev = static_cast<events::LoadServerChannelsRequestEvent*>(event);
    HandleLoadServerChannelsRequestEvent(ev);  // ni
  } else if (type == static_cast<QEvent::Type>(events::BackupRequestEvent::EventType)) {
    events::BackupRequestEvent* ev = static_cast<events::BackupRequestEvent*>(event);
    HandleBackupEvent(ev);  // ni
  } else if (type == static_cast<QEvent::Type>(events::RestoreRequestEvent::EventType)) {
    events::RestoreRequestEvent* ev = static_cast<events::RestoreRequestEvent*>(event);
    HandleRestoreEvent(ev);  // ni
  } else if (type == static_cast<QEvent::Type>(events::LoadDatabaseContentRequestEvent::EventType)) {
    events::LoadDatabaseContentRequestEvent* ev = static_cast<events::LoadDatabaseContentRequestEvent*>(event);
    HandleLoadDatabaseContentEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::DiscoveryInfoRequestEvent::EventType)) {
    events::DiscoveryInfoRequestEvent* ev = static_cast<events::DiscoveryInfoRequestEvent*>(event);
    HandleDiscoveryInfoEvent(ev);  //
  }

  return QObject::customEvent(event);
}

void IDriver::timerEvent(QTimerEvent* event) {
  if (timer_info_id_ == event->timerId() && settings_->IsHistoryEnabled() && IsConnected()) {
    if (!log_file_) {
      std::string path = settings_->GetLoggingPath();
      std::string dir = common::file_system::get_dir_path(path);
      common::ErrnoError err = common::file_system::create_directory(dir, true);
      if (err) {
      }
      if (common::file_system::is_directory(dir) == common::SUCCESS) {
        common::file_system::ascii_string_path p(path);
        log_file_ = new common::file_system::ANSIFile(p);
      }
    }

    if (log_file_ && !log_file_->IsOpened()) {
      common::ErrnoError err = log_file_->Open("ab+");
      if (err) {
        DNOTREACHED();
      }
    }

    if (log_file_ && log_file_->IsOpened()) {
      common::time64_t time = common::time::current_mstime();
      std::string stamp = createStamp(time);
      core::IServerInfo* info = nullptr;
      common::Error err = GetCurrentServerInfo(&info);
      if (err) {
        QObject::timerEvent(event);
        return;
      }

      core::ServerInfoSnapShoot shot(time, core::IServerInfoSPtr(info));
      emit ServerInfoSnapShooted(shot);

      log_file_->Write(stamp);
      log_file_->Write(info->ToString());
      log_file_->Flush();
    }
  }
  QObject::timerEvent(event);
}

void IDriver::NotifyProgress(QObject* reciver, int value) {
  NotifyProgressImpl(this, reciver, value);
}

void IDriver::HandleConnectEvent(events::ConnectRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::ConnectResponceEvent::value_type res(ev->value());
  NotifyProgress(sender, 25);
  common::Error err = SyncConnect();
  if (err) {
    res.setErrorInfo(err);
  }
  NotifyProgress(sender, 75);
  Reply(sender, new events::ConnectResponceEvent(this, res));
  NotifyProgress(sender, 100);
}

void IDriver::HandleDisconnectEvent(events::DisconnectRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::DisconnectResponceEvent::value_type res(ev->value());
  NotifyProgress(sender, 50);

  common::Error err = SyncDisconnect();
  if (err) {
    res.setErrorInfo(err);
  }

  Reply(sender, new events::DisconnectResponceEvent(this, res));
  NotifyProgress(sender, 100);
}

void IDriver::HandleExecuteEvent(events::ExecuteRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::ExecuteResponceEvent::value_type res(ev->value());

  const core::command_buffer_t input_line = res.text;
  std::vector<core::command_buffer_t> commands;
  common::Error err = core::ParseCommands(input_line, &commands);
  if (err) {
    res.setErrorInfo(err);
    Reply(sender, new events::ExecuteResponceEvent(this, res));
    NotifyProgress(sender, 100);
    return;
  }

  const bool silence = res.silence;
  const size_t repeat = res.repeat;
  const bool history = res.history;
  const common::time64_t msec_repeat_interval = res.msec_repeat_interval;
  const core::CmdLoggingType log_type = res.logtype;
  RootLocker* lock = history ? new RootLocker(this, sender, input_line, silence)
                             : new FirstChildUpdateRootLocker(this, sender, input_line, silence, commands);
  core::FastoObjectIPtr obj = lock->Root();
  const double step = 99.0 / double(commands.size() * (repeat + 1));
  double cur_progress = 0.0;
  for (size_t r = 0; r < repeat + 1; ++r) {
    common::time64_t start_ts = common::time::current_mstime();
    for (size_t i = 0; i < commands.size(); ++i) {
      if (IsInterrupted()) {
        res.setErrorInfo(common::make_error(common::COMMON_EINTR));
        goto done;
      }

      cur_progress += step;
      NotifyProgress(sender, static_cast<int>(cur_progress));

      core::command_buffer_t command = commands[i];
      core::FastoObjectCommandIPtr cmd =
          silence ? CreateCommandFast(command, log_type) : CreateCommand(obj.get(), command, log_type);  //
      common::Error err = Execute(cmd);
      if (err) {
        res.setErrorInfo(err);
        goto done;
      }
    }

    common::time64_t finished_ts = common::time::current_mstime();
    common::time64_t diff = finished_ts - start_ts;
    if (msec_repeat_interval > diff) {
      common::time64_t sleep_time = msec_repeat_interval - diff;
      common::threads::PlatformThread::Sleep(sleep_time);
    }
  }

done:
  Reply(sender, new events::ExecuteResponceEvent(this, res));
  NotifyProgress(sender, 100);
  delete lock;
}

void IDriver::HandleLoadServerPropertyEvent(events::ServerPropertyInfoRequestEvent* ev) {
  ReplyNotImplementedYet<events::ServerPropertyInfoRequestEvent, events::ServerPropertyInfoResponceEvent>(
      this, ev, "server property");
}

void IDriver::HandleServerPropertyChangeEvent(events::ChangeServerPropertyInfoRequestEvent* ev) {
  ReplyNotImplementedYet<events::ChangeServerPropertyInfoRequestEvent, events::ChangeServerPropertyInfoResponceEvent>(
      this, ev, "change server property");
}

void IDriver::HandleLoadServerChannelsRequestEvent(events::LoadServerChannelsRequestEvent* ev) {
  ReplyNotImplementedYet<events::LoadServerChannelsRequestEvent, events::LoadServerChannelsResponceEvent>(
      this, ev, "load server channels");
}

void IDriver::HandleBackupEvent(events::BackupRequestEvent* ev) {
  ReplyNotImplementedYet<events::BackupRequestEvent, events::BackupResponceEvent>(this, ev, "backup server");
}

void IDriver::HandleRestoreEvent(events::RestoreRequestEvent* ev) {
  ReplyNotImplementedYet<events::RestoreRequestEvent, events::RestoreResponceEvent>(this, ev, "export server");
}

void IDriver::HandleLoadDatabaseInfosEvent(events::LoadDatabasesInfoRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::LoadDatabasesInfoResponceEvent::value_type res(ev->value());
  NotifyProgress(sender, 50);
  core::IDataBaseInfo* info = nullptr;
  common::Error err = GetCurrentDataBaseInfo(&info);
  if (err) {
    res.setErrorInfo(err);
  } else {
    res.databases.push_back(core::IDataBaseInfoSPtr(info));
  }
  Reply(sender, new events::LoadDatabasesInfoResponceEvent(this, res));
  NotifyProgress(sender, 100);
}

void IDriver::HandleLoadServerInfoEvent(events::ServerInfoRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::ServerInfoResponceEvent::value_type res(ev->value());
  NotifyProgress(sender, 50);
  core::IServerInfo* info = nullptr;
  common::Error err = GetCurrentServerInfo(&info);
  if (err) {
    res.setErrorInfo(err);
  } else {
    core::IServerInfoSPtr mem(info);
    res.setInfo(mem);
  }
  NotifyProgress(sender, 75);
  Reply(sender, new events::ServerInfoResponceEvent(this, res));
  NotifyProgress(sender, 100);
}

void IDriver::HandleLoadServerInfoHistoryEvent(events::ServerInfoHistoryRequestEvent* ev) {
  QObject* sender = ev->sender();
  events::ServerInfoHistoryResponceEvent::value_type res(ev->value());

  std::string path = settings_->GetLoggingPath();
  common::file_system::ascii_string_path p(path);
  common::file_system::ANSIFile readFile(p);
  if (readFile.Open("rb")) {
    events::ServerInfoHistoryResponceEvent::value_type::infos_container_type tmpInfos;

    common::time64_t curStamp = 0;
    common::buffer_t dataInfo;

    while (!readFile.IsEOF()) {
      common::buffer_t data;
      bool res = readFile.ReadLine(&data);
      if (!res || readFile.IsEOF()) {
        if (curStamp) {
          core::ServerInfoSnapShoot shoot(curStamp, MakeServerInfoFromString(common::ConvertToString(dataInfo)));
          tmpInfos.push_back(shoot);
        }
        break;
      }

      common::time64_t tmpStamp = 0;
      bool isSt = getStamp(data, &tmpStamp);
      if (isSt) {
        if (curStamp) {
          core::ServerInfoSnapShoot shoot(curStamp, MakeServerInfoFromString(common::ConvertToString(dataInfo)));
          tmpInfos.push_back(shoot);
        }
        curStamp = tmpStamp;
        dataInfo.clear();
      } else {
        dataInfo.insert(dataInfo.end(), data.begin(), data.end());
      }
    }
    res.setInfos(tmpInfos);
    readFile.Close();
  } else {
    res.setErrorInfo(common::make_error("History file not found"));
  }

  Reply(sender, new events::ServerInfoHistoryResponceEvent(this, res));
}

void IDriver::HandleClearServerHistoryEvent(events::ClearServerHistoryRequestEvent* ev) {
  QObject* sender = ev->sender();
  events::ClearServerHistoryResponceEvent::value_type res(ev->value());

  bool ret = false;

  if (log_file_ && log_file_->IsOpened()) {
    common::ErrnoError err = log_file_->Truncate(0);
    ret = err ? false : true;
  } else {
    std::string path = settings_->GetLoggingPath();
    if (common::file_system::is_file_exist(path)) {
      common::ErrnoError err = common::file_system::remove_file(path);
      if (err) {
        ret = false;
      } else {
        ret = true;
      }
    } else {
      ret = true;
    }
  }

  if (!ret) {
    res.setErrorInfo(common::make_error("Clear file error!"));
  }

  Reply(sender, new events::ClearServerHistoryResponceEvent(this, res));
}

void IDriver::HandleDiscoveryInfoEvent(events::DiscoveryInfoRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::DiscoveryInfoResponceEvent::value_type res(ev->value());
  NotifyProgress(sender, 50);

  if (IsConnected()) {
    core::IServerInfo* info = nullptr;
    core::IDataBaseInfo* db = nullptr;
    std::vector<const core::CommandInfo*> cmds;
    common::Error err = GetServerDiscoveryInfo(&info, &db, &cmds);
    if (err) {
      res.setErrorInfo(err);
    } else {
      DCHECK(info);
      DCHECK(db);

      core::IServerInfoSPtr server_info(info);
      core::IDataBaseInfoSPtr current_database_info(db);

      res.sinfo = server_info;
      res.dbinfo = current_database_info;
      res.commands = cmds;
    }
  } else {
    res.setErrorInfo(common::make_error("Not connected to server, impossible to get discovery info!"));
  }

  NotifyProgress(sender, 75);
  Reply(sender, new events::DiscoveryInfoResponceEvent(this, res));
  NotifyProgress(sender, 100);
}

common::Error IDriver::GetServerDiscoveryInfo(core::IServerInfo** sinfo,
                                              core::IDataBaseInfo** dbinfo,
                                              std::vector<const core::CommandInfo*>* commands) {
  core::IServerInfo* lsinfo = nullptr;
  common::Error err = GetCurrentServerInfo(&lsinfo);
  if (err) {
    return err;
  }

  std::vector<const core::CommandInfo*> lcommands;
  err = GetServerCommands(&lcommands);
  if (err) {
    return err;
  }

  core::IDataBaseInfo* ldbinfo = nullptr;
  err = GetCurrentDataBaseInfo(&ldbinfo);
  if (err) {
    delete lsinfo;
    return err;
  }

  *sinfo = lsinfo;
  *commands = lcommands;
  *dbinfo = ldbinfo;
  return err;
}

void IDriver::OnFlushedCurrentDB() {
  emit DBFlushed();
}

void IDriver::OnCreatedDB(core::IDataBaseInfo* info) {
  core::IDataBaseInfoSPtr curdb(info->Clone());
  emit DBCreated(curdb);
}

void IDriver::OnRemovedDB(core::IDataBaseInfo* info) {
  core::IDataBaseInfoSPtr curdb(info->Clone());
  emit DBRemoved(curdb);
}

void IDriver::OnChangedCurrentDB(core::IDataBaseInfo* info) {
  core::IDataBaseInfoSPtr curdb(info->Clone());
  emit DBChanged(curdb);
}

void IDriver::OnRemovedKeys(const core::NKeys& keys) {
  for (size_t i = 0; i < keys.size(); ++i) {
    emit KeyRemoved(keys[i]);
  }
}

void IDriver::OnAddedKey(const core::NDbKValue& key) {
  emit KeyAdded(key);
}

void IDriver::OnLoadedKey(const core::NDbKValue& key) {
  emit KeyLoaded(key);
}

void IDriver::OnRenamedKey(const core::NKey& key, const core::string_key_t& new_key) {
  emit KeyRenamed(key, new_key);
}

void IDriver::OnChangedKeyTTL(const core::NKey& key, core::ttl_t ttl) {
  emit KeyTTLChanged(key, ttl);
}

void IDriver::OnLoadedKeyTTL(const core::NKey& key, core::ttl_t ttl) {
  emit KeyTTLLoaded(key, ttl);
}

void IDriver::OnQuited() {
  emit Disconnected();
}

}  // namespace proxy
}  // namespace fastonosql
