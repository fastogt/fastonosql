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

#include <string>  // for string

#define UNDEFINED_SINCE 0x00000000U
#define UNDEFINED_ARGS "Undefined"
#define UNDEFINED_SUMMARY "Undefined"
#define UNDEFINED_SINCE_STR "Undefined"
#define UNDEFINED_EXAMPLE_STR "Unspecified"
#define UNDEFINED_STR_IN_PROGRESS "Undefined in progress"
#define INFINITE_COMMAND_ARGS UINT8_MAX

namespace fastonosql {
namespace core {

struct CommandInfo {
  enum Type { Native, Extended, Internal };
  CommandInfo(const std::string& name,
              const std::string& params,
              const std::string& summary,
              uint32_t since,
              const std::string& example,
              uint8_t required_arguments_count,
              uint8_t optional_arguments_count,
              Type type);

  uint16_t GetMaxArgumentsCount() const;
  uint8_t GetMinArgumentsCount() const;

  bool IsEqualName(const std::string& cmd_name) const;

  const std::string name;
  const std::string params;
  const std::string summary;
  const uint32_t since;
  const std::string example;

  const uint8_t required_arguments_count;
  const uint8_t optional_arguments_count;
  const Type type;
};

std::string ConvertVersionNumberToReadableString(uint32_t version);

}  // namespace core
}  // namespace fastonosql
