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

#include "core/connection_settings.h"

namespace fastonosql {
namespace core {

class IClusterSettingsBase : public IConnectionSettings {
 public:
  typedef IConnectionSettingsBaseSPtr cluster_node_t;
  typedef std::vector<cluster_node_t> cluster_nodes_t;
  cluster_nodes_t nodes() const;

  void addNode(IConnectionSettingsBaseSPtr node);

  static IClusterSettingsBase* createFromType(connectionTypes type,
                                              const connection_path_t& connectionPath);
  static IClusterSettingsBase* fromString(const std::string& val);

  virtual std::string toString() const;
  virtual IClusterSettingsBase* Clone() const = 0;

  virtual IConnectionSettingsBaseSPtr findSettingsByHost(
      const common::net::HostAndPort& host) const;

 protected:
  IClusterSettingsBase(const connection_path_t& connectionName, connectionTypes type);

 private:
  cluster_nodes_t clusters_nodes_;
};

typedef common::shared_ptr<IClusterSettingsBase> IClusterSettingsBaseSPtr;

}  // namespace core
}  // namespace fastonosql
