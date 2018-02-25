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
#ifdef IS_PUBLIC_BUILD
#include <QDateTime>
#endif

#include <common/convert2string.h>
#include <common/hash/md5.h>
#include <common/logger.h>
#include <common/net/socket_tcp.h>
#include <common/qt/convert2string.h>
#include <common/qt/translations/translations.h>

#include "proxy/settings_manager.h"
#include "server/server_config.h"

#include "gui/gui_factory.h"
#include "gui/main_window.h"

#include "gui/dialogs/eula_dialog.h"
#include "gui/dialogs/password_dialog.h"
#include "gui/dialogs/trial_time_dialog.h"

#include "translations/global.h"

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

#ifndef IS_PUBLIC_BUILD
  fastonosql::gui::PasswordDialog password_dialog;
  password_dialog.SetLogin(USER_SPECIFIC_LOGIN);
  password_dialog.SetLoginEnabled(false);
  if (password_dialog.exec() == QDialog::Rejected) {
    return EXIT_FAILURE;
  }

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
    QMessageBox::critical(nullptr, fastonosql::translations::trPassword,
                          QObject::tr("Sorry can't connect to server, for checking your passowrd."));
    return EXIT_FAILURE;
  }

  std::string request;
  common::Error request_err =
      fastonosql::server::GenSubscriptionStateRequest(USER_SPECIFIC_LOGIN, hexed_password, &request);
  if (request_err) {
    QMessageBox::critical(nullptr, fastonosql::translations::trPassword,
                          QObject::tr("Sorry can't generate password request, for checking your passowrd."));
    return EXIT_FAILURE;
  }

  size_t nwrite;
  err = client.Write(request, &nwrite);
  if (err) {
    err = client.Close();
    if (err) {
      DNOTREACHED();
    }
    QMessageBox::critical(nullptr, fastonosql::translations::trPassword,
                          QObject::tr("Sorry can't write request, for checking your passowrd."));
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
    QMessageBox::critical(nullptr, fastonosql::translations::trPassword,
                          QObject::tr("Sorry can't get responce, for checking your passowrd."));
    return EXIT_FAILURE;
  }

  fastonosql::server::JsonRPCError jerror = fastonosql::server::ParseSubscriptionStateResponce(subscribe_reply);
  if (jerror) {
    err = client.Close();
    DCHECK(!err) << "Close client error: " << err->GetDescription();
    std::string message = jerror->GetMessage();
    QString qmessage;
    common::ConvertFromString(message, &qmessage);
    QMessageBox::critical(nullptr, fastonosql::translations::trPassword, QObject::tr("%1, bye.").arg(qmessage));
    return EXIT_FAILURE;
  }
#endif

  // EULA License Agreement
  if (!settings_manager->GetAccpetedEula()) {
    fastonosql::gui::EulaDialog eula_dialog;
    if (eula_dialog.exec() == QDialog::Rejected) {
      return EXIT_FAILURE;
    }
    // EULA accepted
    settings_manager->SetAccpetedEula(true);
  }

// 1514764800 1.1.2018:00:00
#if defined(IS_PUBLIC_BUILD) && defined(EXPIRE_APPLICATION_UTC_TIME)
  const QDateTime cur_time = QDateTime::currentDateTimeUtc();
  const QDateTime end_date = QDateTime::fromTime_t(EXPIRE_APPLICATION_UTC_TIME, Qt::UTC);
  if (cur_time > end_date) {
    QMessageBox::critical(nullptr, fastonosql::translations::trTrial, trExpired);
    return EXIT_FAILURE;
  } else {
    uint32_t ex_count = settings_manager->GetExecCount();
    fastonosql::gui::TrialTimeDialog trial_dialog(fastonosql::translations::trTrial, end_date, ex_count);
    if (trial_dialog.exec() == QDialog::Rejected) {
      return EXIT_FAILURE;
    }
  }
#endif

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
