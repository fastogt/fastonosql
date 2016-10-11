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

#include "core/driver/idriver.h"

#ifdef OS_WIN
#include <winsock2.h>
#else
#include <signal.h>
#endif

#include <stddef.h>  // for NULL

#include <string>  // for allocator, string, etc

#include <QApplication>
#include <QThread>

extern "C" {
#include "sds.h"
}

#include <common/convert2string.h>  // for ConvertToString, etc
#include <common/file_system.h>     // for File, ascii_string_path, etc
#include <common/time.h>            // for current_mstime
#include <common/utils.h>           // for c_strornull

#include "core/command/command_logger.h"  // for LOG_COMMAND

#include "core/driver/root_locker.h"
#include "core/driver/first_child_update_root_locker.h"

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
namespace core {
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
  IDriver::reply(reciver, new events::ProgressResponceEvent(
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
  IDriver::reply(esender, resp);
  notifyProgressImpl(sender, esender, 100);
}

}  // namespace

IDriver::IDriver(IConnectionSettingsBaseSPtr settings)
    : settings_(settings), thread_(nullptr), timer_info_id_(0), log_file_(nullptr) {
  thread_ = new QThread(this);
  moveToThread(thread_);

  VERIFY(connect(thread_, &QThread::started, this, &IDriver::init));
  VERIFY(connect(thread_, &QThread::finished, this, &IDriver::clear));
}

IDriver::~IDriver() {
  destroy(&log_file_);
}

common::Error IDriver::execute(FastoObjectCommandIPtr cmd) {
  if (!cmd) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  common::CommandValue* icmd = cmd->cmd();
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
  common::Error err = executeImpl(argc, exec_argv, cmd.get());
  sdsfreesplitres(argv, argc);
  return err;
}

void IDriver::reply(QObject* reciver, QEvent* ev) {
  qApp->postEvent(reciver, ev);
}

connectionTypes IDriver::type() const {
  return settings_->type();
}

IConnectionSettings::connection_path_t IDriver::connectionPath() const {
  return settings_->path();
}

IServerInfoSPtr IDriver::serverInfo() const {
  if (isConnected()) {
    CHECK(server_info_);
    return server_info_;
  }

  return IServerInfoSPtr();
}

IDataBaseInfoSPtr IDriver::currentDatabaseInfo() const {
  if (isConnected()) {
    CHECK(current_database_info_);
    return current_database_info_;
  }

  return IDataBaseInfoSPtr();
}

void IDriver::start() {
  thread_->start();
}

void IDriver::stop() {
  thread_->quit();
  thread_->wait();
}

void IDriver::interrupt() {
  setInterrupted(true);
}

void IDriver::init() {
  if (settings_->loggingEnabled()) {
    int interval = settings_->loggingMsTimeInterval();
    timer_info_id_ = startTimer(interval);
    DCHECK(timer_info_id_ != 0);
  }
  initImpl();
}

void IDriver::clear() {
  if (timer_info_id_ != 0) {
    killTimer(timer_info_id_);
    timer_info_id_ = 0;
  }
  common::Error err = syncDisconnect();
  if (err && err->isError()) {
    DNOTREACHED();
  }
  clearImpl();
}

void IDriver::customEvent(QEvent* event) {
  setInterrupted(false);

  QEvent::Type type = event->type();
  if (type == static_cast<QEvent::Type>(events::ConnectRequestEvent::EventType)) {
    events::ConnectRequestEvent* ev = static_cast<events::ConnectRequestEvent*>(event);
    handleConnectEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::ShutDownRequestEvent::EventType)) {
    events::ShutDownRequestEvent* ev = static_cast<events::ShutDownRequestEvent*>(event);
    handleShutdownEvent(ev);  // ni
  } else if (type == static_cast<QEvent::Type>(events::ProcessConfigArgsRequestEvent::EventType)) {
    events::ProcessConfigArgsRequestEvent* ev =
        static_cast<events::ProcessConfigArgsRequestEvent*>(event);
    handleProcessCommandLineArgs(ev);
  } else if (type == static_cast<QEvent::Type>(events::DisconnectRequestEvent::EventType)) {
    events::DisconnectRequestEvent* ev = static_cast<events::DisconnectRequestEvent*>(event);
    handleDisconnectEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::ExecuteRequestEvent::EventType)) {
    events::ExecuteRequestEvent* ev = static_cast<events::ExecuteRequestEvent*>(event);
    handleExecuteEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::LoadDatabasesInfoRequestEvent::EventType)) {
    events::LoadDatabasesInfoRequestEvent* ev =
        static_cast<events::LoadDatabasesInfoRequestEvent*>(event);
    handleLoadDatabaseInfosEvent(ev);  //
  } else if (type == static_cast<QEvent::Type>(events::ServerInfoRequestEvent::EventType)) {
    events::ServerInfoRequestEvent* ev = static_cast<events::ServerInfoRequestEvent*>(event);
    handleLoadServerInfoEvent(ev);  //
  } else if (type == static_cast<QEvent::Type>(events::ServerInfoHistoryRequestEvent::EventType)) {
    events::ServerInfoHistoryRequestEvent* ev =
        static_cast<events::ServerInfoHistoryRequestEvent*>(event);
    handleLoadServerInfoHistoryEvent(ev);  //
  } else if (type == static_cast<QEvent::Type>(events::ClearServerHistoryRequestEvent::EventType)) {
    events::ClearServerHistoryRequestEvent* ev =
        static_cast<events::ClearServerHistoryRequestEvent*>(event);
    handleClearServerHistoryRequestEvent(ev);  //
  } else if (type == static_cast<QEvent::Type>(events::ServerPropertyInfoRequestEvent::EventType)) {
    events::ServerPropertyInfoRequestEvent* ev =
        static_cast<events::ServerPropertyInfoRequestEvent*>(event);
    handleLoadServerPropertyEvent(ev);  // ni
  } else if (type ==
             static_cast<QEvent::Type>(events::ChangeServerPropertyInfoRequestEvent::EventType)) {
    events::ChangeServerPropertyInfoRequestEvent* ev =
        static_cast<events::ChangeServerPropertyInfoRequestEvent*>(event);
    handleServerPropertyChangeEvent(ev);  // ni
  } else if (type == static_cast<QEvent::Type>(events::BackupRequestEvent::EventType)) {
    events::BackupRequestEvent* ev = static_cast<events::BackupRequestEvent*>(event);
    handleBackupEvent(ev);  // ni
  } else if (type == static_cast<QEvent::Type>(events::ExportRequestEvent::EventType)) {
    events::ExportRequestEvent* ev = static_cast<events::ExportRequestEvent*>(event);
    handleExportEvent(ev);  // ni
  } else if (type == static_cast<QEvent::Type>(events::ChangePasswordRequestEvent::EventType)) {
    events::ChangePasswordRequestEvent* ev =
        static_cast<events::ChangePasswordRequestEvent*>(event);
    handleChangePasswordEvent(ev);  // ni
  } else if (type ==
             static_cast<QEvent::Type>(events::ChangeMaxConnectionRequestEvent::EventType)) {
    events::ChangeMaxConnectionRequestEvent* ev =
        static_cast<events::ChangeMaxConnectionRequestEvent*>(event);
    handleChangeMaxConnectionEvent(ev);  // ni
  } else if (type ==
             static_cast<QEvent::Type>(events::LoadDatabaseContentRequestEvent::EventType)) {
    events::LoadDatabaseContentRequestEvent* ev =
        static_cast<events::LoadDatabaseContentRequestEvent*>(event);
    handleLoadDatabaseContentEvent(ev);
  } else if (type == static_cast<QEvent::Type>(events::ClearDatabaseRequestEvent::EventType)) {
    events::ClearDatabaseRequestEvent* ev = static_cast<events::ClearDatabaseRequestEvent*>(event);
    handleClearDatabaseEvent(ev);  // ni
  } else if (type == static_cast<QEvent::Type>(events::SetDefaultDatabaseRequestEvent::EventType)) {
    events::SetDefaultDatabaseRequestEvent* ev =
        static_cast<events::SetDefaultDatabaseRequestEvent*>(event);
    handleSetDefaultDatabaseEvent(ev);  // ni
  } else if (type == static_cast<QEvent::Type>(events::DiscoveryInfoRequestEvent::EventType)) {
    events::DiscoveryInfoRequestEvent* ev = static_cast<events::DiscoveryInfoRequestEvent*>(event);
    handleDiscoveryInfoRequestEvent(ev);  //
  }

  return QObject::customEvent(event);
}

