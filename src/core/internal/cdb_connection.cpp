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

#include "core/internal/cdb_connection.h"



#include <common/convert2string.h>

#define GET_KEYS_PATTERN_3ARGS_ISI "SCAN %" PRIu64 " MATCH %s COUNT %" PRIu64

namespace fastonosql {
namespace core {

CDBConnectionClient::~CDBConnectionClient() {}

namespace internal {

ConstantCommandsArray::ConstantCommandsArray(std::initializer_list<CommandHolder> l) {
  for (auto it = l.begin(); it != l.end(); ++it) {
    CommandHolder cmd = *it;
    for (auto jt = begin(); jt != end(); ++jt) {
      CommandHolder cmd2 = *jt;
      if (cmd2.IsEqualName(cmd.name)) {
        NOTREACHED() << "Only unique commands can be in array, but command with name: \"" << cmd.name
                     << "\" already exists!";
      }
    }
    push_back(cmd);
  }
}

command_buffer_t GetKeysPattern(uint64_t cursor_in, const std::string& pattern, uint64_t count_keys) {
  command_buffer_writer_t wr;
  wr << "SCAN " << common::ConvertToString(cursor_in) << " MATCH " << pattern << " COUNT "
     << common::ConvertToString(count_keys);
  return wr.str();
}

}  // namespace internal
}  // namespace core
}  // namespace fastonosql
