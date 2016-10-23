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

#include <string>  // for string
#include <vector>  // for vector

#include "core/core_fwd.h"        // for IServerSPtr
#include "core/server/iserver_base.h"

namespace fastonosql {
namespace core {

class ICluster : public IServerBase {
 public:
  typedef IServerSPtr node_t;
  typedef std::vector<node_t> nodes_t;

  virtual std::string Name() const override;
  nodes_t Nodes() const;
  void AddServer(node_t serv);

  node_t Root() const;

 protected:
  explicit ICluster(const std::string& name);

 private:
  const std::string name_;
  nodes_t nodes_;
};

}  // namespace core
}  // namespace fastonosql
