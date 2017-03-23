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

#include <common/macros.h>  // for NOTREACHED
#include <common/value.h>   // for Value, CommandValue (ptr only), Value::Co...

#include "core/types.h"
#include "core/global.h"  // for FastoObjectCommandIPtr, FastoObject (ptr ...

namespace fastonosql {
namespace proxy {

template <typename Command>
core::FastoObjectCommandIPtr CreateCommand(core::FastoObject* parent,
                                           const std::string& input,
                                           core::CmdLoggingType ct) {
  if (!parent) {
    DNOTREACHED();
    return nullptr;
  }

  std::string stable_input = core::StableCommand(input);
  if (stable_input.empty()) {
    DNOTREACHED();
    return nullptr;
  }

  common::StringValue* cmd = common::Value::CreateStringValue(stable_input);
  core::FastoObjectCommandIPtr fs = new Command(parent, cmd, ct, parent->Delimiter());
  parent->AddChildren(fs);
  return fs;
}

template <typename Command>
core::FastoObjectCommandIPtr CreateCommandFast(const std::string& input, core::CmdLoggingType ct) {
  std::string stable_input = core::StableCommand(input);
  if (stable_input.empty()) {
    DNOTREACHED();
    return nullptr;
  }

  common::StringValue* cmd = common::Value::CreateStringValue(stable_input);
  return new Command(nullptr, cmd, ct, std::string());
}

}  // namespace core
}  // namespace fastonosql
