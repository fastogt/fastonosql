/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

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

#include <string>

#include "core/types.h"

#define SSDB_COMMON_LABEL "# Common"

#define SSDB_VERSION_LABEL "version"
#define SSDB_LINKS_LABEL "links"
#define SSDB_TOTAL_CALLS_LABEL "total_calls"
#define SSDB_DBSIZE_LABEL "dbsize"
#define SSDB_BINLOGS_LABEL "binlogs"

namespace fastonosql {
namespace ssdb {

class SsdbServerInfo
  : public ServerInfo {
 public:
  struct Common
    : FieldByIndex {
      Common();
      explicit Common(const std::string& common_text);
      common::Value* valueByIndex(unsigned char index) const;

      std::string version;
      uint32_t links;
      uint32_t total_calls;
      uint32_t dbsize;
      std::string binlogs;
  } common_;

  SsdbServerInfo();
  explicit SsdbServerInfo(const Common& common);
  virtual common::Value* valueByIndexes(unsigned char property, unsigned char field) const;
  virtual std::string toString() const;
  virtual uint32_t version() const;
};

std::ostream& operator << (std::ostream& out, const SsdbServerInfo& value);

SsdbServerInfo* makeSsdbServerInfo(const std::string &content);
SsdbServerInfo* makeSsdbServerInfo(FastoObject *root);

class SsdbDataBaseInfo
  : public IDataBaseInfo {
 public:
  SsdbDataBaseInfo(const std::string& name, bool isDefault, size_t size,
                   const keys_cont_type& keys = keys_cont_type());
  virtual IDataBaseInfo* clone() const;
};

class SsdbCommand
  : public FastoObjectCommand {
 public:
  SsdbCommand(FastoObject* parent, common::CommandValue* cmd, const std::string &delemitr);
  virtual bool isReadOnly() const;
};

}  // namespace ssdb
}  // namespace fastonosql
