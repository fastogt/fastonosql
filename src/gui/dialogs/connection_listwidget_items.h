/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    SiteOnYourDevice is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SiteOnYourDevice is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SiteOnYourDevice.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <QTreeWidgetItem>

#include "core/connection_settings.h"

namespace fastonosql {

class ConnectionListWidgetItem
  : public QTreeWidgetItem {
 public:
  explicit ConnectionListWidgetItem(IConnectionSettingsBaseSPtr connection);
  void setConnection(IConnectionSettingsBaseSPtr cons);
  IConnectionSettingsBaseSPtr connection() const;

 private:
  IConnectionSettingsBaseSPtr connection_;
};

class ConnectionListWidgetItemEx
  : public ConnectionListWidgetItem {
 public:
  ConnectionListWidgetItemEx(IConnectionSettingsBaseSPtr connection, serverTypes st);
};

class ClusterConnectionListWidgetItem
      : public QTreeWidgetItem {
 public:
  explicit ClusterConnectionListWidgetItem(IClusterSettingsBaseSPtr connection);
  void setConnection(IClusterSettingsBaseSPtr cons);
  IClusterSettingsBaseSPtr connection() const;

 private:
  IClusterSettingsBaseSPtr connection_;
};

}
