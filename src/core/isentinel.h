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

#include <vector>
#include <string>

#include "core/iserver.h"

namespace fastonosql {
namespace core {

class ISentinel
  : public IServerBase {
 public:
  typedef std::vector<IServerSPtr> nodes_type;

  std::string name() const;
  nodes_type nodes() const;
  void addServer(IServerSPtr serv);

  void setRoot(IServerSPtr root);
  IServerSPtr root() const; //sentinel server

 protected:
  explicit ISentinel(const std::string& name);

 private:
  const std::string name_;
  IServerSPtr root_;
  nodes_type nodes_;
};

}  // namespace core
}  // namespace fastonosql
