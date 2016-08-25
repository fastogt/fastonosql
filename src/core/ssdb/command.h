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

#include <string>                       // for string

#include "global/global.h"              // for FastoObject (ptr only), etc

namespace fastonosql {
namespace core {
namespace ssdb {

class Command
  : public FastoObjectCommand {
 public:
  Command(FastoObject* parent, common::CommandValue* cmd, const std::string& delimiter,
              const std::string& ns_separator);
  virtual bool isReadOnly() const;
};

}  // namespace ssdb
}  // namespace core
}  // namespace fastonosql
