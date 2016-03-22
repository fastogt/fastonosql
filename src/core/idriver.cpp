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

#include "core/idriver.h"

#ifdef OS_WIN
#include <winsock2.h>
#else
#include <signal.h>
#endif

extern "C" {
  #include "sds.h"
}

#include <string>

#include <QThread>
#include <QApplication>

#include "common/file_system.h"
#include "common/time.h"
#include "common/sprintf.h"
#include "common/utils.h"

#include "core/command_logger.h"

namespace {

#ifdef OS_WIN
struct WinsockInit {
  WinsockInit() {
    WSADATA d;
    if (WSAStartup(MAKEWORD(2, 2), &d) != 0 ) {
      _exit(1);
    }
  }
  ~WinsockInit() { WSACleanup(); }
} winsock_init;
#else
struct SigIgnInit {
  SigIgnInit() {
    signal(SIGPIPE, SIG_IGN);
  }
} sig_init;
#endif

const char magicNumber = 0x1E;
std::string createStamp(common::time64_t time) {
  return magicNumber + common::convertToString(time) + '\n';
}

bool getStamp(common::buffer_type stamp, common::time64_t* timeOut) {
  if (stamp.empty()) {
    return false;
  }

  if (stamp[0] != magicNumber) {
    return false;
  }

  if (stamp[stamp.size() - 1] == '\n') {
    stamp.pop_back();
  }

  common::time64_t ltimeOut = common::convertFromString<common::time64_t>((const char*)(stamp.data() + 1));
  *timeOut = ltimeOut;
  return ltimeOut != 0;
}

}

