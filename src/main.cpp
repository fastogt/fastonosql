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

#ifdef OS_WIN
#include <winsock2.h>
#else
#include <signal.h>
#endif

#include <QApplication>
#include <QDesktopWidget>
#include <QFile>

#include "gui/gui_factory.h"
#include "gui/main_window.h"

#include <common/logger.h>
#include <common/qt/translations/translations.h>
#include <proxy/settings_manager.h>

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
}

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  fastonosql::proxy::SettingsManager::GetInstance()->Load();
  app.setOrganizationName(PROJECT_COMPANYNAME);
  app.setOrganizationDomain(PROJECT_COMPANYNAME_DOMAIN);
  app.setApplicationName(PROJECT_NAME);
  app.setApplicationVersion(PROJECT_VERSION);
  app.setAttribute(Qt::AA_UseHighDpiPixmaps);
  // Cross Platform High DPI support - Qt 5.7
  app.setAttribute(Qt::AA_EnableHighDpiScaling);
  app.setWindowIcon(fastonosql::gui::GuiFactory::GetInstance().logoIcon());  // default icon for app

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

  win.show();
  int res = app.exec();
  fastonosql::proxy::SettingsManager::GetInstance()->Save();
  fastonosql::proxy::SettingsManager::GetInstance()->FreeInstance();
  return res;
}
