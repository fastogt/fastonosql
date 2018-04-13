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

#ifdef OS_WIN
#include <winsock2.h>
#else
#include <signal.h>
#endif

#include <QApplication>
#include <QDesktopWidget>
#include <QFile>
#include <QMessageBox>

#include <common/convert2string.h>
#include <common/file_system/file.h>
#include <common/file_system/file_system.h>
#include <common/file_system/string_path_utils.h>
#include <common/hash/md5.h>
#include <common/logger.h>
#include <common/net/socket_tcp.h>
#include <common/qt/convert2string.h>
#include <common/qt/translations/translations.h>

#include "proxy/server_config.h"
#include "proxy/settings_manager.h"

#include "gui/gui_factory.h"
#include "gui/main_window.h"

#include "gui/dialogs/eula_dialog.h"
#include "gui/dialogs/password_dialog.h"
#include "gui/dialogs/trial_time_dialog.h"

#include "translations/global.h"

#define IDENTITY_FILE_NAME "IDENTITY"

namespace {
#ifdef OS_WIN
struct WinsockInit {
  WinsockInit() {
    WSADATA d;
    if (WSAStartup(0x202, &d) != 0) {
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

const QSize preferedSize = QSize(1024, 768);
const QString trExpired = QObject::tr(
    "<h4>Your trial version has expired.</h4>"
    "Please <a href=\"" PROJECT_DOWNLOAD_LINK "\">subscribe</a> and continue using " PROJECT_NAME_TITLE ".");

const QString trDoubleUse_1S = QObject::tr(
    "<h4>You trying to use another's trial version. Login: %1 will be banned.</h4>"
    "Please <a href=\"" PROJECT_DOWNLOAD_LINK "\">subscribe</a> and continue using " PROJECT_NAME_TITLE ".");

const QString trCantCreateIdentity = QObject::tr(
    "<h4>We can't create your identity.</h4>"
    "Please <a href=\"" PROJECT_DOWNLOAD_LINK "\">subscribe</a> and continue using " PROJECT_NAME_TITLE ".");

const QString trCantVerifyIdentity = QObject::tr(
    "<h4>We can't verify your identity, it is unregistered device (device limitation is: 1).</h4>"
    "Please <a href=\"" PROJECT_DOWNLOAD_LINK "\">subscribe</a> and continue using " PROJECT_NAME_TITLE ".");

const QString trIdentityMissmatch = QObject::tr(
    "<h4>Missmatch identity.</h4>"
    "Please <a href=\"" PROJECT_DOWNLOAD_LINK "\">subscribe</a> and continue using " PROJECT_NAME_TITLE ".");

const QString trCantSaveIdentity = QObject::tr(
    "<h4>We can't save your identity.</h4>"
    "Please <a href=\"" PROJECT_DOWNLOAD_LINK "\">subscribe</a> and continue using " PROJECT_NAME_TITLE ".");

const QString trSignin =
    QObject::tr("<b>Please sign in (use the same credentials like on <a href=\"" PROJECT_DOMAIN "\">website</a>)</b>");
}  // namespace

common::ErrnoError prepare_to_start(const std::string& runtime_directory_absolute_path) {
  if (!common::file_system::is_directory_exist(runtime_directory_absolute_path)) {
    common::ErrnoError err = common::file_system::create_directory(runtime_directory_absolute_path, true);
    if (err) {
      return err;
    }
  }

  common::ErrnoError err = common::file_system::node_access(runtime_directory_absolute_path);
  if (err) {
    return err;
  }

  return common::ErrnoError();
}

common::Error ban_user(const fastonosql::proxy::UserInfo& user, const std::string& collision_id) {
#if defined(FASTONOSQL)
  common::net::ClientSocketTcp client(common::net::HostAndPort(FASTONOSQL_HOST, SERVER_REQUESTS_PORT));
#elif defined(FASTOREDIS)
  common::net::ClientSocketTcp client(common::net::HostAndPort(FASTOREDIS_HOST, SERVER_REQUESTS_PORT));
#else
#error please specify url and port of version information
#endif

  common::ErrnoError err = client.Connect();
  if (err) {
    return common::make_error_from_errno(err);
  }

  std::string request;
  common::Error request_err = fastonosql::proxy::GenBanUserRequest(user, collision_id, &request);
  if (request_err) {
    return request_err;
  }

  size_t nwrite;
  err = client.Write(request, &nwrite);
  if (err) {
    common::ErrnoError close_err = client.Close();
    DCHECK(!close_err) << "Close client error: " << close_err->GetDescription();
    return common::make_error_from_errno(err);
  }

  std::string ban_reply;
  size_t nread = 0;
  err = client.Read(&ban_reply, 256, &nread);
  if (err) {
    common::ErrnoError close_err = client.Close();
    DCHECK(!close_err) << "Close client error: " << close_err->GetDescription();
    return common::make_error_from_errno(err);
  }

  common::Error jerror = fastonosql::proxy::ParseGenBanUserResponce(ban_reply);
  if (jerror) {
    common::ErrnoError close_err = client.Close();
    DCHECK(!close_err) << "Close client error: " << close_err->GetDescription();
    return jerror;
  }

  err = client.Close();
  DCHECK(!err) << "Close client error: " << err->GetDescription();
  return common::Error();
}

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  const auto settings_manager = fastonosql::proxy::SettingsManager::GetInstance();
  settings_manager->Load();
  app.setOrganizationName(PROJECT_COMPANYNAME);
  app.setOrganizationDomain(PROJECT_COMPANYNAME_DOMAIN);
  app.setApplicationName(PROJECT_NAME);
  app.setApplicationVersion(PROJECT_VERSION);
  app.setAttribute(Qt::AA_UseHighDpiPixmaps);
  // Cross Platform High DPI support - Qt 5.7
  app.setAttribute(Qt::AA_EnableHighDpiScaling);
  app.setWindowIcon(fastonosql::gui::GuiFactory::GetInstance().GetLogoIcon());  // default icon for app

  // EULA License Agreement
  if (!settings_manager->GetAccpetedEula()) {
    fastonosql::gui::EulaDialog eula_dialog;
    if (eula_dialog.exec() == QDialog::Rejected) {
      return EXIT_FAILURE;
    }
    // EULA accepted
    settings_manager->SetAccpetedEula(true);
  }

  fastonosql::gui::PasswordDialog password_dialog;
  password_dialog.SetDescription(trSignin);
#ifndef IS_PUBLIC_BUILD
  password_dialog.SetLogin(USER_LOGIN);
  password_dialog.SetLoginEnabled(false);
#endif
  if (password_dialog.exec() == QDialog::Rejected) {
    return EXIT_FAILURE;
  }

  const QString login = password_dialog.GetLogin();
  const std::string login_str = common::ConvertToString(login);
  const QString password = password_dialog.GetPassword();
  const std::string password_str = common::ConvertToString(password);
  unsigned char md5_result[MD5_HASH_LENGHT];
  common::hash::MD5_CTX ctx;
  common::hash::MD5_Init(&ctx);
  common::hash::MD5_Update(&ctx, reinterpret_cast<const unsigned char*>(password_str.data()), password_str.size());
  common::hash::MD5_Final(&ctx, md5_result);
  std::string hexed_password = common::utils::hex::encode(std::string(md5_result, md5_result + MD5_HASH_LENGHT), true);

#if defined(FASTONOSQL)
  common::net::ClientSocketTcp client(common::net::HostAndPort(FASTONOSQL_HOST, SERVER_REQUESTS_PORT));
#elif defined(FASTOREDIS)
  common::net::ClientSocketTcp client(common::net::HostAndPort(FASTOREDIS_HOST, SERVER_REQUESTS_PORT));
#else
#error please specify url and port of version information
#endif
  common::ErrnoError err = client.Connect();
  if (err) {
    QMessageBox::critical(nullptr, fastonosql::translations::trAuthentication,
                          QObject::tr("Sorry can't connect to server, for checking your credentials."));
    return EXIT_FAILURE;
  }

  fastonosql::proxy::UserInfo user_info(login_str, hexed_password);
  std::string request;
  common::Error request_err = fastonosql::proxy::GenSubscriptionStateRequest(user_info, &request);
  if (request_err) {
    QMessageBox::critical(nullptr, fastonosql::translations::trAuthentication,
                          QObject::tr("Sorry can't generate password request, for checking your credentials."));
    return EXIT_FAILURE;
  }

  size_t nwrite;
  err = client.Write(request, &nwrite);
  if (err) {
    err = client.Close();
    if (err) {
      DNOTREACHED();
    }
    QMessageBox::critical(nullptr, fastonosql::translations::trAuthentication,
                          QObject::tr("Sorry can't write request, for checking your credentials."));
    return EXIT_FAILURE;
  }

  std::string subscribe_reply;
  size_t nread = 0;
  err = client.Read(&subscribe_reply, 256, &nread);
  if (err) {
    err = client.Close();
    if (err) {
      DNOTREACHED();
    }
    QMessageBox::critical(nullptr, fastonosql::translations::trAuthentication,
                          QObject::tr("Sorry can't get responce, for checking your credentials."));
    return EXIT_FAILURE;
  }

  common::Error jerror = fastonosql::proxy::ParseSubscriptionStateResponce(subscribe_reply, &user_info);
  if (jerror) {
    err = client.Close();
    DCHECK(!err) << "Close client error: " << err->GetDescription();
    QString qmessage;
    common::ConvertFromString(jerror->GetDescription(), &qmessage);
    QMessageBox::critical(nullptr, fastonosql::translations::trAuthentication, QObject::tr("%1, bye.").arg(qmessage));
    return EXIT_FAILURE;
  }

  fastonosql::proxy::UserInfo::SubscriptionState user_sub_state = user_info.GetSubscriptionState();
  if (user_sub_state != fastonosql::proxy::UserInfo::SUBSCRIBED) {
    size_t exec_count = user_info.GetExecCount();
    fastonosql::proxy::user_id_t user_id = user_info.GetUserID();
    const std::string runtime_dir_path = settings_manager->GetSettingsDirPath();  // stabled

    err = prepare_to_start(runtime_dir_path);
    DCHECK(!err) << "Create runtime directory error: " << err->GetDescription();
    const std::string identity_path = runtime_dir_path + IDENTITY_FILE_NAME;

    if (exec_count == 1 && !common::file_system::is_file_exist(identity_path)) {
      common::file_system::File identity_file;
      err = identity_file.Open(identity_path,
                               common::file_system::File::FLAG_CREATE | common::file_system::File::FLAG_WRITE);
      if (err) {
        QMessageBox::critical(nullptr, fastonosql::translations::trTrial, trCantCreateIdentity);
        return EXIT_FAILURE;
      }

      size_t writed;
      err = identity_file.Write(user_id, &writed);
      if (err) {
        QMessageBox::critical(nullptr, fastonosql::translations::trTrial, trCantSaveIdentity);
        return EXIT_FAILURE;
      }

      err = identity_file.Close();
      if (err) {
        QMessageBox::critical(nullptr, fastonosql::translations::trTrial, trCantSaveIdentity);
        return EXIT_FAILURE;
      }
    } else {
      common::file_system::ANSIFile read_file;
      common::ErrnoError err = read_file.Open(identity_path, "rb");
      if (err) {
        QMessageBox::critical(nullptr, fastonosql::translations::trTrial, trCantVerifyIdentity);
        return EXIT_FAILURE;
      }

      fastonosql::proxy::user_id_t readed_id;
      if (!read_file.ReadLine(&readed_id)) {
        read_file.Close();
        QMessageBox::critical(nullptr, fastonosql::translations::trTrial, trCantVerifyIdentity);
        return EXIT_FAILURE;
      }

      read_file.Close();

      if (readed_id != user_info.GetUserID()) {
        ban_user(user_info, readed_id);
        QMessageBox::critical(nullptr, fastonosql::translations::trTrial, trIdentityMissmatch);
        return EXIT_FAILURE;
      }
    }

    time_t expire_application_utc_time = user_info.GetExpireTime();
    const QDateTime cur_time = QDateTime::currentDateTimeUtc();
    const QDateTime end_date = QDateTime::fromTime_t(expire_application_utc_time, Qt::UTC);
    if (cur_time > end_date) {
      QMessageBox::critical(nullptr, fastonosql::translations::trTrial, trExpired);
      return EXIT_FAILURE;
    } else {
      uint32_t ex_count = user_info.GetExecCount();
      fastonosql::gui::TrialTimeDialog trial_dialog(fastonosql::translations::trTrial, end_date, ex_count);
      if (trial_dialog.exec() == QDialog::Rejected) {
        return EXIT_FAILURE;
      }
    }
  }

  settings_manager->SetUserInfo(user_info);

  QFile file(":" PROJECT_NAME_LOWERCASE "/default.qss");
  file.open(QFile::ReadOnly);
  QString styleSheet = QLatin1String(file.readAll());
  app.setStyleSheet(styleSheet);
#if defined(NDEBUG)
  common::logging::LOG_LEVEL level = common::logging::LOG_LEVEL_INFO;
#else
  common::logging::LOG_LEVEL level = common::logging::LOG_LEVEL_DEBUG;
#endif
#if defined(LOG_TO_FILE)
  std::string log_path = common::file_system::prepare_path("~/" PROJECT_NAME_LOWERCASE ".log");
  INIT_LOGGER(PROJECT_NAME_TITLE, log_path, level);
#else
  INIT_LOGGER(PROJECT_NAME_TITLE, level);
#endif

  INIT_TRANSLATION(PROJECT_NAME_LOWERCASE);

  fastonosql::gui::MainWindow win;
  QByteArray win_settings = settings_manager->GetWindowSettings();
  if (!win_settings.isEmpty()) {
    win.restoreGeometry(win_settings);
  } else {
    QRect screenGeometry = app.desktop()->availableGeometry();
    QSize screenSize(screenGeometry.width(), screenGeometry.height());

#ifdef OS_ANDROID
    win.resize(screenSize);
#else
    QSize size(screenGeometry.width() / 2, screenGeometry.height() / 2);
    if (preferedSize.height() <= screenSize.height() && preferedSize.width() <= screenSize.width()) {
      win.resize(preferedSize);
    } else {
      win.resize(size);
    }

    QPoint center = screenGeometry.center();
    win.move(center.x() - win.width() / 2, center.y() - win.height() / 2);
#endif
  }

  win.show();
  int res = app.exec();
  settings_manager->SetWindowSettings(win.saveGeometry());
  settings_manager->Save();
  settings_manager->FreeInstance();
  return res;
}
