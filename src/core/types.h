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

#include <deque>

#include <common/byte_writer.h>

namespace common {
class Value;
}

namespace fastonosql {
namespace core {

typedef std::vector<unsigned char> command_buffer_t;
typedef common::unsigned_char_writer<512> command_buffer_writer_t;
typedef std::deque<std::string> commands_args_t;

struct IStateField {
  virtual common::Value* ValueByIndex(unsigned char index) const = 0;
  virtual ~IStateField();
};

command_buffer_t StableCommand(command_buffer_t command);

}  // namespace core
}  // namespace fastonosql
