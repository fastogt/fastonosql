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
#include <string>

namespace fastonosql {
namespace core {

typedef char command_buffer_char_t;
typedef std::basic_string<command_buffer_char_t> command_buffer_t;
typedef std::basic_stringstream<command_buffer_char_t> command_buffer_writer_t;
typedef std::deque<command_buffer_t> commands_args_t;

command_buffer_t StableCommand(const command_buffer_t& command);

}  // namespace core
}  // namespace fastonosql
