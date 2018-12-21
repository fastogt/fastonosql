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

#include "gui/widgets/base_widget.h"

#include "proxy/proxy_fwd.h"

class QLineEdit;

namespace fastonosql {
namespace gui {
class ExplorerTreeView;

class ExplorerTreeWidget : public BaseWidget {
  Q_OBJECT

 public:
  typedef BaseWidget base_class;
  template <typename T, typename... Args>
  friend T* createWidget(Args&&... args);

 Q_SIGNALS:
  void consoleOpened(proxy::IServerSPtr server, const QString& text);
  void consoleOpenedAndExecute(proxy::IServerSPtr server, const QString& text);
  void serverClosed(proxy::IServerSPtr server);
#if defined(PRO_VERSION)
  void sentinelClosed(proxy::ISentinelSPtr sentinel);
  void clusterClosed(proxy::IClusterSPtr cluster);
#endif

 public Q_SLOTS:
  void addServer(proxy::IServerSPtr server);
  void removeServer(proxy::IServerSPtr server);

#if defined(PRO_VERSION)
  void addSentinel(proxy::ISentinelSPtr sentinel);
  void removeSentinel(proxy::ISentinelSPtr sentinel);

  void addCluster(proxy::IClusterSPtr cluster);
  void removeCluster(proxy::IClusterSPtr cluster);
#endif
 protected:
  explicit ExplorerTreeWidget(QWidget* parent = Q_NULLPTR);
  void retranslateUi() override;

 private:
  QLineEdit* filter_edit_;
  ExplorerTreeView* view_;
};

}  // namespace gui
}  // namespace fastonosql
