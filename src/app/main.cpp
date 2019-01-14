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
    along with FastoNoSQL. If not, see <http://www.gnu.org/licenses/>.
*/

#if defined(OS_WIN)
#include <winsock2.h>
#else
#include <signal.h>
#endif

#include <QApplication>
#include <QDesktopWidget>
#include <QFile>
#include <QMessageBox>

#include <common/file_system/file.h>
#include <common/file_system/file_system.h>
#include <common/file_system/string_path_utils.h>
#include <common/net/socket_tcp.h>
#include <common/qt/translations/translations.h>
#include <common/time.h>

#include "app/credentials_dialog.h"

#include "proxy/server_config.h"
#include "proxy/settings_manager.h"

#include "gui/gui_factory.h"
#include "gui/main_window.h"

#include "gui/dialogs/eula_dialog.h"
#include "gui/dialogs/how_to_use_dialog.h"
#include "translations/global.h"

#if defined(PRO_VERSION)
#define IDENTITY_FILE_NAME "IDENTITY"
#define COMMUNITY_STRATEGY 0
#define PUBLIC_STRATEGY 1
#define PRIVATE_STRATEGY 2

#if BUILD_STRATEGY == COMMUNITY_STRATEGY || BUILD_STRATEGY == PUBLIC_STRATEGY
#include "app/online_verify_user.h"
#else
#include "app/offline_verify_user.h"
#endif
#endif

namespace {
#if defined(OS_WIN)
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

const QString trEulaTitle = QObject::tr("Eula for " PROJECT_NAME_TITLE);

#if defined(PRO_VERSION)
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

common::ErrnoError prepare_to_start(const std::string& runtime_directory_absolute_path) {
  if (!common::file_system::is_directory_exist(runtime_directory_absolute_path)) {
    common::ErrnoError err = common::file_system::create_directory(runtime_directory_absolute_path, true);
    if (err) {
      return err;
    }
  }

  return common::file_system::node_access(runtime_directory_absolute_path);
}

common::ErrnoError ban_user(const fastonosql::proxy::UserInfo& user, const std::string& collision_id) {
#if defined(FASTONOSQL)
  common::net::ClientSocketTcp client(common::net::HostAndPort(FASTONOSQL_HOST, SERVER_REQUESTS_PORT));
#elif defined(FASTOREDIS)
  common::net::ClientSocketTcp client(common::net::HostAndPort(FASTOREDIS_HOST, SERVER_REQUESTS_PORT));
#else
#error please specify url and port of version information
#endif

  common::ErrnoError err = client.Connect();
  if (err) {
    return err;
  }

  std::string request;
  common::Error request_err = fastonosql::proxy::GenBanUserRequest(user, collision_id, &request);
  if (request_err) {
    return common::make_errno_error(request_err->GetDescription(), EINTR);
  }

  size_t nwrite;
  err = client.WriteBuffer(request, &nwrite);
  if (err) {
    common::ErrnoError close_err = client.Close();
    DCHECK(!close_err) << "Close client error: " << close_err->GetDescription();
    return err;
  }

  std::string ban_reply;
  size_t nread;
  err = client.Read(&ban_reply, 256, &nread);
  if (err) {
    common::ErrnoError close_err = client.Close();
    DCHECK(!close_err) << "Close client error: " << close_err->GetDescription();
    return err;
  }

  common::Error jerror = fastonosql::proxy::ParseGenBanUserResponse(ban_reply);
  if (jerror) {
    common::ErrnoError close_err = client.Close();
    DCHECK(!close_err) << "Close client error: " << close_err->GetDescription();
    return common::make_errno_error(jerror->GetDescription(), EINTR);
  }

  err = client.Close();
  DCHECK(!err) << "Close client error: " << err->GetDescription();
  return common::ErrnoError();
}

class MainCredentialsDialog : public fastonosql::CredentialsDialog {
 public:
  typedef fastonosql::CredentialsDialog base_class;
  template <typename T, typename... Args>
  friend T* fastonosql::gui::createDialog(Args&&... args);

 protected:
  MainCredentialsDialog() : base_class(PROJECT_NAME_TITLE) {}

