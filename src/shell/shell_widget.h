/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    FastoNoSQL is free software: you can redistribute it
   and/or modify
    it under the terms of the GNU General Public License as
   published by
    the Free Software Foundation, either version 3 of the
   License, or
    (at your option) any later version.

    FastoNoSQL is distributed in the hope that it will be
   useful,
    but WITHOUT ANY WARRANTY; without even the implied
   warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General
   Public License
    along with FastoNoSQL.  If not, see
   <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <QWidget>

#include "core/connection_types.h"  // for connectionTypes

#include "core/core_fwd.h"  // for IServerSPtr

#include "core/types.h"  // for IDataBaseInfoSPtr, etc

class QAction;       // lines 26-26
class QComboBox;     // lines 29-29
class QProgressBar;  // lines 27-27

namespace fasto {
namespace qt {
namespace gui {
class IconLabel;
}
}
}  // lines 34-34
namespace fastonosql {
namespace core {
namespace events_info {
struct ConnectInfoRequest;
}
}
}
namespace fastonosql {
namespace core {
namespace events_info {
struct ConnectInfoResponce;
}
}
}
namespace fastonosql {
namespace core {
namespace events_info {
struct DisConnectInfoRequest;
}
}
}
namespace fastonosql {
namespace core {
namespace events_info {
struct DisConnectInfoResponce;
}
}
}
namespace fastonosql {
namespace core {
namespace events_info {
struct DiscoveryInfoRequest;
}
}
}
namespace fastonosql {
namespace core {
namespace events_info {
struct DiscoveryInfoResponce;
}
}
}
namespace fastonosql {
namespace core {
namespace events_info {
struct EnterModeInfo;
}
}
}
namespace fastonosql {
namespace core {
namespace events_info {
struct LeaveModeInfo;
}
}
}
namespace fastonosql {
namespace core {
namespace events_info {
struct ProgressInfoResponce;
}
}
}
namespace fastonosql {
namespace core {
namespace events_info {
struct SetDefaultDatabaseRequest;
}
}
}
namespace fastonosql {
namespace core {
namespace events_info {
struct SetDefaultDatabaseResponce;
}
}
}
namespace fastonosql {
namespace shell {
class BaseShell;
}
}  // lines 42-42

namespace fastonosql {
namespace shell {

class BaseShellWidget : public QWidget {
  Q_OBJECT
 public:
  explicit BaseShellWidget(core::IServerSPtr server,
                           const QString& filePath = QString(),
                           QWidget* parent = 0);
  virtual ~BaseShellWidget();

  QString text() const;

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

  void startConnect(const core::events_info::ConnectInfoRequest& req);
  void finishConnect(const core::events_info::ConnectInfoResponce& res);
  void startDisconnect(const core::events_info::DisConnectInfoRequest& req);
  void finishDisconnect(const core::events_info::DisConnectInfoResponce& res);

  void startSetDefaultDatabase(const core::events_info::SetDefaultDatabaseRequest& req);
  void finishSetDefaultDatabase(const core::events_info::SetDefaultDatabaseResponce& res);

  void progressChange(const core::events_info::ProgressInfoResponce& res);

  void enterMode(const core::events_info::EnterModeInfo& res);
  void leaveMode(const core::events_info::LeaveModeInfo& res);

  void startLoadDiscoveryInfo(const core::events_info::DiscoveryInfoRequest& res);
  void finishLoadDiscoveryInfo(const core::events_info::DiscoveryInfoResponce& res);

 private:
  void syncConnectionActions();
  void syncServerInfo(core::IServerInfoSPtr inf);
  void updateDefaultDatabase(core::IDataBaseInfoSPtr dbs);
  void initShellByType(core::connectionTypes type);

  const core::IServerSPtr server_;
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

}  // namespace shell
}  // namespace fastonosql