void IDriver::timerEvent(QTimerEvent* event) {
  if (timer_info_id_ == event->timerId() && settings_->loggingEnabled() && isConnected()) {
    if (!log_file_) {
      std::string path = settings_->loggingPath();
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
      IServerInfo* info = nullptr;
      common::Error er = serverInfo(&info);
      if (er && er->isError()) {
        QObject::timerEvent(event);
        return;
      }

      ServerInfoSnapShoot shot(time, IServerInfoSPtr(info));
      emit serverInfoSnapShoot(shot);

      log_file_->write(stamp);
      log_file_->write(info->toString());
      log_file_->flush();
    }
  }
  QObject::timerEvent(event);
}

void IDriver::notifyProgress(QObject* reciver, int value) {
  notifyProgressImpl(this, reciver, value);
}

void IDriver::handleConnectEvent(events::ConnectRequestEvent* ev) {
  QObject* sender = ev->sender();
  notifyProgress(sender, 0);
  events::ConnectResponceEvent::value_type res(ev->value());
  notifyProgress(sender, 25);
  common::Error er = syncConnect();
  if (er && er->isError()) {
    res.setErrorInfo(er);
  }
  notifyProgress(sender, 75);
  reply(sender, new events::ConnectResponceEvent(this, res));
  notifyProgress(sender, 100);
}

