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

#include "common/value.h"

#include "global/global.h"

namespace fastonosql {
namespace core {

std::string StableCommand(std::string command);

template<typename Command>
Command* CreateCommand(FastoObject* parent, const std::string& input,
                                  common::Value::CommandLoggingType ct) {
  if (!parent) {
    NOTREACHED();
    return nullptr;
  }

  std::string stable_input = StableCommand(input);
  if (stable_input.empty()) {
    NOTREACHED();
    return nullptr;
  }

  common::CommandValue* cmd = common::Value::createCommand(stable_input, ct);
  Command* fs = new Command(parent, cmd, parent->delemitr(), parent->nsSeparator());
  parent->addChildren(fs);
  return fs;
}

template<typename Command>
Command* CreateCommand(FastoObjectIPtr parent, const std::string& input,
                                  common::Value::CommandLoggingType ct) {
  return CreateCommand<Command>(parent.get(), input, ct);
}

}  // namespace core
}  // namespace fastonosql
