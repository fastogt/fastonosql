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
    along with FastoNoSQL.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <fastonosql/core/db_key.h>

namespace fastonosql {
namespace gui {

class KeyInfo {
 public:
  typedef std::vector<std::string> splited_namespaces_t;
  KeyInfo(const core::key_t& key, std::string ns_separator);

  std::string keyName() const;
  std::string key() const;
  bool hasNamespace() const;
  size_t nsSplitedSize() const;
  splited_namespaces_t namespaces() const;
  std::string nsSeparator() const;

 private:
  core::key_t key_;
  splited_namespaces_t splited_namespaces_;
  std::string key_name_;
  std::string ns_separator_;
};

}  // namespace gui
}  // namespace fastonosql
