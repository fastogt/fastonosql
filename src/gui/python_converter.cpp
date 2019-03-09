/*  Copyright (C) 2014-2018 FastoGT. All right reserved.

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
    along with FastoNoSQL. If not, see <http://www.gnu.org/licenses/>.
*/

#include "gui/python_converter.h"

#include <common/file_system/string_path_utils.h>
#include <common/qt/convert2string.h>
#include <common/sprintf.h>

#include "gui/text_converter.h"

#include "proxy/settings_manager.h"

#define CONVERT_PICKLE_SCRIPT_NAME "convert_pickle.py"

namespace fastonosql {
namespace gui {

bool string_from_pickle(const convert_in_t& value, convert_out_t* out) {
  if (!out || value.empty()) {
    return false;
  }

  convert_out_t value_xhex;
  if (!string_to_hex(value, &value_xhex)) {
    return false;
  }

  const QString python_path = proxy::SettingsManager::GetInstance()->GetPythonPath();
  if (python_path.isEmpty()) {
    return false;
  }

  const std::string python_path_str = common::ConvertToString(python_path);
  if (!common::file_system::is_file_exist(python_path_str)) {
    return false;
  }

  const std::string converters_path = proxy::SettingsManager::GetInstance()->GetConvertersPath();
  const std::string pickle_script_path = common::file_system::make_path(converters_path, CONVERT_PICKLE_SCRIPT_NAME);
  if (!common::file_system::is_file_exist(pickle_script_path)) {
    return false;
  }

  const std::string cmd = common::MemSPrintf("%s %s %s", python_path_str, pickle_script_path, value_xhex.as_string());
  FILE* fp = popen(cmd.c_str(), "r");
  if (!fp) {
    return false;
  }

  convert_in_t xhexed_res;
  char c;
  do {
    c = fgetc(fp);
    if (c == EOF || c == '\n') {
      break;
    }
    xhexed_res.append(c);
  } while (true);
  pclose(fp);

  if (xhexed_res.empty()) {
    return false;
  }

  convert_out_t clear_data;
  if (!string_from_hex(xhexed_res, &clear_data)) {
    return false;
  }

  *out = clear_data;
  return true;
}

bool string_to_pickle(const convert_in_t& data, convert_out_t* out) {
  if (!out || data.empty()) {
    return false;
  }

  convert_out_t value_xhex;
  if (!string_to_hex(data, &value_xhex)) {
    return false;
  }

  const QString python_path = proxy::SettingsManager::GetInstance()->GetPythonPath();
  if (python_path.isEmpty()) {
    return false;
  }

  const std::string python_path_str = common::ConvertToString(python_path);
  if (!common::file_system::is_file_exist(python_path_str)) {
    return false;
  }

  const std::string converters_path = proxy::SettingsManager::GetInstance()->GetConvertersPath();
  const std::string pickle_script_path = common::file_system::make_path(converters_path, CONVERT_PICKLE_SCRIPT_NAME);
  if (!common::file_system::is_file_exist(pickle_script_path)) {
    return false;
  }

  const std::string cmd =
      common::MemSPrintf("%s %s --encode %s", python_path_str, pickle_script_path, value_xhex.as_string());
  FILE* fp = popen(cmd.c_str(), "r");
  if (!fp) {
    return false;
  }

  convert_in_t xhexed_res;
  char c;
  do {
    c = fgetc(fp);
    if (c == EOF || c == '\n') {
      break;
    }
    xhexed_res.append(c);
  } while (true);
  pclose(fp);

  if (xhexed_res.empty()) {
    return false;
  }

  convert_out_t clear_data;
  if (!string_from_hex(xhexed_res, &clear_data)) {
    return false;
  }

  *out = clear_data;
  return true;
}

}  // namespace gui
}  // namespace fastonosql
