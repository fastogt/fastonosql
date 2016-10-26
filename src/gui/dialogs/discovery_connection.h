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

#include <vector>  // for vector

#include <QObject>

#include <common/types.h>  // for time64_t

#include "core/connection_settings/connection_settings.h"  // for IConnectionSettingsBaseSPtr
#include "core/server/iserver_info.h"

namespace fastonosql {
namespace gui {

class DiscoveryConnection : public QObject {
  Q_OBJECT
 public:
  explicit DiscoveryConnection(core::IConnectionSettingsBaseSPtr conn, QObject* parent = 0);

 Q_SIGNALS:
  void ConnectionResult(bool suc,
                        qint64 msTimeExecute,
                        const QString& resultText,
                        std::vector<core::ServerDiscoveryClusterInfoSPtr> infos);

 public Q_SLOTS:
  void Routine();

 private:
  core::IConnectionSettingsBaseSPtr connection_;
  common::time64_t start_time_;
};

}  // namespace gui
}  // namespace fastonosql
