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

#include "proxy/driver/idriver.h"

#ifdef OS_WIN
#include <winsock2.h>
#else
#include <signal.h>
#endif

#include <memory>  // for __shared_ptr
#include <vector>  // for vector
#include <string>  // for allocator, string, etc

#include <QApplication>
#include <QThread>

extern "C" {
#include "sds.h"
}

#include <common/convert2string.h>  // for ConvertToString, etc
#include <common/file_system.h>     // for File, ascii_string_path, etc
#include <common/intrusive_ptr.h>   // for intrusive_ptr
#include <common/log_levels.h>      // for LEVEL_LOG::L_WARNING
#include <common/qt/utils_qt.h>     // for Event<>::value_type
#include <common/sprintf.h>         // for MemSPrintf
#include <common/string_util.h>     // for Tokenize
#include <common/time.h>            // for current_mstime
#include <common/types.h>           // for buffer_t, time64_t, etc
#include <common/utils.h>           // for c_strornull, msleep

#include "proxy/command/command_logger.h"  // for LOG_COMMAND
#include "proxy/driver/first_child_update_root_locker.h"
#include "proxy/driver/root_locker.h"  // for RootLocker
#include "proxy/events/events_info.h"

namespace {
#ifdef OS_WIN
struct WinsockInit {
  WinsockInit() {
    WSADATA d;
    if (WSAStartup(MAKEWORD(2, 2), &d) != 0) {
      _exit(1);
    }
  }
  ~WinsockInit() { WSACleanup(); }
} winsock_init;
#else
struct SigIgnInit {
  SigIgnInit() { signal(SIGPIPE, SIG_IGN); }
} sig_init;
#endif

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

  common::time64_t ltime_out = common::ConvertFromBytes<common::time64_t>(stamp);
  *time_out = ltime_out;
  return ltime_out != 0;
}
}  // namespace