 private:
  fastonosql::IVerifyUser* createChecker() const override {
#if BUILD_STRATEGY == COMMUNITY_STRATEGY
    return new fastonosql::OnlineVerifyUser(login(), password(), fastonosql::proxy::UserInfo::COMMUNITY_BUILD);
#elif BUILD_STRATEGY == PUBLIC_STRATEGY
    return new fastonosql::OnlineVerifyUser(login(), password(), fastonosql::proxy::UserInfo::PUBLIC_BUILD);
#elif BUILD_STRATEGY == PRIVATE_STRATEGY
    return new fastonosql::OfflineVerifyUser(login(), password(), fastonosql::proxy::UserInfo::PRIVATE_BUILD);
#endif
  }
};
#endif
}  // namespace

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  const auto settings_manager = fastonosql::proxy::SettingsManager::GetInstance();
  settings_manager->Load();
  app.setOrganizationName(PROJECT_COMPANYNAME);
  app.setOrganizationDomain(PROJECT_COMPANYNAME_DOMAIN);
  app.setApplicationName(PROJECT_NAME_TITLE);
  app.setApplicationVersion(PROJECT_VERSION);
  app.setAttribute(Qt::AA_UseHighDpiPixmaps);
  // Cross Platform High DPI support - Qt 5.7
  app.setAttribute(Qt::AA_EnableHighDpiScaling);
  app.setWindowIcon(fastonosql::gui::GuiFactory::GetInstance().logoIcon());  // default icon for app

  // EULA License Agreement
  if (!settings_manager->GetAccpetedEula()) {
    fastonosql::gui::EulaDialog eula_dialog(trEulaTitle);
    if (eula_dialog.exec() == QDialog::Rejected) {
      return EXIT_FAILURE;
    }
    // EULA accepted
    settings_manager->SetAccpetedEula(true);
  }

  {  // scope for password_dialog
#if defined(PRO_VERSION)
    std::unique_ptr<MainCredentialsDialog> password_dialog(
        fastonosql::gui::createDialog<MainCredentialsDialog>());  // +
#if BUILD_STRATEGY == COMMUNITY_STRATEGY
    const QString last_login = settings_manager->GetLastLogin();
    if (!last_login.isEmpty()) {
      password_dialog->setLogin(last_login);
      password_dialog->setFocusInPassword();
    }
#elif BUILD_STRATEGY == PUBLIC_STRATEGY || BUILD_STRATEGY == PRIVATE_STRATEGY
    password_dialog->setLogin(USER_LOGIN);
    password_dialog->setLoginEnabled(false);
#endif
    int dialog_result = password_dialog->exec();
    if (dialog_result == QDialog::Rejected) {
      return EXIT_FAILURE;
    }

    const fastonosql::proxy::UserInfo user_info = password_dialog->userInfo();
    // start application
    const fastonosql::proxy::UserInfo::SubscriptionState user_sub_state = user_info.GetSubscriptionState();
    const size_t exec_count = user_info.GetExecCount();
    if (user_sub_state != fastonosql::proxy::UserInfo::SUBSCRIBED) {
      fastonosql::proxy::user_id_t user_id = user_info.GetUserID();
      const std::string runtime_dir_path = settings_manager->GetSettingsDirPath();  // stabled

      common::ErrnoError err = prepare_to_start(runtime_dir_path);
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
        err = identity_file.WriteBuffer(user_id, &writed);
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

      const time_t expire_application_utc_time = user_info.GetExpireTime();
      const time_t current_time = common::time::current_utc_mstime() / 1000;
      if (current_time > expire_application_utc_time) {
        QMessageBox::critical(nullptr, fastonosql::translations::trTrial, trExpired);
        return EXIT_FAILURE;
      }
    }

    if (exec_count == 1) {
      std::unique_ptr<fastonosql::gui::HowToUseDialog> howto_use_dialog(
          fastonosql::gui::createDialog<fastonosql::gui::HowToUseDialog>());  // +
      howto_use_dialog->exec();
    }

#if BUILD_STRATEGY == COMMUNITY_STRATEGY
    settings_manager->SetLastLogin(password_dialog->login());
#endif
    settings_manager->SetUserInfo(user_info);
#endif
  }

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

  fastonosql::gui::MainWindow main_window;
  QByteArray win_settings = settings_manager->GetMainWindowSettings();
  if (!win_settings.isEmpty()) {
    main_window.restoreGeometry(win_settings);
  } else {
    const QRect screen_geometry = app.desktop()->availableGeometry();
    const QSize screen_size(screen_geometry.width(), screen_geometry.height());

#if defined(OS_ANDROID)
    main_window.resize(screen_size);
#else
    const QSize preferred_size =
        QSize(fastonosql::gui::MainWindow::preferred_width, fastonosql::gui::MainWindow::preferred_height);
    if (preferred_size.width() <= screen_size.width() && preferred_size.height() <= screen_size.height()) {
      main_window.resize(preferred_size);
    }

    QPoint center = screen_geometry.center();
    main_window.move(center.x() - main_window.width() / 2, center.y() - main_window.height() / 2);
#endif
  }

  main_window.show();
  int res = app.exec();
  settings_manager->SetMainWindowSettings(main_window.saveGeometry());
  settings_manager->Save();
  settings_manager->FreeInstance();
  return res;
}
