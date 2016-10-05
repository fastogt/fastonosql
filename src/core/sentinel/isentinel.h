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
#include "core/server/iserver.h"  // for IServerBase

namespace fastonosql {
namespace core {

struct Sentinel {
  typedef std::vector<IServerSPtr> nodes_t;

  IServerSPtr sentinel;
  nodes_t sentinels_nodes;
};

class ISentinel : public IServerBase {
 public:
  typedef Sentinel sentinel_t;
  typedef std::vector<sentinel_t> sentinels_t;

  std::string name() const;

  void addSentinel(sentinel_t root);
  sentinels_t sentinels() const;

 protected:
  explicit ISentinel(const std::string& name);

 private:
  const std::string name_;
  sentinels_t sentinels_;
};

}  // namespace core
}  // namespace fastonosql
