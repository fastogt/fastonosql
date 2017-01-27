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

#pragma once

#include <QDialog>

#include "proxy/proxy_fwd.h"  // for IDatabaseSPtr

class QLabel;     // lines 30-30
class QLineEdit;  // lines 28-28

namespace fastonosql {
namespace proxy {
namespace events_info {
struct ExecuteInfoRequest;
struct ExecuteInfoResponce;
}
}
}
namespace fastonosql {
namespace gui {
class FastoTableView;
class ChannelsTableModel;
}
}

namespace fastonosql {
namespace gui {

class PubSubDialog : public QDialog {
  Q_OBJECT
 public:
  enum {
    min_width = 320,
    min_height = 200
  };

  explicit PubSubDialog(const QString& title, proxy::IServerSPtr server, QWidget* parent = 0);

 private Q_SLOTS:
  void startExecute(const proxy::events_info::ExecuteInfoRequest& req);
  void finishExecute(const proxy::events_info::ExecuteInfoResponce& res);

  void searchLineChanged(const QString& text);
  void searchClicked();

 protected:
  virtual void changeEvent(QEvent* ev) override;

 private:
  void retranslateUi();

  QLineEdit* searchBox_;
  QPushButton* searchButton_;

  FastoTableView* channelsTable_;
  ChannelsTableModel* channelsModel_;
  proxy::IServerSPtr server_;
};

}  // namespace gui
}  // namespace fastonosql