void IDriver::handleDisconnectEvent(events::DisconnectRequestEvent* ev) {
  QObject* sender = ev->sender();
  notifyProgress(sender, 0);
  events::DisconnectResponceEvent::value_type res(ev->value());
  notifyProgress(sender, 50);

  common::Error er = syncDisconnect();
  if (er && er->isError()) {
    res.setErrorInfo(er);
  }

  reply(sender, new events::DisconnectResponceEvent(this, res));
  notifyProgress(sender, 100);
}

void IDriver::handleExecuteEvent(events::ExecuteRequestEvent* ev) {
  QObject* sender = ev->sender();
  notifyProgress(sender, 0);
  events::ExecuteResponceEvent::value_type res(ev->value());

  const std::string inputLine = res.text;
  if (inputLine.empty()) {
    res.setErrorInfo(common::make_error_value("Empty command line.", common::ErrorValue::E_ERROR));
    reply(sender, new events::ExecuteResponceEvent(this, res));
    notifyProgress(sender, 100);
    return;
  }

  const bool silence = res.silence;
  const size_t repeat = res.repeat;
  const bool history = res.history;
  const common::time64_t msec_repeat_interval = res.msec_repeat_interval;
  size_t length = inputLine.length();
  RootLocker* lock = history ? new RootLocker(this, sender, inputLine, silence)
                             : new FirstChildUpdateRootLocker(this, sender, inputLine, silence);
  FastoObjectIPtr obj = lock->root();
  const double step = 100.0 / double(length * (repeat + 1));
  int cur_progress = 0;
  for (size_t r = 0; r < repeat + 1; ++r) {
    common::time64_t start_ts = common::time::current_mstime();
    int offset = 0;
    for (size_t i = 0; i < length; ++i) {
      if (isInterrupted()) {
        res.setErrorInfo(common::make_error_value(
            "Interrupted exec.", common::ErrorValue::E_INTERRUPTED, common::logging::L_WARNING));
        goto done;
      }

      if (inputLine[i] == '\n' || i == length - 1) {
        cur_progress += step * i;
        notifyProgress(sender, cur_progress);
        std::string command;
        if (i == length - 1) {
          command = inputLine.substr(offset);
        } else {
          command = inputLine.substr(offset, i - offset);
        }

        offset = i + 1;
        FastoObjectCommandIPtr cmd =
            silence ? createCommandFast(command, common::Value::C_USER)
                    : createCommand(obj.get(), command, common::Value::C_USER);  //
        common::Error er = execute(cmd);
        if (er && er->isError()) {
          res.setErrorInfo(er);
          goto done;
        }
      }
    }

    common::time64_t finished_ts = common::time::current_mstime();
    auto diff = finished_ts - start_ts;
    if (msec_repeat_interval > diff) {
      useconds_t sleep_time = msec_repeat_interval - diff;
      common::utils::msleep(sleep_time);
    }
  }

done:
  reply(sender, new events::ExecuteResponceEvent(this, res));
  notifyProgress(sender, 100);
  delete lock;
}

