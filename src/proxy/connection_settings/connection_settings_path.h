/*  Copyright (C) 2014-2022 FastoGT. All right reserved.

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

#pragma once

#include <string>

#include <common/file_system/path.h>

namespace fastonosql {
namespace proxy {

class ConnectionSettingsPath {
 public:
  ConnectionSettingsPath();
  explicit ConnectionSettingsPath(const std::string& path);

  std::string GetName() const;
  std::string GetDirectory() const;
  bool Equals(const ConnectionSettingsPath& path) const;
  std::string ToString() const;
  static ConnectionSettingsPath GetRoot();

 private:
  explicit ConnectionSettingsPath(const common::file_system::ascii_string_path& path);
  common::file_system::ascii_string_path path_;
};

inline bool operator==(const ConnectionSettingsPath& r, const ConnectionSettingsPath& l) {
  return r.Equals(l);
}

typedef ConnectionSettingsPath connection_path_t;

}  // namespace proxy
}  // namespace fastonosql
