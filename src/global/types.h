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

#include <string>

#include "common/smart_ptr.h"

namespace fastonosql { class FastoObjectCommand; }

namespace fastonosql {

enum supportedViews {
  Tree = 0,
  Table,
  Text
};

static const std::string viewsText[] = { "Tree", "Table", "Text" };

class Command {
 public:
  typedef common::shared_ptr<FastoObjectCommand> cmd_t;
  Command();
  explicit Command(FastoObjectCommand* cmd);  // take ownerships

  cmd_t cmd() const;

 private:
  cmd_t cmd_;
};

}  // namespace fastonosql

namespace common {
  std::string ConvertToString(fastonosql::supportedViews v);
}  // namespace common
