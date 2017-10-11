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

#include "core/db_key.h"

namespace fastonosql {
namespace proxy {

enum supportedViews { Tree = 0, Table, Text };

extern const std::vector<const char*> supported_views_text;

class KeyInfo {
 public:
  typedef std::vector<std::string> splited_namespaces_t;
  KeyInfo(const core::key_t& key, std::string ns_separator);

  std::string GetKey() const;
  bool HasNamespace() const;
  size_t GetNspaceSize() const;
  splited_namespaces_t GetNamespaces() const;

 private:
  core::key_t key_;
  splited_namespaces_t splited_namespaces_;
  std::string ns_separator_;
};

}  // namespace proxy
}  // namespace fastonosql

namespace common {
std::string ConvertToStsring(fastonosql::proxy::supportedViews v);
bool ConvertFromString(const std::string& from, fastonosql::proxy::supportedViews* out);
}  // namespace common
