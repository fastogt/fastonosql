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

#include "core/db/leveldb/server_info.h"

#include <ostream>  // for operator<<, basic_ostream, etc
#include <sstream>
#include <stddef.h>  // for size_t
#include <string>    // for string, operator==, etc
#include <utility>   // for make_pair
#include <vector>    // for vector

#include <common/convert2string.h>  // for ConvertFromString
#include <common/macros.h>          // for NOTREACHED, DCHECK_EQ
#include <common/value.h>           // for Value, FundamentalValue, etc

#include "core/connection_types.h"  // for connectionTypes::LEVELDB
#include "core/db_traits.h"

#define MARKER "\r\n"

namespace fastonosql {
namespace core {

namespace {

const std::vector<Field> LeveldbCommonFields = {
    Field(LEVELDB_CAMPACTIONS_LEVEL_LABEL, common::Value::TYPE_UINTEGER),
    Field(LEVELDB_FILE_SIZE_MB_LABEL, common::Value::TYPE_UINTEGER),
    Field(LEVELDB_TIME_SEC_LABEL, common::Value::TYPE_UINTEGER),
    Field(LEVELDB_READ_MB_LABEL, common::Value::TYPE_UINTEGER),
    Field(LEVELDB_WRITE_MB_LABEL, common::Value::TYPE_UINTEGER)};

}  // namespace

template <>
std::vector<common::Value::Type> DBTraits<LEVELDB>::SupportedTypes() {
  return {common::Value::TYPE_BOOLEAN, common::Value::TYPE_INTEGER, common::Value::TYPE_UINTEGER,
          common::Value::TYPE_DOUBLE, common::Value::TYPE_STRING};
}

template <>
std::vector<info_field_t> DBTraits<LEVELDB>::InfoFields() {
  return {std::make_pair(LEVELDB_STATS_LABEL, LeveldbCommonFields)};
}

namespace leveldb {

ServerInfo::Stats::Stats()
    : compactions_level(0), file_size_mb(0), time_sec(0), read_mb(0), write_mb(0) {}

ServerInfo::Stats::Stats(const std::string& common_text) {
  size_t pos = 0;
  size_t start = 0;

  while ((pos = common_text.find(MARKER, start)) != std::string::npos) {
    std::string line = common_text.substr(start, pos - start);
    size_t delem = line.find_first_of(':');
    std::string field = line.substr(0, delem);
    std::string value = line.substr(delem + 1);
    if (field == LEVELDB_CAMPACTIONS_LEVEL_LABEL) {
      compactions_level = common::ConvertFromString<uint32_t>(value);
    } else if (field == LEVELDB_FILE_SIZE_MB_LABEL) {
      file_size_mb = common::ConvertFromString<uint32_t>(value);
    } else if (field == LEVELDB_TIME_SEC_LABEL) {
      time_sec = common::ConvertFromString<uint32_t>(value);
    } else if (field == LEVELDB_READ_MB_LABEL) {
      read_mb = common::ConvertFromString<uint32_t>(value);
    } else if (field == LEVELDB_WRITE_MB_LABEL) {
      write_mb = common::ConvertFromString<uint32_t>(value);
    }
    start = pos + 2;
  }
}

common::Value* ServerInfo::Stats::ValueByIndex(unsigned char index) const {
  switch (index) {
    case 0:
      return new common::FundamentalValue(compactions_level);
    case 1:
      return new common::FundamentalValue(file_size_mb);
    case 2:
      return new common::FundamentalValue(time_sec);
    case 3:
      return new common::FundamentalValue(read_mb);
    case 4:
      return new common::FundamentalValue(write_mb);
    default:
      break;
  }

  NOTREACHED();
  return nullptr;
}

ServerInfo::ServerInfo() : IServerInfo(LEVELDB) {}

ServerInfo::ServerInfo(const Stats& stats) : IServerInfo(LEVELDB), stats_(stats) {}

common::Value* ServerInfo::ValueByIndexes(unsigned char property, unsigned char field) const {
  switch (property) {
    case 0:
      return stats_.ValueByIndex(field);
    default:
      break;
  }

  NOTREACHED();
  return nullptr;
}

std::ostream& operator<<(std::ostream& out, const ServerInfo::Stats& value) {
  return out << LEVELDB_CAMPACTIONS_LEVEL_LABEL ":" << value.compactions_level << MARKER
             << LEVELDB_FILE_SIZE_MB_LABEL ":" << value.file_size_mb << MARKER
             << LEVELDB_TIME_SEC_LABEL ":" << value.time_sec << MARKER << LEVELDB_READ_MB_LABEL ":"
             << value.read_mb << MARKER << LEVELDB_WRITE_MB_LABEL ":" << value.write_mb << MARKER;
}

std::ostream& operator<<(std::ostream& out, const ServerInfo& value) {
  return out << value.ToString();
}

ServerInfo* MakeLeveldbServerInfo(const std::string& content) {
  if (content.empty()) {
    return nullptr;
  }

  ServerInfo* result = new ServerInfo;
  static const std::vector<info_field_t> fields = DBTraits<LEVELDB>::InfoFields();
  std::string word;
  DCHECK_EQ(fields.size(), 1);

  for (size_t i = 0; i < content.size(); ++i) {
    word += content[i];
    if (word == fields[0].first) {
      std::string part = content.substr(i + 1);
      result->stats_ = ServerInfo::Stats(part);
      break;
    }
  }

  return result;
}

std::string ServerInfo::ToString() const {
  std::stringstream str;
  str << LEVELDB_STATS_LABEL MARKER << stats_;
  return str.str();
}

uint32_t ServerInfo::Version() const {
  return 0;
}

}  // namespace leveldb
}  // namespace core
}  // namespace fastonosql
