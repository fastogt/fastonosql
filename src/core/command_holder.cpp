/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    FastoNoSQL is free software: you can redistribute it
   and/or modify
    it under the terms of the GNU General Public License as
   published by
    the Free Software Foundation, either version 3 of the
   License, or
    (at your option) any later version.

    FastoNoSQL is distributed in the hope that it will be
   useful,
    but WITHOUT ANY WARRANTY; without even the implied
   warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General
   Public License
    along with FastoNoSQL.  If not, see
   <http://www.gnu.org/licenses/>.
*/

#include "core/command_holder.h"

#include <string>  // for string

#include "common/string_util.h"  // for FullEqualsASCII

namespace fastonosql {
namespace core {

CommandHolder::CommandHolder(const std::string& name,
                             const std::string& params,
                             const std::string& summary,
                             uint32_t since,
                             const std::string& example,
                             uint8_t required_arguments_count,
                             uint8_t optional_arguments_count,
                             function_t func)
    : CommandInfo(name,
                  params,
                  summary,
                  since,
                  example,
                  required_arguments_count,
                  optional_arguments_count),
      func_(func),
      white_spaces_count_(
          std::count_if(name.begin(), name.end(), [](char c) { return std::isspace(c); })) {}

bool CommandHolder::isCommand(int argc, char** argv, size_t* offset) {
  if (white_spaces_count_ == 0) {
    char* cmd = argv[0];
    if (!common::FullEqualsASCII(cmd, name, false)) {
      return false;
    }
  } else {
    if (argc == 1) {
      return false;
    }

    if (white_spaces_count_ > argc) {
      return false;
    }

    std::vector<std::string> merged;
    for (size_t i = 0; i <= white_spaces_count_; ++i) {
      merged.push_back(argv[i]);
    }
    std::string ws = common::JoinString(merged, ' ');
    if (!common::FullEqualsASCII(ws, name, false)) {
      return false;
    }
  }

  if (offset) {
    *offset = white_spaces_count_ + 1;
  }
  return true;
}

common::Error CommandHolder::execute(CommandHandler* handler,
                                     int argc,
                                     char** argv,
                                     FastoObject* out) {
  return func_(handler, argc, argv, out);
}

}  // namespace core
}  // namespace fastonosql