namespace fastonosql {
namespace proxy {
namespace {
struct RegisterTypes {
  RegisterTypes() {
    qRegisterMetaType<common::ValueSPtr>("common::ValueSPtr");
    qRegisterMetaType<fastonosql::FastoObjectIPtr>("FastoObjectIPtr");
    qRegisterMetaType<core::NKey>("core::NKey");
    qRegisterMetaType<core::NDbKValue>("core::NDbKValue");
    qRegisterMetaType<core::IDataBaseInfoSPtr>("core::IDataBaseInfoSPtr");
    qRegisterMetaType<core::ttl_t>("core::ttl_t");
    qRegisterMetaType<std::string>("std::string");
    qRegisterMetaType<core::ServerInfoSnapShoot>("core::ServerInfoSnapShoot");
  }
} reg_type;

void notifyProgressImpl(IDriver* sender, QObject* reciver, int value) {
  IDriver::Reply(reciver, new events::ProgressResponceEvent(
                              sender, events::ProgressResponceEvent::value_type(value)));
}

template <typename event_request_type, typename event_responce_type>
void replyNotImplementedYet(IDriver* sender, event_request_type* ev, const char* eventCommandText) {
  QObject* esender = ev->sender();
  notifyProgressImpl(sender, esender, 0);
  typename event_responce_type::value_type res(ev->value());

  std::string patternResult = common::MemSPrintf(
      "Sorry, but now " PROJECT_NAME_TITLE " not supported %s command.", eventCommandText);
  common::Error er = common::make_error_value(patternResult, common::ErrorValue::E_ERROR);
  res.setErrorInfo(er);
  event_responce_type* resp = new event_responce_type(sender, res);
  IDriver::Reply(esender, resp);
  notifyProgressImpl(sender, esender, 100);
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
  destroy(&log_file_);
}

common::Error IDriver::Execute(FastoObjectCommandIPtr cmd) {
  if (!cmd) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  common::CommandValue* icmd = cmd->Cmd();
  const std::string command = icmd->inputCommand();
  const char* ccommand = common::utils::c_strornull(command);
  if (!ccommand) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  LOG_COMMAND(cmd);
  int argc;
  sds* argv = sdssplitargslong(ccommand, &argc);
  if (!argv) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  const char** exec_argv = const_cast<const char**>(argv);
  common::Error err = ExecuteImpl(argc, exec_argv, cmd.get());
  sdsfreesplitres(argv, argc);
  return err;
}

void IDriver::Reply(QObject* reciver, QEvent* ev) {
  qApp->postEvent(reciver, ev);
}

core::connectionTypes IDriver::Type() const {
  return settings_->Type();
}

connection_path_t IDriver::ConnectionPath() const {
  return settings_->Path();
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
    uint32_t interval = settings_->LoggingMsTimeInterval();
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
  if (err && err->isError()) {
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
  } else if (type == static_cast<QEvent::Type>(events::ShutDownRequestEvent::EventType)) {
    events::ShutDownRequestEvent* ev = static_cast<events::ShutDownRequestEvent*>(event);
    HandleShutdownEvent(ev);  // ni
  } else if (type == static_cast<QEvent::Type>(events::ProcessConfigArgsRequestEvent::EventType)) {
    events::ProcessConfigArgsRequestEvent* ev =
        static_cast<events::ProcessConfigArgsRequestEvent*>(event);
    HandleProcessCommandLineArgsEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::DisconnectRequestEvent::EventType)) {
    events::DisconnectRequestEvent* ev = static_cast<events::DisconnectRequestEvent*>(event);
    HandleDisconnectEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::ExecuteRequestEvent::EventType)) {
    events::ExecuteRequestEvent* ev = static_cast<events::ExecuteRequestEvent*>(event);
    HandleExecuteEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::LoadDatabasesInfoRequestEvent::EventType)) {
    events::LoadDatabasesInfoRequestEvent* ev =
        static_cast<events::LoadDatabasesInfoRequestEvent*>(event);
    HandleLoadDatabaseInfosEvent(ev);  //
  } else if (type == static_cast<QEvent::Type>(events::ServerInfoRequestEvent::EventType)) {
    events::ServerInfoRequestEvent* ev = static_cast<events::ServerInfoRequestEvent*>(event);
    HandleLoadServerInfoEvent(ev);  //
  } else if (type == static_cast<QEvent::Type>(events::ServerInfoHistoryRequestEvent::EventType)) {
    events::ServerInfoHistoryRequestEvent* ev =
        static_cast<events::ServerInfoHistoryRequestEvent*>(event);
    HandleLoadServerInfoHistoryEvent(ev);  //
  } else if (type == static_cast<QEvent::Type>(events::ClearServerHistoryRequestEvent::EventType)) {
    events::ClearServerHistoryRequestEvent* ev =
        static_cast<events::ClearServerHistoryRequestEvent*>(event);
    HandleClearServerHistoryEvent(ev);  //
  } else if (type == static_cast<QEvent::Type>(events::ServerPropertyInfoRequestEvent::EventType)) {
    events::ServerPropertyInfoRequestEvent* ev =
        static_cast<events::ServerPropertyInfoRequestEvent*>(event);
    HandleLoadServerPropertyEvent(ev);  // ni
  } else if (type ==
             static_cast<QEvent::Type>(events::ChangeServerPropertyInfoRequestEvent::EventType)) {
    events::ChangeServerPropertyInfoRequestEvent* ev =
        static_cast<events::ChangeServerPropertyInfoRequestEvent*>(event);
    HandleServerPropertyChangeEvent(ev);  // ni
  } else if (type == static_cast<QEvent::Type>(events::BackupRequestEvent::EventType)) {
    events::BackupRequestEvent* ev = static_cast<events::BackupRequestEvent*>(event);
    HandleBackupEvent(ev);  // ni
  } else if (type == static_cast<QEvent::Type>(events::ExportRequestEvent::EventType)) {
    events::ExportRequestEvent* ev = static_cast<events::ExportRequestEvent*>(event);
    HandleExportEvent(ev);  // ni
  } else if (type == static_cast<QEvent::Type>(events::ChangePasswordRequestEvent::EventType)) {
    events::ChangePasswordRequestEvent* ev =
        static_cast<events::ChangePasswordRequestEvent*>(event);
    HandleChangePasswordEvent(ev);  // ni
  } else if (type ==
             static_cast<QEvent::Type>(events::ChangeMaxConnectionRequestEvent::EventType)) {
    events::ChangeMaxConnectionRequestEvent* ev =
        static_cast<events::ChangeMaxConnectionRequestEvent*>(event);
    HandleChangeMaxConnectionEvent(ev);  // ni
  } else if (type ==
             static_cast<QEvent::Type>(events::LoadDatabaseContentRequestEvent::EventType)) {
    events::LoadDatabaseContentRequestEvent* ev =
        static_cast<events::LoadDatabaseContentRequestEvent*>(event);
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
      std::string path = settings_->LoggingPath();
      std::string dir = common::file_system::get_dir_path(path);
      common::Error err = common::file_system::create_directory(dir, true);
      if (err && err->isError()) {
      }
      if (common::file_system::is_directory(dir) == common::SUCCESS) {
        common::file_system::ascii_string_path p(path);
        log_file_ = new common::file_system::File(p);
      }
    }

