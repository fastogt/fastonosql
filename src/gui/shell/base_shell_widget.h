/*  Copyright (C) 2014-2020 FastoGT. All right reserved.

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

#pragma once

#include <vector>

#include <common/error.h>

#include <fastonosql/core/connection_types.h>
#include <fastonosql/core/database/idatabase_info.h>
#include <fastonosql/core/server/iserver_info.h>

#include "gui/widgets/base_widget.h"
#include "proxy/proxy_fwd.h"

class QPushButton;
class QComboBox;
class QProgressBar;
class QCheckBox;
class QSpinBox;
class QHBoxLayout;
class QToolBar;
class QLabel;

namespace common {
namespace qt {
namespace gui {
class IconLabel;
}
}  // namespace qt
}  // namespace common

namespace fastonosql {
namespace core {
struct CommandInfo;
}
namespace proxy {
namespace events_info {
struct ConnectInfoRequest;
struct ConnectInfoResponse;
struct DisConnectInfoRequest;
struct DisConnectInfoResponse;
struct DiscoveryInfoRequest;
struct DiscoveryInfoResponse;
struct ExecuteInfoRequest;
struct ExecuteInfoResponse;
struct EnterModeInfo;
struct LeaveModeInfo;
struct ProgressInfoResponse;
struct ServerInfoRequest;
class ServerInfoResponse;
}  // namespace events_info
}  // namespace proxy
namespace gui {

class BaseShell;

class BaseShellWidget : public BaseWidget {
  Q_OBJECT

 public:
  typedef BaseWidget base_class;
  template <typename T, typename... Args>
  friend T* createWidget(Args&&... args);

  static const QSize kIconSize;
  static const QSize kShellIconSize;

  static BaseShellWidget* createWidgetFactory(proxy::IServerSPtr server,
                                              const QString& file_path = QString(),
                                              QWidget* parent = Q_NULLPTR);

  QString text() const;

 public Q_SLOTS:
  void setText(const QString& text);
  void executeText(const QString& text);
  void executeArgs(const QString& text, size_t repeat, int interval, bool history);

 private Q_SLOTS:
  void execute();
  void stop();
  void connectToServer();
  void disconnectFromServer();
  void loadFromFile();
  void loadFromFileEmptyPath();
  bool loadFromFile(const QString& path);
  void saveToFileAs();
  void saveToFile();
  void validateClick();
  void helpClick();
  void inputTextChanged();

  void advancedOptionsChange(int state);
  void changeVersionApi(int index);

  void startConnect(const proxy::events_info::ConnectInfoRequest& req);
  void finishConnect(const proxy::events_info::ConnectInfoResponse& res);
  void startDisconnect(const proxy::events_info::DisConnectInfoRequest& req);
  void finishDisconnect(const proxy::events_info::DisConnectInfoResponse& res);

  void progressChange(const proxy::events_info::ProgressInfoResponse& res);

  void enterMode(const proxy::events_info::EnterModeInfo& res);
  void leaveMode(const proxy::events_info::LeaveModeInfo& res);

  void startLoadServerInfo(const proxy::events_info::ServerInfoRequest& req);
  void finishLoadServerInfo(const proxy::events_info::ServerInfoResponse& res);

  void startLoadDiscoveryInfo(const proxy::events_info::DiscoveryInfoRequest& req);
  void finishLoadDiscoveryInfo(const proxy::events_info::DiscoveryInfoResponse& res);

  void startExecute(const proxy::events_info::ExecuteInfoRequest& req);
  void finishExecute(const proxy::events_info::ExecuteInfoResponse& res);

  void serverConnect();
  void serverDisconnect();

 protected:
  explicit BaseShellWidget(proxy::IServerSPtr server,
                           const QString& file_path = QString(),
                           QWidget* parent = Q_NULLPTR);
  void retranslateUi() override;
  void init() override;

  virtual QHBoxLayout* createTopLayout(core::ConnectionType ct);

  // notify methods for derived classes
  virtual void OnServerConnected();
  virtual void OnServerDisconnected();

  virtual void OnStartedLoadServerInfo(const proxy::events_info::ServerInfoRequest& res);
  virtual void OnFinishedLoadServerInfo(const proxy::events_info::ServerInfoResponse& res);

  virtual void OnStartedLoadDiscoveryInfo(const proxy::events_info::DiscoveryInfoRequest& res);
  virtual void OnFinishedLoadDiscoveryInfo(const proxy::events_info::DiscoveryInfoResponse& res);

 private:
  QHBoxLayout* createActionBar();

  common::Error validate(const QString& text);

  void syncConnectionActions();
  void updateServerInfo(core::IServerInfoSPtr inf);
  void updateDefaultDatabase(core::IDataBaseInfoSPtr dbs);
  void updateCommands(const std::vector<const core::CommandInfo*>& commands);

  void updateServerLabel(const QString& text);
  void updateDBLabel(const QString& text);

  const proxy::IServerSPtr server_;
  QPushButton* execute_action_;
  QPushButton* stop_action_;
  QPushButton* connect_action_;
  QPushButton* disconnect_action_;
  QPushButton* load_action_;
  QPushButton* save_action_;
  QPushButton* save_as_action_;
  QPushButton* validate_action_;
  QPushButton* help_action_;
  QLabel* supported_commands_count_;
  QLabel* validated_commands_count_;
  QComboBox* commands_version_api_;

  BaseShell* input_;

  QProgressBar* work_progressbar_;
  common::qt::gui::IconLabel* connection_mode_;
  common::qt::gui::IconLabel* server_name_;
  common::qt::gui::IconLabel* db_name_;

  QCheckBox* advanced_options_;
  QWidget* advanced_options_widget_;
  QSpinBox* repeat_count_;
  QSpinBox* interval_msec_;
  QCheckBox* history_call_;
  QString file_path_;
};

}  // namespace gui
}  // namespace fastonosql
