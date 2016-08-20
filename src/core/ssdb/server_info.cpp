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

#include "core/ssdb/server_info.h"

#include <stddef.h>                     // for size_t

#include <sstream>                      // for operator<<, basic_ostream, etc
#include <string>                       // for string, operator==, etc
#include <utility>                      // for make_pair
#include <vector>                       // for vector

#include "common/convert2string.h"      // for ConvertFromString, etc
#include "common/macros.h"              // for NOTREACHED, DCHECK_EQ
#include "common/value.h"               // for Value, FundamentalValue, etc

#include "core/connection_types.h"      // for connectionTypes::SSDB
#include "core/db_traits.h"

#define MARKER "\r\n"

namespace fastonosql {
namespace core {
namespace {

const std::vector<Field> SsdbCommonFields = {
  Field(SSDB_VERSION_LABEL, common::Value::TYPE_STRING),
  Field(SSDB_LINKS_LABEL, common::Value::TYPE_UINTEGER),
  Field(SSDB_TOTAL_CALLS_LABEL, common::Value::TYPE_UINTEGER),
  Field(SSDB_DBSIZE_LABEL, common::Value::TYPE_UINTEGER),
  Field(SSDB_BINLOGS_LABEL, common::Value::TYPE_STRING)
};

}  // namespace

template<>
std::vector<common::Value::Type> DBTraits<SSDB>::supportedTypes() {
  return  {
              common::Value::TYPE_BOOLEAN,
              common::Value::TYPE_INTEGER,
              common::Value::TYPE_UINTEGER,
              common::Value::TYPE_DOUBLE,
              common::Value::TYPE_STRING,
              common::Value::TYPE_ARRAY,
              common::Value::TYPE_SET,
              common::Value::TYPE_ZSET,
              common::Value::TYPE_HASH
          };
}

template<>
std::vector<info_field_t> DBTraits<SSDB>::infoFields() {
  return { std::make_pair(SSDB_COMMON_LABEL, SsdbCommonFields) };
}

namespace ssdb {

ServerInfo::Common::Common() {
}

ServerInfo::Common::Common(const std::string& common_text) {
  size_t pos = 0;
  size_t start = 0;

  while ((pos = common_text.find(MARKER, start)) != std::string::npos) {
    std::string line = common_text.substr(start, pos-start);
    size_t delem = line.find_first_of(':');
    std::string field = line.substr(0, delem);
    std::string value = line.substr(delem + 1);
    if (field == SSDB_VERSION_LABEL) {
      version = value;
    } else if (field == SSDB_LINKS_LABEL) {
      links = common::ConvertFromString<uint32_t>(value);
    } else if (field == SSDB_TOTAL_CALLS_LABEL) {
      total_calls = common::ConvertFromString<uint32_t>(value);
    } else if (field == SSDB_DBSIZE_LABEL) {
        dbsize = common::ConvertFromString<uint32_t>(value);
    } else if (field == SSDB_BINLOGS_LABEL) {
        binlogs = value;
    }
    start = pos + 2;
  }
}

common::Value* ServerInfo::Common::valueByIndex(unsigned char index) const {
  switch (index) {
  case 0:
    return new common::StringValue(version);
  case 1:
    return new common::FundamentalValue(links);
  case 2:
    return new common::FundamentalValue(total_calls);
  case 3:
    return new common::FundamentalValue(dbsize);
  case 4:
    return new common::StringValue(binlogs);
  default:
    NOTREACHED();
    break;
  }
  return nullptr;
}

ServerInfo::ServerInfo()
  : IServerInfo(SSDB) {
}

ServerInfo::ServerInfo(const Common& common)
  : IServerInfo(SSDB), common_(common) {
}

common::Value* ServerInfo::valueByIndexes(unsigned char property, unsigned char field) const {
  switch (property) {
  case 0:
    return common_.valueByIndex(field);
  default:
    NOTREACHED();
    break;
  }

  return nullptr;
}

std::ostream& operator<<(std::ostream& out, const ServerInfo::Common& value) {
  return out << SSDB_VERSION_LABEL":" << value.version << MARKER
             << SSDB_LINKS_LABEL":" << value.links << MARKER
             << SSDB_TOTAL_CALLS_LABEL":" << value.total_calls << MARKER
             << SSDB_DBSIZE_LABEL":" << value.dbsize << MARKER
             << SSDB_BINLOGS_LABEL":" << value.binlogs << MARKER;
}

std::ostream& operator<<(std::ostream& out, const ServerInfo& value) {
  return out << value.toString();
}

ServerInfo* makeSsdbServerInfo(const std::string& content) {
  if (content.empty()) {
    return nullptr;
  }

  ServerInfo* result = new ServerInfo;
  static const std::vector<info_field_t> fields = DBTraits<SSDB>::infoFields();
  std::string word;
  DCHECK_EQ(fields.size(), 1);

  for (size_t i = 0; i < content.size(); ++i) {
    word += content[i];
    if (word == fields[0].first) {
      std::string part = content.substr(i + 1);
      result->common_ = ServerInfo::Common(part);
      break;
    }
  }

  return result;
}


std::string ServerInfo::toString() const {
  std::stringstream str;
  str << SSDB_COMMON_LABEL MARKER << common_;
  return str.str();
}

uint32_t ServerInfo::version() const {
  return common::ConvertVersionNumberFromString(common_.version);
}

}  // namespace ssdb
}  // namespace core
}  // namespace fastonosql
