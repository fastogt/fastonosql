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

#include "core/db/rocksdb/server_info.h"

#include <stddef.h>  // for size_t

#include <sstream>  // for operator<<, basic_ostream, etc
#include <string>   // for string, operator==, etc
#include <utility>  // for make_pair
#include <vector>   // for vector

#include <common/convert2string.h>  // for ConvertFromString
#include <common/macros.h>          // for NOTREACHED, DCHECK_EQ
#include <common/value.h>           // for Value, FundamentalValue, etc

#include "core/connection_types.h"  // for connectionTypes::ROCKSDB
#include "core/db_traits.h"

#define MARKER "\r\n"

namespace fastonosql {
namespace core {
namespace {

const std::vector<Field> rockCommonFields = {Field(ROCKSDB_CAMPACTIONS_LEVEL_LABEL, common::Value::TYPE_UINTEGER),
                                             Field(ROCKSDB_FILE_SIZE_MB_LABEL, common::Value::TYPE_UINTEGER),
                                             Field(ROCKSDB_TIME_SEC_LABEL, common::Value::TYPE_UINTEGER),
                                             Field(ROCKSDB_READ_MB_LABEL, common::Value::TYPE_UINTEGER),
                                             Field(ROCKSDB_WRITE_MB_LABEL, common::Value::TYPE_UINTEGER)};

}  // namespace

template <>
std::vector<common::Value::Type> DBTraits<ROCKSDB>::SupportedTypes() {
  return {common::Value::TYPE_BOOLEAN, common::Value::TYPE_INTEGER, common::Value::TYPE_UINTEGER,
          common::Value::TYPE_DOUBLE, common::Value::TYPE_STRING};
}

template <>
std::vector<info_field_t> DBTraits<ROCKSDB>::InfoFields() {
  return {std::make_pair(ROCKSDB_STATS_LABEL, rockCommonFields)};
}

namespace rocksdb {

ServerInfo::Stats::Stats() : compactions_level(0), file_size_mb(0), time_sec(0), read_mb(0), write_mb(0) {}

ServerInfo::Stats::Stats(const std::string& common_text) {
  size_t pos = 0;
  size_t start = 0;

  while ((pos = common_text.find(MARKER, start)) != std::string::npos) {
    std::string line = common_text.substr(start, pos - start);
    size_t delem = line.find_first_of(':');
    std::string field = line.substr(0, delem);
    std::string value = line.substr(delem + 1);
    if (field == ROCKSDB_CAMPACTIONS_LEVEL_LABEL) {
      uint32_t lcompactions_level;
      if (common::ConvertFromString(value, &lcompactions_level)) {
        compactions_level = lcompactions_level;
      }
    } else if (field == ROCKSDB_FILE_SIZE_MB_LABEL) {
      uint32_t lfile_size_mb;
      if (common::ConvertFromString(value, &lfile_size_mb)) {
        file_size_mb = lfile_size_mb;
      }
    } else if (field == ROCKSDB_TIME_SEC_LABEL) {
      uint32_t ltime_sec;
      if (common::ConvertFromString(value, &ltime_sec)) {
        time_sec = ltime_sec;
      }
    } else if (field == ROCKSDB_READ_MB_LABEL) {
      uint32_t lread_mb;
      if (common::ConvertFromString(value, &lread_mb)) {
        read_mb = lread_mb;
      }
    } else if (field == ROCKSDB_WRITE_MB_LABEL) {
      uint32_t lwrite_mb;
      if (common::ConvertFromString(value, &lwrite_mb)) {
        write_mb = lwrite_mb;
      }
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

ServerInfo::ServerInfo() : IServerInfo(ROCKSDB) {}

ServerInfo::ServerInfo(const Stats& stats) : IServerInfo(ROCKSDB), stats_(stats) {}

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
  return out << ROCKSDB_CAMPACTIONS_LEVEL_LABEL ":" << value.compactions_level << MARKER
             << ROCKSDB_FILE_SIZE_MB_LABEL ":" << value.file_size_mb << MARKER << ROCKSDB_TIME_SEC_LABEL ":"
             << value.time_sec << MARKER << ROCKSDB_READ_MB_LABEL ":" << value.read_mb << MARKER
             << ROCKSDB_WRITE_MB_LABEL ":" << value.write_mb << MARKER;
}

std::ostream& operator<<(std::ostream& out, const ServerInfo& value) {
  return out << value.ToString();
}

ServerInfo* MakeRocksdbServerInfo(const std::string& content) {
  if (content.empty()) {
    return nullptr;
  }

  ServerInfo* result = new ServerInfo;
  static const std::vector<info_field_t> fields = DBTraits<ROCKSDB>::InfoFields();
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
  str << ROCKSDB_STATS_LABEL MARKER << stats_;
  return str.str();
}

uint32_t ServerInfo::Version() const {
  return 0;
}

}  // namespace rocksdb
}  // namespace core
}  // namespace fastonosql
