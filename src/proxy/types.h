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

static const char* supported_views_text[] = {"Tree", "Table", "Text"};

class KeyInfo {
 public:
  typedef std::vector<std::string> splited_namespaces_t;
  KeyInfo(const splited_namespaces_t& splited_namespaces_and_key, std::string ns_separator);

  std::string GetKey() const;
  bool HasNamespace() const;
  std::string GetNspace() const;
  std::string JoinNamespace(size_t pos) const;
  size_t GetNspaceSize() const;

 private:
  splited_namespaces_t splited_namespaces_and_key_;
  std::string ns_separator_;
};

KeyInfo MakeKeyInfo(const core::key_t& key, const std::string& ns_separator);

}  // namespace proxy
}  // namespace fastonosql

namespace common {
std::string ConvertToStsring(fastonosql::proxy::supportedViews v);
bool ConvertFromString(const std::string& from, fastonosql::proxy::supportedViews* out);
}  // namespace common
