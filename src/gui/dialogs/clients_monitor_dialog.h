/*  Copyright (C) 2014-2019 FastoGT. All right reserved.

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

#include "gui/dialogs/base_dialog.h"

#include "proxy/proxy_fwd.h"

class QLabel;
class QLineEdit;
class QSortFilterProxyModel;

namespace fastonosql {
namespace proxy {
namespace events_info {
struct LoadServerClientsRequest;
struct LoadServerClientsResponse;
struct ExecuteInfoRequest;
struct ExecuteInfoResponse;
}  // namespace events_info
}  // namespace proxy
namespace gui {
class FastoTableView;
class ClientsTableModel;

class ClientsMonitorDialog : public BaseDialog {
  Q_OBJECT

 public:
  typedef BaseDialog base_class;
  template <typename T, typename... Args>
  friend T* createDialog(Args&&... args);
  enum { min_width = 800, min_height = 600 };

 private Q_SLOTS:
  void startLoadServerClients(const proxy::events_info::LoadServerClientsRequest& req);
  void finishLoadServerClients(const proxy::events_info::LoadServerClientsResponse& res);

  void startExecuteCommand(const proxy::events_info::ExecuteInfoRequest& req);
  void finishExecuteCommand(const proxy::events_info::ExecuteInfoResponse& res);

  void showContextMenu(const QPoint& point);
  void updateClicked();
  void killClient();

 protected:
  explicit ClientsMonitorDialog(const QString& title,
                                const QIcon& icon,
                                proxy::IServerSPtr server,
                                QWidget* parent = Q_NULLPTR);

  void showEvent(QShowEvent* e) override;

  void retranslateUi() override;

  QModelIndex selectedIndex() const;

 private:
  QPushButton* update_button_;
  FastoTableView* clients_table_;
  ClientsTableModel* clients_model_;
  QSortFilterProxyModel* proxy_model_;
  proxy::IServerSPtr server_;
};

}  // namespace gui
}  // namespace fastonosql