    if (log_file_ && !log_file_->isOpened()) {
      bool opened = log_file_->open("ab+");
      DCHECK(opened);
    }

    if (log_file_ && log_file_->isOpened()) {
      common::time64_t time = common::time::current_mstime();
      std::string stamp = createStamp(time);
      core::IServerInfo* info = nullptr;
      common::Error er = CurrentServerInfo(&info);
      if (er && er->isError()) {
        QObject::timerEvent(event);
        return;
      }

      struct core::ServerInfoSnapShoot shot(time, core::IServerInfoSPtr(info));
      emit ServerInfoSnapShoot(shot);

      log_file_->write(stamp);
      log_file_->write(info->ToString());
      log_file_->flush();
    }
  }
  QObject::timerEvent(event);
}

void IDriver::NotifyProgress(QObject* reciver, int value) {
  notifyProgressImpl(this, reciver, value);
}

void IDriver::HandleConnectEvent(events::ConnectRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::ConnectResponceEvent::value_type res(ev->value());
  NotifyProgress(sender, 25);
  common::Error er = SyncConnect();
  if (er && er->isError()) {
    res.setErrorInfo(er);
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

  common::Error er = SyncDisconnect();
  if (er && er->isError()) {
    res.setErrorInfo(er);
  }

  Reply(sender, new events::DisconnectResponceEvent(this, res));
  NotifyProgress(sender, 100);
}

void IDriver::HandleExecuteEvent(events::ExecuteRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::ExecuteResponceEvent::value_type res(ev->value());

  const std::string inputLine = res.text;
  std::vector<std::string> commands;
  common::Error err = core::ParseCommands(inputLine, &commands);
  if (err && err->isError()) {
    res.setErrorInfo(err);
    Reply(sender, new events::ExecuteResponceEvent(this, res));
    NotifyProgress(sender, 100);
    return;
  }

  const bool silence = res.silence;
  const size_t repeat = res.repeat;
  const bool history = res.history;
  const common::time64_t msec_repeat_interval = res.msec_repeat_interval;
  const common::Value::CommandLoggingType log_type = res.logtype;
  RootLocker* lock =
      history ? new RootLocker(this, sender, inputLine, silence)
              : new FirstChildUpdateRootLocker(this, sender, inputLine, silence, commands);
  FastoObjectIPtr obj = lock->Root();
  const double step = 99.0 / double(commands.size() * (repeat + 1));
  double cur_progress = 0.0;
  for (size_t r = 0; r < repeat + 1; ++r) {
    common::time64_t start_ts = common::time::current_mstime();
    for (size_t i = 0; i < commands.size(); ++i) {
      if (IsInterrupted()) {
        res.setErrorInfo(common::make_error_value(
            "Interrupted exec.", common::ErrorValue::E_INTERRUPTED, common::logging::L_WARNING));
        goto done;
      }

      cur_progress += step;
      NotifyProgress(sender, cur_progress);

      std::string command = commands[i];
      FastoObjectCommandIPtr cmd = silence ? CreateCommandFast(command, log_type)
                                           : CreateCommand(obj.get(), command, log_type);  //
      common::Error err = Execute(cmd);
      if (err && err->isError()) {
        res.setErrorInfo(err);
        goto done;
      }
    }

    common::time64_t finished_ts = common::time::current_mstime();
    common::time64_t diff = finished_ts - start_ts;
    if (msec_repeat_interval > diff) {
      common::time64_t sleep_time = msec_repeat_interval - diff;
      common::utils::msleep(sleep_time);
    }
  }

done:
  Reply(sender, new events::ExecuteResponceEvent(this, res));
  NotifyProgress(sender, 100);
  delete lock;
}

void IDriver::HandleLoadServerPropertyEvent(events::ServerPropertyInfoRequestEvent* ev) {
  replyNotImplementedYet<events::ServerPropertyInfoRequestEvent,
                         events::ServerPropertyInfoResponceEvent>(this, ev, "server property");
}

void IDriver::HandleServerPropertyChangeEvent(events::ChangeServerPropertyInfoRequestEvent* ev) {
  replyNotImplementedYet<events::ChangeServerPropertyInfoRequestEvent,
                         events::ChangeServerPropertyInfoResponceEvent>(this, ev,
                                                                        "change server property");
}

void IDriver::HandleShutdownEvent(events::ShutDownRequestEvent* ev) {
  replyNotImplementedYet<events::ShutDownRequestEvent, events::ShutDownResponceEvent>(this, ev,
                                                                                      "shutdown");
}

void IDriver::HandleBackupEvent(events::BackupRequestEvent* ev) {
  replyNotImplementedYet<events::BackupRequestEvent, events::BackupResponceEvent>(this, ev,
                                                                                  "backup server");
}

void IDriver::HandleExportEvent(events::ExportRequestEvent* ev) {
  replyNotImplementedYet<events::ExportRequestEvent, events::ExportResponceEvent>(this, ev,
                                                                                  "export server");
}

void IDriver::HandleChangePasswordEvent(events::ChangePasswordRequestEvent* ev) {
  replyNotImplementedYet<events::ChangePasswordRequestEvent, events::ChangePasswordResponceEvent>(
      this, ev, "change password");
}

void IDriver::HandleChangeMaxConnectionEvent(events::ChangeMaxConnectionRequestEvent* ev) {
  replyNotImplementedYet<events::ChangeMaxConnectionRequestEvent,
                         events::ChangeMaxConnectionResponceEvent>(this, ev,
                                                                   "change maximum connection");
}

void IDriver::HandleLoadDatabaseInfosEvent(events::LoadDatabasesInfoRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::LoadDatabasesInfoResponceEvent::value_type res(ev->value());
  NotifyProgress(sender, 50);
  core::IDataBaseInfo* info = nullptr;
  common::Error err = CurrentDataBaseInfo(&info);
  if (err && err->isError()) {
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
  common::Error err = CurrentServerInfo(&info);
  if (err && err->isError()) {
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

  std::string path = settings_->LoggingPath();
  common::file_system::ascii_string_path p(path);
  common::file_system::File readFile(p);
  if (readFile.open("rb")) {
    events::ServerInfoHistoryResponceEvent::value_type::infos_container_type tmpInfos;

    common::time64_t curStamp = 0;
    common::buffer_t dataInfo;

    while (!readFile.isEof()) {
      common::buffer_t data;
      bool res = readFile.readLine(&data);
      if (!res || readFile.isEof()) {
        if (curStamp) {
          struct core::ServerInfoSnapShoot shoot(
              curStamp, MakeServerInfoFromString(common::ConvertToString(dataInfo)));
          tmpInfos.push_back(shoot);
        }
        break;
      }

      common::time64_t tmpStamp = 0;
      bool isSt = getStamp(data, &tmpStamp);
      if (isSt) {
        if (curStamp) {
          struct core::ServerInfoSnapShoot shoot(
              curStamp, MakeServerInfoFromString(common::ConvertToString(dataInfo)));
          tmpInfos.push_back(shoot);
        }
        curStamp = tmpStamp;
        dataInfo.clear();
      } else {
        dataInfo.insert(dataInfo.end(), data.begin(), data.end());
      }
    }
    res.setInfos(tmpInfos);
  } else {
    res.setErrorInfo(
        common::make_error_value("History file not found", common::ErrorValue::E_ERROR));
  }

  Reply(sender, new events::ServerInfoHistoryResponceEvent(this, res));
}

void IDriver::HandleClearServerHistoryEvent(events::ClearServerHistoryRequestEvent* ev) {
  QObject* sender = ev->sender();
  events::ClearServerHistoryResponceEvent::value_type res(ev->value());

  bool ret = false;

  if (log_file_ && log_file_->isOpened()) {
    ret = log_file_->truncate(0);
  } else {
    std::string path = settings_->LoggingPath();
    if (common::file_system::is_file_exist(path)) {
      common::Error err = common::file_system::remove_file(path);
      if (err && err->isError()) {
        ret = false;
      } else {
        ret = true;
      }
    } else {
      ret = true;
    }
  }

  if (!ret) {
    res.setErrorInfo(common::make_error_value("Clear file error!", common::ErrorValue::E_ERROR));
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
    common::Error err = ServerDiscoveryInfo(&info, &db);
    if (err && err->isError()) {
      res.setErrorInfo(err);
    } else {
      DCHECK(info);
      DCHECK(db);

      core::IServerInfoSPtr server_info(info);
      core::IDataBaseInfoSPtr current_database_info(db);

      res.sinfo = server_info;
      res.dbinfo = current_database_info;
    }
  } else {
    res.setErrorInfo(common::make_error_value(
        "Not connected to server, impossible to get discovery info!", common::Value::E_ERROR));
  }

  NotifyProgress(sender, 75);
  Reply(sender, new events::DiscoveryInfoResponceEvent(this, res));
  NotifyProgress(sender, 100);
}

common::Error IDriver::ServerDiscoveryInfo(core::IServerInfo** sinfo, core::IDataBaseInfo** dbinfo) {
  core::IServerInfo* lsinfo = nullptr;
  common::Error er = CurrentServerInfo(&lsinfo);
  if (er && er->isError()) {
    return er;
  }

  core::IDataBaseInfo* ldbinfo = nullptr;
  er = CurrentDataBaseInfo(&ldbinfo);
  if (er && er->isError()) {
    delete lsinfo;
    return er;
  }

  *sinfo = lsinfo;
  *dbinfo = ldbinfo;
  return er;
}

void IDriver::OnFlushedCurrentDB() {
  emit FlushedDB();
}

void IDriver::OnCurrentDataBaseChanged(core::IDataBaseInfo* info) {
  core::IDataBaseInfoSPtr curdb(info->Clone());
  emit CurrentDataBaseChanged(curdb);
}

void IDriver::OnKeysRemoved(const core::NKeys& keys) {
  for (size_t i = 0; i < keys.size(); ++i) {
    emit KeyRemoved(keys[i]);
  }
}

void IDriver::OnKeyAdded(const core::NDbKValue& key) {
  emit KeyAdded(key);
}

void IDriver::OnKeyLoaded(const core::NDbKValue& key) {
  emit KeyLoaded(key);
}

void IDriver::OnKeyRenamed(const core::NKey& key, const std::string& new_key) {
  emit KeyRenamed(key, new_key);
}

void IDriver::OnKeyTTLChanged(const core::NKey& key, core::ttl_t ttl) {
  emit KeyTTLChanged(key, ttl);
}

void IDriver::OnKeyTTLLoaded(const core::NKey& key, core::ttl_t ttl) {
  emit KeyTTLLoaded(key, ttl);
}

void IDriver::OnQuited() {
  emit Disconnected();
}

}  // namespace proxy
}  // namespace fastonosql
