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

#pragma once

#include "gui/dialogs/base_dialog.h"

#include "proxy/proxy_fwd.h"

class QLabel;
class QLineEdit;
class QSortFilterProxyModel;

namespace fastonosql {
namespace proxy {
namespace events_info {
struct LoadServerChannelsRequest;
struct LoadServerChannelsResponse;
}  // namespace events_info
}  // namespace proxy
namespace gui {
class FastoTableView;
class ChannelsTableModel;

class PubSubDialog : public BaseDialog {
  Q_OBJECT

 public:
  typedef BaseDialog base_class;
  template <typename T, typename... Args>
  friend T* createDialog(Args&&... args);
  enum { min_width = 640, min_height = 480 };

 Q_SIGNALS:
  void consoleOpenedAndExecute(proxy::IServerSPtr server, const QString& text);

 private Q_SLOTS:
  void startLoadServerChannels(const proxy::events_info::LoadServerChannelsRequest& req);
  void finishLoadServerChannels(const proxy::events_info::LoadServerChannelsResponse& res);

  void searchLineChanged(const QString& text);
  void searchClicked();
  void showContextMenu(const QPoint& point);
  void publish();
  void subscribeInNewConsole();

 protected:
  explicit PubSubDialog(const QString& title,
                        const QIcon& icon,
                        proxy::IServerSPtr server,
                        QWidget* parent = Q_NULLPTR);

  void retranslateUi() override;
  QModelIndex selectedIndex() const;

 private:
  QLineEdit* search_box_;
  QPushButton* search_button_;

  FastoTableView* channels_table_;
  ChannelsTableModel* channels_model_;
  QSortFilterProxyModel* proxy_model_;
  proxy::IServerSPtr server_;
};

}  // namespace gui
}  // namespace fastonosql
