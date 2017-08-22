/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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

#include "proxy/proxy_fwd.h"            // for IServerSPtr
#include "proxy/server/iserver_base.h"  // for IServerBase

namespace fastonosql {
namespace proxy {

struct Sentinel {
  typedef std::vector<IServerSPtr> nodes_t;

  IServerSPtr sentinel;
  nodes_t sentinels_nodes;
};

class ISentinel : public IServerBase {
 public:
  typedef Sentinel sentinel_t;
  typedef std::vector<sentinel_t> sentinels_t;

  virtual std::string GetName() const override;

  void AddSentinel(sentinel_t root);
  sentinels_t Sentinels() const;

 protected:
  explicit ISentinel(const std::string& name);

 private:
  const std::string name_;
  sentinels_t sentinels_;
};

}  // namespace proxy
}  // namespace fastonosql