namespace fastonosql {
namespace {

void notifyProgressImpl(IDriver* sender, QObject *reciver, int value) {
  IDriver::reply(reciver, new events::ProgressResponceEvent(sender, events::ProgressResponceEvent::value_type(value)));
}

template<typename event_request_type, typename event_responce_type>
void replyNotImplementedYet(IDriver* sender, event_request_type* ev, const char* eventCommandText) {
  QObject* esender = ev->sender();
  notifyProgressImpl(sender, esender, 0);
  typename event_responce_type::value_type res(ev->value());

  char patternResult[1024] = {0};
  common::SNPrintf(patternResult, sizeof(patternResult),
                   "Sorry, but now " PROJECT_NAME_TITLE " not supported %s.", eventCommandText);

  common::Error er = common::make_error_value(patternResult, common::ErrorValue::E_ERROR);
  res.setErrorInfo(er);
  event_responce_type* resp = new event_responce_type(sender, res);
  IDriver::reply(esender, resp);
  notifyProgressImpl(sender, esender, 100);
}

}

common::Error IDriver::execute(FastoObjectCommand* cmd) {
  DCHECK(cmd);
  if (!cmd) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  common::CommandValue* icmd = cmd->cmd();
  std::string command = icmd->inputCommand();
  const char* ccommand = common::utils::c_strornull(command);
  if (!ccommand) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  LOG_COMMAND(type(), Command(icmd));
  int argc;
  sds* argv = sdssplitargs(ccommand, &argc);

  if (argv) {
    common::Error err = executeImpl(argc, argv, cmd);
    sdsfreesplitres(argv, argc);
    return err;
  }

  return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
}

IDriver::IDriver(IConnectionSettingsBaseSPtr settings)
  : settings_(settings), interrupt_(false), server_disc_info_(), thread_(nullptr),
    timer_info_id_(0), log_file_(nullptr) {
  thread_ = new QThread(this);
  moveToThread(thread_);

  VERIFY(connect(thread_, &QThread::started, this, &IDriver::init));
  VERIFY(connect(thread_, &QThread::finished, this, &IDriver::clear));
}

IDriver::~IDriver() {
  destroy(&log_file_);
}

void IDriver::reply(QObject *reciver, QEvent *ev) {
  qApp->postEvent(reciver, ev);
}

connectionTypes IDriver::type() const {
  return settings_->type();
}

ServerDiscoveryInfoSPtr IDriver::serverDiscoveryInfo() const {
  return server_disc_info_;
}

std::string IDriver::connectionName() const {
  return settings_->name();
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

common::Error IDriver::commandByType(CommandKeySPtr command, std::string* cmdstring) const {
  if (!command || !cmdstring) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  CommandKey::cmdtype t = command->type();

  if (t == CommandKey::C_DELETE) {
    CommandDeleteKey* delc = dynamic_cast<CommandDeleteKey*>(command.get());
    if (!delc) {
      DNOTREACHED();
      return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
    }
    return commandDeleteImpl(delc, cmdstring);
  } else if (t == CommandKey::C_LOAD) {
    CommandLoadKey* loadc = dynamic_cast<CommandLoadKey*>(command.get());
    if (!loadc) {
      DNOTREACHED();
      return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
    }
    return commandLoadImpl(loadc, cmdstring);
  } else if (t == CommandKey::C_CREATE) {
    CommandCreateKey* createc = dynamic_cast<CommandCreateKey*>(command.get());
    if (!createc) {
      DNOTREACHED();
      return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
    }
    return commandCreateImpl(createc, cmdstring);
  } else if (t == CommandKey::C_CHANGE_TTL) {
    CommandChangeTTL* changettl = dynamic_cast<CommandChangeTTL*>(command.get());
    if (!changettl) {
      DNOTREACHED();
      return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
    }
    return commandChangeTTLImpl(changettl, cmdstring);
  } else {
    NOTREACHED();
    return common::make_error_value("Unknown command", common::ErrorValue::E_ERROR);
  }
}

void IDriver::interrupt() {
  interrupt_ = true;
}

void IDriver::init() {
  int interval = settings_->loggingMsTimeInterval();
  timer_info_id_ = startTimer(interval);
  DCHECK(timer_info_id_);
  initImpl();
}

void IDriver::clear() {
  killTimer(timer_info_id_);
  timer_info_id_ = 0;
  clearImpl();
}

void IDriver::customEvent(QEvent *event) {
  using namespace events;
  QEvent::Type type = event->type();
  if (type == static_cast<QEvent::Type>(ConnectRequestEvent::EventType)) {
    ConnectRequestEvent *ev = static_cast<ConnectRequestEvent*>(event);
    handleConnectEvent(ev);
  } else if (type == static_cast<QEvent::Type>(ShutDownRequestEvent::EventType)) {
    ShutDownRequestEvent *ev = static_cast<ShutDownRequestEvent*>(event);
    handleShutdownEvent(ev);
  } else if (type == static_cast<QEvent::Type>(ProcessConfigArgsRequestEvent::EventType)) {
    ProcessConfigArgsRequestEvent *ev = static_cast<ProcessConfigArgsRequestEvent*>(event);
    handleProcessCommandLineArgs(ev);
  } else if (type == static_cast<QEvent::Type>(DisconnectRequestEvent::EventType)) {
    DisconnectRequestEvent *ev = static_cast<DisconnectRequestEvent*>(event);
    handleDisconnectEvent(ev);
  } else if (type == static_cast<QEvent::Type>(ExecuteRequestEvent::EventType)) {
    ExecuteRequestEvent *ev = static_cast<ExecuteRequestEvent*>(event);
    handleExecuteEvent(ev);
  } else if (type == static_cast<QEvent::Type>(LoadDatabasesInfoRequestEvent::EventType)) {
    LoadDatabasesInfoRequestEvent *ev = static_cast<LoadDatabasesInfoRequestEvent*>(event);
    handleLoadDatabaseInfosEvent(ev);
  } else if (type == static_cast<QEvent::Type>(ServerInfoRequestEvent::EventType)) {
    ServerInfoRequestEvent *ev = static_cast<ServerInfoRequestEvent*>(event);
    handleLoadServerInfoEvent(ev);
  } else if (type == static_cast<QEvent::Type>(ServerInfoHistoryRequestEvent::EventType)) {
    ServerInfoHistoryRequestEvent *ev = static_cast<ServerInfoHistoryRequestEvent*>(event);
    handleLoadServerInfoHistoryEvent(ev);
  } else if (type == static_cast<QEvent::Type>(ClearServerHistoryRequestEvent::EventType)) {
    ClearServerHistoryRequestEvent *ev = static_cast<ClearServerHistoryRequestEvent*>(event);
    handleClearServerHistoryRequestEvent(ev);
  } else if (type == static_cast<QEvent::Type>(ServerPropertyInfoRequestEvent::EventType)) {
    ServerPropertyInfoRequestEvent *ev = static_cast<ServerPropertyInfoRequestEvent*>(event);
    handleLoadServerPropertyEvent(ev);
  } else if (type == static_cast<QEvent::Type>(ChangeServerPropertyInfoRequestEvent::EventType)) {
    ChangeServerPropertyInfoRequestEvent *ev = static_cast<ChangeServerPropertyInfoRequestEvent*>(event);
    handleServerPropertyChangeEvent(ev);
  } else if (type == static_cast<QEvent::Type>(BackupRequestEvent::EventType)) {
    BackupRequestEvent *ev = static_cast<BackupRequestEvent*>(event);
    handleBackupEvent(ev);
  } else if (type == static_cast<QEvent::Type>(ExportRequestEvent::EventType)) {
    ExportRequestEvent *ev = static_cast<ExportRequestEvent*>(event);
    handleExportEvent(ev);
  } else if (type == static_cast<QEvent::Type>(ChangePasswordRequestEvent::EventType)) {
    ChangePasswordRequestEvent *ev = static_cast<ChangePasswordRequestEvent*>(event);
    handleChangePasswordEvent(ev);
  } else if (type == static_cast<QEvent::Type>(ChangeMaxConnectionRequestEvent::EventType)) {
    ChangeMaxConnectionRequestEvent *ev = static_cast<ChangeMaxConnectionRequestEvent*>(event);
    handleChangeMaxConnectionEvent(ev);
  } else if (type == static_cast<QEvent::Type>(LoadDatabaseContentRequestEvent::EventType)) {
    LoadDatabaseContentRequestEvent *ev = static_cast<LoadDatabaseContentRequestEvent*>(event);
    handleLoadDatabaseContentEvent(ev);
  } else if (type == static_cast<QEvent::Type>(ClearDatabaseRequestEvent::EventType)) {
    ClearDatabaseRequestEvent *ev = static_cast<ClearDatabaseRequestEvent*>(event);
    handleClearDatabaseEvent(ev);
  } else if (type == static_cast<QEvent::Type>(SetDefaultDatabaseRequestEvent::EventType)) {
    SetDefaultDatabaseRequestEvent *ev = static_cast<SetDefaultDatabaseRequestEvent*>(event);
    handleSetDefaultDatabaseEvent(ev);
  } else if (type == static_cast<QEvent::Type>(CommandRequestEvent::EventType)) {
    events::CommandRequestEvent* ev = static_cast<events::CommandRequestEvent*>(event);
    handleCommandRequestEvent(ev);
  } else if (type == static_cast<QEvent::Type>(DiscoveryInfoRequestEvent::EventType)) {
    events::DiscoveryInfoRequestEvent* ev = static_cast<events::DiscoveryInfoRequestEvent*>(event);
    handleDiscoveryInfoRequestEvent(ev);
  }

  interrupt_ = false;
  return QObject::customEvent(event);
}

void IDriver::timerEvent(QTimerEvent* event) {
  if (timer_info_id_ == event->timerId() && isConnected() && settings_->loggingEnabled()) {
    if (!log_file_) {
      std::string path = settings_->loggingPath();
      std::string dir = common::file_system::get_dir_path(path);
      common::Error err = common::file_system::create_directory(dir, true);
      if (err && err->isError()) {

      }
      if (common::file_system::is_directory(dir) == common::SUCCESS) {
        common::file_system::Path p(path);
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

void IDriver::notifyProgress(QObject *reciver, int value) {
  notifyProgressImpl(this, reciver, value);
}

void IDriver::handleLoadServerPropertyEvent(events::ServerPropertyInfoRequestEvent* ev) {
  replyNotImplementedYet<events::ServerPropertyInfoRequestEvent, events::ServerPropertyInfoResponceEvent>(this, ev, "server property command");
}

void IDriver::handleServerPropertyChangeEvent(events::ChangeServerPropertyInfoRequestEvent* ev) {
  replyNotImplementedYet<events::ChangeServerPropertyInfoRequestEvent, events::ChangeServerPropertyInfoResponceEvent>(this, ev, "change server property command");
}

void IDriver::handleShutdownEvent(events::ShutDownRequestEvent* ev) {
  replyNotImplementedYet<events::ShutDownRequestEvent, events::ShutDownResponceEvent>(this, ev, "shutdown command");
}

void IDriver::handleBackupEvent(events::BackupRequestEvent* ev) {
  replyNotImplementedYet<events::BackupRequestEvent, events::BackupResponceEvent>(this, ev, "backup server command");
}

void IDriver::handleExportEvent(events::ExportRequestEvent* ev) {
  replyNotImplementedYet<events::ExportRequestEvent, events::ExportResponceEvent>(this, ev, "export server command");
}

void IDriver::handleChangePasswordEvent(events::ChangePasswordRequestEvent* ev) {
  replyNotImplementedYet<events::ChangePasswordRequestEvent, events::ChangePasswordResponceEvent>(this, ev, "change password command");
}

void IDriver::handleChangeMaxConnectionEvent(events::ChangeMaxConnectionRequestEvent* ev) {
  replyNotImplementedYet<events::ChangeMaxConnectionRequestEvent, events::ChangeMaxConnectionResponceEvent>(this, ev, "change maximum connection command");
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
  replyNotImplementedYet<events::ClearDatabaseRequestEvent, events::ClearDatabaseResponceEvent>(this, ev, "clear database command");
}

void IDriver::handleSetDefaultDatabaseEvent(events::SetDefaultDatabaseRequestEvent* ev) {
  replyNotImplementedYet<events::SetDefaultDatabaseRequestEvent, events::SetDefaultDatabaseResponceEvent>(this, ev, "set default database command");
}

IDriver::RootLocker::RootLocker(IDriver* parent, QObject* receiver, const std::string& text)
  : parent_(parent), receiver_(receiver), tstart_(common::time::current_mstime()) {
  DCHECK(parent_);
  root_ = FastoObject::createRoot(text, parent_);
  events::CommandRootCreatedEvent::value_type res(this, root_);
  reply(receiver_, new events::CommandRootCreatedEvent(parent_, res));
}

IDriver::RootLocker::~RootLocker() {
  events::CommandRootCompleatedEvent::value_type res(this, tstart_, root_);
  reply(receiver_, new events::CommandRootCompleatedEvent(parent_, res));
}

void IDriver::setCurrentDatabaseInfo(IDataBaseInfo *inf) {
  current_database_info_.reset(inf);
}

void IDriver::handleLoadServerInfoEvent(events::ServerInfoRequestEvent* ev) {
  QObject* sender = ev->sender();
  notifyProgress(sender, 0);
  events::ServerInfoResponceEvent::value_type res(ev->value());
  notifyProgress(sender, 50);
  IServerInfo* info = NULL;
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

void IDriver::handleLoadServerInfoHistoryEvent(events::ServerInfoHistoryRequestEvent *ev) {
  QObject* sender = ev->sender();
  events::ServerInfoHistoryResponceEvent::value_type res(ev->value());

  std::string path = settings_->loggingPath();
  common::file_system::Path p(path);
  common::file_system::File readFile(p);
  if (readFile.open("rb")) {
    events::ServerInfoHistoryResponceEvent::value_type::infos_container_type tmpInfos;

    common::time64_t curStamp = 0;
    common::buffer_type dataInfo;

    while(!readFile.isEof()) {
      common::buffer_type data;
      bool res = readFile.readLine(&data);
      if (!res || readFile.isEof()) {
        if (curStamp) {
          tmpInfos.push_back(ServerInfoSnapShoot(curStamp, makeServerInfoFromString(common::convertToString(dataInfo))));
        }
        break;
      }

      common::time64_t tmpStamp = 0;
      bool isSt = getStamp(data, &tmpStamp);
      if (isSt) {
        if (curStamp) {
          tmpInfos.push_back(ServerInfoSnapShoot(curStamp, makeServerInfoFromString(common::convertToString(dataInfo))));
        }
        curStamp = tmpStamp;
        dataInfo.clear();
      } else {
        dataInfo.insert(dataInfo.end(), data.begin(), data.end());
      }
    }
    res.setInfos(tmpInfos);
  } else {
    res.setErrorInfo(common::make_error_value("Logging file not found", common::ErrorValue::E_ERROR));
  }

  reply(sender, new events::ServerInfoHistoryResponceEvent(this, res));
}

void IDriver::handleClearServerHistoryRequestEvent(events::ClearServerHistoryRequestEvent *ev) {
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
  events::DiscoveryInfoResponceEvent::value_type res(ev->value());

  if (isConnected()) {
    ServerDiscoveryInfo* disc = nullptr;
    IServerInfo* info = nullptr;
    IDataBaseInfo* db = nullptr;
    common::Error err = serverDiscoveryInfo(&disc, &info, &db);
    if (err && err->isError()) {
      res.setErrorInfo(err);
    } else {
      DCHECK(info);
      DCHECK(db);
      server_info_.reset(info);
      server_disc_info_.reset(disc);
      current_database_info_.reset(db);

      res.sinfo = server_info_;
      res.dinfo = server_disc_info_;
      res.dbinfo = current_database_info_;
    }
  } else {
    res.setErrorInfo(common::make_error_value("Not connected to server, impossible to get discovery info!", common::Value::E_ERROR));
  }

  reply(sender, new events::DiscoveryInfoResponceEvent(this, res));
}

void IDriver::addedChildren(FastoObject* child) {
  DCHECK(child);
  if (!child) {
    return;
  }

  emit addedChild(child);
}

void IDriver::updated(FastoObject* item, common::Value* val) {
  emit itemUpdated(item, val);
}

IDriverLocal::IDriverLocal(IConnectionSettingsBaseSPtr settings)
  : IDriver(settings) {
  CHECK(!isRemoteType(type()));
}

IDriverRemote::IDriverRemote(IConnectionSettingsBaseSPtr settings)
  : IDriver(settings) {
  CHECK(isRemoteType(type()));
}

}  // namespace fastonosql
