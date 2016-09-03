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

#include <QDialog>

#include "core/core_fwd.h"  // for IServerSPtr
#include "core/events/events_info.h"

class QComboBox;  // lines 23-23
class QEvent;
class QPushButton;  // lines 24-24
class QShowEvent;
class QWidget;

namespace fasto {
namespace qt {
namespace gui {
class GlassWidget;
}
}
}  // lines 32-32
namespace fasto {
namespace qt {
namespace gui {
class GraphWidget;
}
}
}  // lines 33-33
namespace fastonosql {
namespace core {
struct ServerInfoSnapShoot;
}
}

namespace fastonosql {
namespace gui {

class ServerHistoryDialog : public QDialog {
  Q_OBJECT
 public:
  explicit ServerHistoryDialog(core::IServerSPtr server, QWidget* parent = 0);

  enum { min_width = 640, min_height = 480 };

 private Q_SLOTS:
  void startLoadServerHistoryInfo(const core::events_info::ServerInfoHistoryRequest& req);
  void finishLoadServerHistoryInfo(const core::events_info::ServerInfoHistoryResponce& res);
  void startClearServerHistory(const core::events_info::ClearServerHistoryRequest& req);
  void finishClearServerHistory(const core::events_info::ClearServerHistoryResponce& res);
  void snapShotAdd(core::ServerInfoSnapShoot snapshot);
  void clearHistory();

  void refreshInfoFields(int index);
  void refreshGraph(int index);

 protected:
  virtual void changeEvent(QEvent* e);
  virtual void showEvent(QShowEvent* e);

 private:
  void reset();
  void retranslateUi();
  void requestHistoryInfo();

  QWidget* settingsGraph_;
  QPushButton* clearHistory_;
  QComboBox* serverInfoGroupsNames_;
  QComboBox* serverInfoFields_;

  fasto::qt::gui::GraphWidget* graphWidget_;

  fasto::qt::gui::GlassWidget* glassWidget_;
  core::events_info::ServerInfoHistoryResponce::infos_container_type infos_;
  const core::IServerSPtr server_;
};

}  // namespace gui
}  // namespace fastonosql
