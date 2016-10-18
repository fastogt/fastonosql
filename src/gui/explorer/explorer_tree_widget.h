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

#include <QWidget>

#include "core/core_fwd.h"

class QLineEdit;

namespace fastonosql {
namespace gui {
class ExplorerTreeView;
}
}

namespace fastonosql {
namespace gui {
class ExplorerTreeWidget : public QWidget {
  Q_OBJECT
 public:
  explicit ExplorerTreeWidget(QWidget* parent = 0);

 Q_SIGNALS:
  void consoleOpened(core::IServerSPtr server, const QString& text);
  void serverClosed(core::IServerSPtr server);
  void sentinelClosed(core::ISentinelSPtr sentinel);
  void clusterClosed(core::IClusterSPtr cluster);

 public Q_SLOTS:
  void addServer(core::IServerSPtr server);
  void removeServer(core::IServerSPtr server);

  void addSentinel(core::ISentinelSPtr sentinel);
  void removeSentinel(core::ISentinelSPtr sentinel);

  void addCluster(core::IClusterSPtr cluster);
  void removeCluster(core::IClusterSPtr cluster);

 protected:
  virtual void changeEvent(QEvent* e) override;

 private:
  void retranslateUi();

  QLineEdit* filter_edit_;
  ExplorerTreeView* view_;
};
}  // namespace gui
}  // namespace fastonosql
