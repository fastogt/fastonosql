/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    SiteOnYourDevice is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SiteOnYourDevice is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SiteOnYourDevice.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <QWidget>

#include "core/iserver.h"
#include "core/events/events_info.h"

class QAction;
class QProgressBar;
class QToolButton;
class QComboBox;

namespace fasto {
namespace qt {
namespace gui {
class IconLabel;
}
}
}

namespace fastonosql {
class BaseShell;

class BaseShellWidget
  : public QWidget {
  Q_OBJECT
 public:
  explicit BaseShellWidget(IServerSPtr server,
                           const QString& filePath = QString(), QWidget* parent = 0);
  virtual ~BaseShellWidget();

  IServerSPtr server() const;
  QString text() const;

 Q_SIGNALS:
  void startedExecute(const EventsInfo::ExecuteInfoRequest& req);
  void rootCreated(const EventsInfo::CommandRootCreatedInfo& res);
  void rootCompleated(const EventsInfo::CommandRootCompleatedInfo& res);

  void addedChild(FastoObject* child);
  void itemUpdated(FastoObject* item, common::Value* value);

 public Q_SLOTS:
  void setText(const QString& text);
  void executeText(const QString& text);

 private Q_SLOTS:
  void execute();
  void stop();
  void connectToServer();
  void disconnectFromServer();
  void loadFromFile();
  bool loadFromFile(const QString& path);
  void saveToFileAs();
  void saveToFile();

  void changeVersionApi(int index);

  void startConnect(const EventsInfo::ConnectInfoRequest& req);
  void finishConnect(const EventsInfo::ConnectInfoResponce& res);
  void startDisconnect(const EventsInfo::DisConnectInfoRequest& req);
  void finishDisconnect(const EventsInfo::DisConnectInfoResponce& res);

  void startSetDefaultDatabase(const EventsInfo::SetDefaultDatabaseRequest& req);
  void finishSetDefaultDatabase(const EventsInfo::SetDefaultDatabaseResponce& res);

  void progressChange(const EventsInfo::ProgressInfoResponce& res);

  void enterMode(const EventsInfo::EnterModeInfo& res);
  void leaveMode(const EventsInfo::LeaveModeInfo& res);

  void startLoadDiscoveryInfo(const EventsInfo::DiscoveryInfoRequest& res);
  void finishLoadDiscoveryInfo(const EventsInfo::DiscoveryInfoResponce& res);

 private:
  void syncConnectionActions();
  void syncServerInfo(ServerInfoSPtr inf);
  void updateDefaultDatabase(DataBaseInfoSPtr dbs);
  void initShellByType(connectionTypes type);

  const IServerSPtr server_;
  QAction* executeAction_;
  QAction* connectAction_;
  QAction* disConnectAction_;
  QAction* loadAction_;
  QAction* saveAction_;
  QAction* saveAsAction_;
  QComboBox* commandsVersionApi_;

  BaseShell* input_;

  QProgressBar* workProgressBar_;
  fasto::qt::gui::IconLabel* connectionMode_;
  fasto::qt::gui::IconLabel* dbName_;
  QString filePath_;
};

}  // namespace fastonosql
