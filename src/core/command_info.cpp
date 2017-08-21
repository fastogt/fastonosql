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

#include "core/command_info.h"

#include <common/convert2string.h>
#include <common/string_util.h>  // for FullEqualsASCII

namespace fastonosql {
namespace core {

CommandInfo::CommandInfo(const std::string& name,
                         const std::string& params,
                         const std::string& summary,
                         uint32_t since,
                         const std::string& example,
                         uint8_t required_arguments_count,
                         uint8_t optional_arguments_count)
    : name(name),
      params(params),
      summary(summary),
      since(since),
      example(example),
      required_arguments_count(required_arguments_count),
      optional_arguments_count(optional_arguments_count) {}

uint16_t CommandInfo::MaxArgumentsCount() const {
  return required_arguments_count + optional_arguments_count;
}

uint8_t CommandInfo::MinArgumentsCount() const {
  return required_arguments_count;
}

bool CommandInfo::IsEqualName(const std::string& cmd_name) const {
  return common::FullEqualsASCII(cmd_name, name, false);
}

std::string ConvertVersionNumberToReadableString(uint32_t version) {
  if (version != UNDEFINED_SINCE) {
    return common::ConvertVersionNumberTo2DotString(version);
  }

  return UNDEFINED_SINCE_STR;
}

}  // namespace core
}  // namespace fastonosql
