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
  typedef core::readable_string_t string_t;
  typedef std::vector<string_t> splited_namespaces_t;
  KeyInfo(const core::key_t& key, const string_t& ns_separator);

  string_t keyName() const;
  string_t key() const;
  bool hasNamespace() const;
  size_t nsSplitedSize() const;
  splited_namespaces_t namespaces() const;
  string_t nsSeparator() const;

 private:
  core::key_t key_;
  splited_namespaces_t splited_namespaces_;
  string_t key_name_;
  string_t ns_separator_;
};

}  // namespace gui
}  // namespace fastonosql
