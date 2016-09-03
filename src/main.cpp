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

#include <QApplication>
#include <QDesktopWidget>

#include "gui/main_window.h"
#include "gui/gui_factory.h"

#include "common/logger.h"

namespace {
const QSize preferedSize = QSize(1024, 768);
}

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  app.setOrganizationName(PROJECT_COMPANYNAME);
  app.setOrganizationDomain(PROJECT_COMPANYNAME_DOMAIN);
  app.setApplicationName(PROJECT_NAME);
  app.setApplicationVersion(PROJECT_VERSION);
  app.setAttribute(Qt::AA_UseHighDpiPixmaps);
  app.setWindowIcon(fastonosql::gui::GuiFactory::instance().logoIcon());  // default icon for app
#if defined(LOG_TO_FILE)
  std::string log_path = common::file_system::prepare_path("~/" PROJECT_NAME_LOWERCASE ".log");
  INIT_LOGGER(PROJECT_NAME_TITLE, log_path);
#else
  INIT_LOGGER(PROJECT_NAME_TITLE);
#endif
#ifdef NDEBUG
  SET_LOG_LEVEL(common::logging::L_INFO);
#else
  SET_LOG_LEVEL(common::logging::L_DEBUG);
#endif

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
  win.move(center.x() - win.width() * 0.5, center.y() - win.height() * 0.5);
#endif

  win.show();
  return app.exec();
}
