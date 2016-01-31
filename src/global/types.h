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

#include "common/value.h"
#include "common/convert2string.h"

namespace fastonosql {

enum supportedViews
{
  Tree = 0,
  Table,
  Text
};

static const std::string viewsText[] = { "Tree", "Table", "Text" };

class Command {
 public:
  Command();
  Command(const std::string& mess, common::Value::CommandLoggingType commandT);
  const std::string& message() const;
  common::Value::CommandLoggingType type() const;

 private:
  const std::string message_;
  const common::Value::CommandLoggingType type_;
};

}

namespace common {
  std::string convertToString(fastonosql::supportedViews v);
}
