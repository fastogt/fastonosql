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

#include "proxy/proxy_fwd.h"  // for IDatabaseSPtr

class QLabel;     // lines 30-30
class QLineEdit;  // lines 28-28
class QSortFilterProxyModel;

namespace fastonosql {
namespace proxy {
namespace events_info {
struct LoadServerChannelsRequest;
struct LoadServerChannelsResponce;
}  // namespace events_info
}  // namespace proxy
namespace gui {
class FastoTableView;
class ChannelsTableModel;

class PubSubDialog : public QDialog {
  Q_OBJECT

 public:
  enum { min_width = 640, min_height = 480 };

  explicit PubSubDialog(const QString& title,
                        const QIcon& icon,
                        proxy::IServerSPtr server,
                        QWidget* parent = Q_NULLPTR);
 Q_SIGNALS:
  void consoleOpenedAndExecute(proxy::IServerSPtr server, const QString& text);

 private Q_SLOTS:
  void startLoadServerChannels(const proxy::events_info::LoadServerChannelsRequest& req);
  void finishLoadServerChannels(const proxy::events_info::LoadServerChannelsResponce& res);

  void searchLineChanged(const QString& text);
  void searchClicked();
  void showContextMenu(const QPoint& point);
  void publish();
  void subscribeInNewConsole();

 protected:
  void changeEvent(QEvent* ev) override;
  QModelIndex selectedIndex() const;

 private:
  void retranslateUi();

  QLineEdit* search_box_;
  QPushButton* search_button_;

  FastoTableView* channels_table_;
  ChannelsTableModel* channels_model_;
  QSortFilterProxyModel* proxy_model_;
  proxy::IServerSPtr server_;
};

}  // namespace gui
}  // namespace fastonosql