void IDriver::handleLoadServerPropertyEvent(events::ServerPropertyInfoRequestEvent* ev) {
  replyNotImplementedYet<events::ServerPropertyInfoRequestEvent,
                         events::ServerPropertyInfoResponceEvent>(this, ev, "server property");
}

void IDriver::handleServerPropertyChangeEvent(events::ChangeServerPropertyInfoRequestEvent* ev) {
  replyNotImplementedYet<events::ChangeServerPropertyInfoRequestEvent,
                         events::ChangeServerPropertyInfoResponceEvent>(this, ev,
                                                                        "change server property");
}

void IDriver::handleShutdownEvent(events::ShutDownRequestEvent* ev) {
  replyNotImplementedYet<events::ShutDownRequestEvent, events::ShutDownResponceEvent>(this, ev,
                                                                                      "shutdown");
}

void IDriver::handleBackupEvent(events::BackupRequestEvent* ev) {
  replyNotImplementedYet<events::BackupRequestEvent, events::BackupResponceEvent>(this, ev,
                                                                                  "backup server");
}

void IDriver::handleExportEvent(events::ExportRequestEvent* ev) {
  replyNotImplementedYet<events::ExportRequestEvent, events::ExportResponceEvent>(this, ev,
                                                                                  "export server");
}

void IDriver::handleChangePasswordEvent(events::ChangePasswordRequestEvent* ev) {
  replyNotImplementedYet<events::ChangePasswordRequestEvent, events::ChangePasswordResponceEvent>(
      this, ev, "change password");
}

void IDriver::handleChangeMaxConnectionEvent(events::ChangeMaxConnectionRequestEvent* ev) {
  replyNotImplementedYet<events::ChangeMaxConnectionRequestEvent,
                         events::ChangeMaxConnectionResponceEvent>(this, ev,
                                                                   "change maximum connection");
}

void IDriver::handleLoadDatabaseInfosEvent(events::LoadDatabasesInfoRequestEvent* ev) {
  QObject* sender = ev->sender();
  notifyProgress(sender, 0);
  events::LoadDatabasesInfoResponceEvent::value_type res(ev->value());
  notifyProgress(sender, 50);
  IDataBaseInfoSPtr curdb = currentDatabaseInfo();
  CHECK(curdb);
  res.databases.push_back(curdb);
  reply(sender, new events::LoadDatabasesInfoResponceEvent(this, res));
  notifyProgress(sender, 100);
}

void IDriver::handleClearDatabaseEvent(events::ClearDatabaseRequestEvent* ev) {
  replyNotImplementedYet<events::ClearDatabaseRequestEvent, events::ClearDatabaseResponceEvent>(
      this, ev, "clear database");
}

void IDriver::handleSetDefaultDatabaseEvent(events::SetDefaultDatabaseRequestEvent* ev) {
  replyNotImplementedYet<events::SetDefaultDatabaseRequestEvent,
                         events::SetDefaultDatabaseResponceEvent>(this, ev, "set default database");
}

void IDriver::handleLoadServerInfoEvent(events::ServerInfoRequestEvent* ev) {
  QObject* sender = ev->sender();
  notifyProgress(sender, 0);
  events::ServerInfoResponceEvent::value_type res(ev->value());
  notifyProgress(sender, 50);
  IServerInfo* info = nullptr;
  common::Error err = serverInfo(&info);
  if (err && err->isError()) {
    res.setErrorInfo(err);
  } else {
    IServerInfoSPtr mem(info);
    res.setInfo(mem);
  }
  notifyProgress(sender, 75);
  reply(sender, new events::ServerInfoResponceEvent(this, res));
  notifyProgress(sender, 100);
}

