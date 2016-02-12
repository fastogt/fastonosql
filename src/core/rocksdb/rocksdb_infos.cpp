/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    SiteOnYourDevice is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SiteOnYourDevice is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SiteOnYourDevice.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "core/rocksdb/rocksdb_infos.h"

#include <ostream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>

#define MARKER "\r\n"

namespace fastonosql {
namespace {

const std::vector<Field> rockCommonFields = {
    Field(ROCKSDB_CAMPACTIONS_LEVEL_LABEL, common::Value::TYPE_UINTEGER),
    Field(ROCKSDB_FILE_SIZE_MB_LABEL, common::Value::TYPE_UINTEGER),
    Field(ROCKSDB_TIME_SEC_LABEL, common::Value::TYPE_UINTEGER),
    Field(ROCKSDB_READ_MB_LABEL, common::Value::TYPE_UINTEGER),
    Field(ROCKSDB_WRITE_MB_LABEL, common::Value::TYPE_UINTEGER)
};

}  // namespace

template<>
std::vector<common::Value::Type> DBTraits<ROCKSDB>::supportedTypes() {
  return  {
              common::Value::TYPE_BOOLEAN,
              common::Value::TYPE_INTEGER,
              common::Value::TYPE_UINTEGER,
              common::Value::TYPE_DOUBLE,
              common::Value::TYPE_STRING,
              common::Value::TYPE_ARRAY
          };
}

template<>
std::vector<std::string> DBTraits<ROCKSDB>::infoHeaders() {
  return { ROCKSDB_STATS_LABEL };
}

template<>
std::vector<std::vector<Field> > DBTraits<ROCKSDB>::infoFields() {
  return  { rockCommonFields };
}

RocksdbServerInfo::Stats::Stats()
  : compactions_level(0), file_size_mb(0), time_sec(0), read_mb(0), write_mb(0) {
}

RocksdbServerInfo::Stats::Stats(const std::string& common_text) {
  const std::string &src = common_text;
  size_t pos = 0;
  size_t start = 0;

  while ((pos = src.find(MARKER, start)) != std::string::npos) {
    std::string line = src.substr(start, pos-start);
    size_t delem = line.find_first_of(':');
    std::string field = line.substr(0, delem);
    std::string value = line.substr(delem + 1);
    if (field == ROCKSDB_CAMPACTIONS_LEVEL_LABEL) {
        compactions_level = common::convertFromString<uint32_t>(value);
    } else if (field == ROCKSDB_FILE_SIZE_MB_LABEL) {
        file_size_mb = common::convertFromString<uint32_t>(value);
    } else if (field == ROCKSDB_TIME_SEC_LABEL) {
        time_sec = common::convertFromString<uint32_t>(value);
    } else if (field == ROCKSDB_READ_MB_LABEL) {
        read_mb = common::convertFromString<uint32_t>(value);
    } else if (field == ROCKSDB_WRITE_MB_LABEL) {
        write_mb = common::convertFromString<uint32_t>(value);
    }
    start = pos + 2;
  }
}

common::Value* RocksdbServerInfo::Stats::valueByIndex(unsigned char index) const {
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
      NOTREACHED();
      break;
  }
  return NULL;
}

RocksdbServerInfo::RocksdbServerInfo()
  : ServerInfo(ROCKSDB) {
}

RocksdbServerInfo::RocksdbServerInfo(const Stats &stats)
  : ServerInfo(ROCKSDB), stats_(stats) {
}

common::Value* RocksdbServerInfo::valueByIndexes(unsigned char property,
                                                 unsigned char field) const {
  switch (property) {
    case 0:
      return stats_.valueByIndex(field);
    default:
      NOTREACHED();
      break;
  }
  return NULL;
}

std::ostream& operator<<(std::ostream& out, const RocksdbServerInfo::Stats& value) {
  return out << ROCKSDB_CAMPACTIONS_LEVEL_LABEL":" << value.compactions_level << MARKER
              << ROCKSDB_FILE_SIZE_MB_LABEL":" << value.file_size_mb << MARKER
              << ROCKSDB_TIME_SEC_LABEL":" << value.time_sec << MARKER
              << ROCKSDB_READ_MB_LABEL":" << value.read_mb << MARKER
              << ROCKSDB_WRITE_MB_LABEL":" << value.write_mb << MARKER;
}

std::ostream& operator<<(std::ostream& out, const RocksdbServerInfo& value) {
  return out << value.toString();
}

RocksdbServerInfo* makeRocksdbServerInfo(const std::string &content) {
  if (content.empty()) {
    return NULL;
  }

  RocksdbServerInfo* result = new RocksdbServerInfo;

  const std::vector<std::string> headers = DBTraits<ROCKSDB>::infoHeaders();
  std::string word;
  DCHECK_EQ(headers.size(), 1);

  for (size_t i = 0; i < content.size(); ++i) {
      word += content[i];
      if (word == headers[0]) {
          std::string part = content.substr(i + 1);
          result->stats_ = RocksdbServerInfo::Stats(part);
          break;
      }
  }

  return result;
}


std::string RocksdbServerInfo::toString() const {
  std::stringstream str;
  str << ROCKSDB_STATS_LABEL MARKER << stats_;
  return str.str();
}

uint32_t RocksdbServerInfo::version() const {
  return 0;
}

RocksdbServerInfo* makeRocksdbServerInfo(FastoObject* root) {
  const std::string content = common::convertToString(root);
  return makeRocksdbServerInfo(content);
}

RocksdbDataBaseInfo::RocksdbDataBaseInfo(const std::string& name, bool isDefault, size_t size,
                                         const keys_cont_type &keys)
  : IDataBaseInfo(name, isDefault, ROCKSDB, size, keys) {
}

IDataBaseInfo* RocksdbDataBaseInfo::clone() const {
  return new RocksdbDataBaseInfo(*this);
}

RocksdbCommand::RocksdbCommand(FastoObject* parent, common::CommandValue* cmd,
                               const std::string &delemitr)
  : FastoObjectCommand(parent, cmd, delemitr) {
}

bool RocksdbCommand::isReadOnly() const {
  std::string key = inputCmd();
  if (key.empty()) {
    return true;
  }

  std::transform(key.begin(), key.end(), key.begin(), ::tolower);
  return key != "get";
}

}  // namespace fastonosql
