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

#include "proxy/connection_settings/connection_settings_path.h"

namespace fastonosql {
namespace proxy {

ConnectionSettingsPath::ConnectionSettingsPath() : path_() {}

ConnectionSettingsPath::ConnectionSettingsPath(const std::string& path) : path_(path) {}

ConnectionSettingsPath::ConnectionSettingsPath(const common::file_system::ascii_string_path& path) : path_(path) {}

bool ConnectionSettingsPath::Equals(const ConnectionSettingsPath& path) const {
  return path_.Equals(path.path_);
}

std::string ConnectionSettingsPath::GetName() const {
  std::string path = path_.GetPath();
  return common::file_system::get_file_or_dir_name(path);
}

std::string ConnectionSettingsPath::GetDirectory() const {
  return path_.GetDirectory();
}

std::string ConnectionSettingsPath::ToString() const {
  return common::ConvertToString(path_);
}

ConnectionSettingsPath ConnectionSettingsPath::GetRoot() {
  static const common::file_system::ascii_string_path root(common::file_system::get_separator_string<char>());
  return ConnectionSettingsPath(root);
}

}  // namespace proxy
}  // namespace fastonosql