void IDriver::handleLoadServerInfoHistoryEvent(events::ServerInfoHistoryRequestEvent* ev) {
  QObject* sender = ev->sender();
  events::ServerInfoHistoryResponceEvent::value_type res(ev->value());

  std::string path = settings_->loggingPath();
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
          tmpInfos.push_back(ServerInfoSnapShoot(
              curStamp, makeServerInfoFromString(common::ConvertToString(dataInfo))));
        }
        break;
      }

      common::time64_t tmpStamp = 0;
      bool isSt = getStamp(data, &tmpStamp);
      if (isSt) {
        if (curStamp) {
          tmpInfos.push_back(ServerInfoSnapShoot(
              curStamp, makeServerInfoFromString(common::ConvertToString(dataInfo))));
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

  reply(sender, new events::ServerInfoHistoryResponceEvent(this, res));
}

void IDriver::handleClearServerHistoryRequestEvent(events::ClearServerHistoryRequestEvent* ev) {
  QObject* sender = ev->sender();
  events::ClearServerHistoryResponceEvent::value_type res(ev->value());

  bool ret = false;

  if (log_file_ && log_file_->isOpened()) {
    ret = log_file_->truncate(0);
  } else {
    std::string path = settings_->loggingPath();
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

  reply(sender, new events::ClearServerHistoryResponceEvent(this, res));
}

void IDriver::handleDiscoveryInfoRequestEvent(events::DiscoveryInfoRequestEvent* ev) {
  QObject* sender = ev->sender();
  notifyProgress(sender, 0);
  events::DiscoveryInfoResponceEvent::value_type res(ev->value());
  notifyProgress(sender, 50);

  if (isConnected()) {
    IServerInfo* info = nullptr;
    IDataBaseInfo* db = nullptr;
    common::Error err = serverDiscoveryInfo(&info, &db);
    if (err && err->isError()) {
      res.setErrorInfo(err);
    } else {
      DCHECK(info);
      DCHECK(db);

      server_info_.reset(info);
      current_database_info_.reset(db);

      res.sinfo = server_info_;
      res.dbinfo = current_database_info_;
    }
  } else {
    res.setErrorInfo(common::make_error_value(
        "Not connected to server, impossible to get discovery info!", common::Value::E_ERROR));
  }

  notifyProgress(sender, 75);
  reply(sender, new events::DiscoveryInfoResponceEvent(this, res));
  notifyProgress(sender, 100);
}

common::Error IDriver::serverDiscoveryInfo(IServerInfo** sinfo, IDataBaseInfo** dbinfo) {
  IServerInfo* lsinfo = nullptr;
  common::Error er = serverInfo(&lsinfo);
  if (er && er->isError()) {
    return er;
  }

  IDataBaseInfo* ldbinfo = nullptr;
  er = currentDataBaseInfo(&ldbinfo);
  if (er && er->isError()) {
    delete lsinfo;
    return er;
  }

  *sinfo = lsinfo;
  *dbinfo = ldbinfo;
  return er;
}

void IDriver::onCurrentDataBaseChanged(IDataBaseInfo* info) {
  current_database_info_.reset(info->Clone());
}

void IDriver::onKeysRemoved(const keys_t& keys) {
  for (size_t i = 0; i < keys.size(); ++i) {
    emit keyRemoved(current_database_info_, keys[i]);
  }
}

void IDriver::onKeyAdded(const key_and_value_t& key) {
  emit keyAdded(current_database_info_, key);
}

void IDriver::onKeyLoaded(const key_and_value_t& key) {
  emit keyLoaded(current_database_info_, key);
}

void IDriver::onKeyRenamed(const key_t& key, const std::string& new_key) {
  emit keyRenamed(current_database_info_, key, new_key);
}

void IDriver::onKeyTTLChanged(const key_t& key, ttl_t ttl) {
  emit keyTTLChanged(current_database_info_, key, ttl);
}
}  // namespace core
}  // namespace fastonosql
