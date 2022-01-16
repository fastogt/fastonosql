/*  Copyright (C) 2014-2022 FastoGT. All right reserved.

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

#include <string>

#include "proxy/server/iserver.h"

namespace fastonosql {
namespace proxy {

class IServerLocal : public IServer {
  Q_OBJECT

 public:
  virtual std::string GetPath() const = 0;
  IDatabaseSPtr CreateDatabase(core::IDataBaseInfoSPtr info) override = 0;

 protected:
  explicit IServerLocal(IDriver* drv);
};

}  // namespace proxy
}  // namespace fastonosql
