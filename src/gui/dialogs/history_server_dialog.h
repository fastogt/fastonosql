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

#pragma once

#include <QDialog>

#include "proxy/events/events_info.h"
#include "proxy/proxy_fwd.h"  // for IServerSPtr

class QComboBox;    // lines 23-23
class QPushButton;  // lines 24-24

namespace common {
namespace qt {
namespace gui {
class GlassWidget;
class GraphWidget;
}  // namespace gui
}  // namespace qt
}  // namespace common

namespace fastonosql {
namespace gui {
class ServerHistoryDialog : public QDialog {
  Q_OBJECT
 public:
  explicit ServerHistoryDialog(const QString& title,
                               const QIcon& icon,
                               proxy::IServerSPtr server,
                               QWidget* parent = Q_NULLPTR);

  enum { min_width = 640, min_height = 480 };

 private Q_SLOTS:
  void startLoadServerHistoryInfo(const proxy::events_info::ServerInfoHistoryRequest& req);
  void finishLoadServerHistoryInfo(const proxy::events_info::ServerInfoHistoryResponce& res);
  void startClearServerHistory(const proxy::events_info::ClearServerHistoryRequest& req);
  void finishClearServerHistory(const proxy::events_info::ClearServerHistoryResponce& res);
  void snapShotAdd(core::ServerInfoSnapShoot snapshot);
  void clearHistory();

  void refreshInfoFields(int index);
  void refreshGraph(int index);

 protected:
  virtual void changeEvent(QEvent* e) override;
  virtual void showEvent(QShowEvent* e) override;

 private:
  void reset();
  void retranslateUi();
  void requestHistoryInfo();

  QWidget* settings_graph_;
  QPushButton* clear_history_;
  QComboBox* server_info_groups_names_;
  QComboBox* server_info_fields_;

  common::qt::gui::GraphWidget* graph_widget_;

  common::qt::gui::GlassWidget* glass_widget_;
  proxy::events_info::ServerInfoHistoryResponce::infos_container_type infos_;
  const proxy::IServerSPtr server_;
};
}  // namespace gui
}  // namespace fastonosql
