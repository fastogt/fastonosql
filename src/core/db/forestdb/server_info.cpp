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

#include "core/db/forestdb/server_info.h"

#include <common/convert2string.h>

#include "core/db_traits.h"
#include "core/value.h"

#define MARKER "\r\n"

namespace fastonosql {
namespace core {
namespace {

const std::vector<Field> forestdb_common_fields = {
    Field(FORESTDB_STATS_DB_FILE_PATH_LABEL, common::Value::TYPE_STRING),
    Field(FORESTDB_STATS_DB_FILE_SIZE_LABEL, common::Value::TYPE_ULONG_INTEGER)};

}  // namespace

template <>
std::vector<common::Value::Type> DBTraits<FORESTDB>::GetSupportedValueTypes() {
  return {common::Value::TYPE_BOOLEAN, common::Value::TYPE_INTEGER, common::Value::TYPE_UINTEGER,
          common::Value::TYPE_DOUBLE,  common::Value::TYPE_STRING,  JsonValue::TYPE_JSON};
}

template <>
std::vector<info_field_t> DBTraits<FORESTDB>::GetInfoFields() {
  return {std::make_pair(FORESTDB_STATS_LABEL, forestdb_common_fields)};
}
namespace forestdb {

ServerInfo::Stats::Stats() : db_path(), db_size() {}

ServerInfo::Stats::Stats(const std::string& common_text) {
  size_t pos = 0;
  size_t start = 0;

  while ((pos = common_text.find(MARKER, start)) != std::string::npos) {
    std::string line = common_text.substr(start, pos - start);
    size_t delem = line.find_first_of(':');
    std::string field = line.substr(0, delem);
    std::string value = line.substr(delem + 1);
    if (field == FORESTDB_STATS_DB_FILE_PATH_LABEL) {
      db_path = value;
    } else if (field == FORESTDB_STATS_DB_FILE_SIZE_LABEL) {
      off_t sz;
      if (common::ConvertFromString(value, &sz)) {
        db_size = sz;
      }
    }
    start = pos + 2;
  }
}

common::Value* ServerInfo::Stats::GetValueByIndex(unsigned char index) const {
  switch (index) {
    case 0:
      return new common::StringValue(db_path);
    case 1:
      return new common::FundamentalValue(db_size);
    default:
      break;
  }

  NOTREACHED();
  return nullptr;
}

ServerInfo::ServerInfo() : IServerInfo(FORESTDB) {}

ServerInfo::ServerInfo(const Stats& stats) : IServerInfo(FORESTDB), stats_(stats) {}

common::Value* ServerInfo::GetValueByIndexes(unsigned char property, unsigned char field) const {
  switch (property) {
    case 0:
      return stats_.GetValueByIndex(field);
    case 1:
      return stats_.GetValueByIndex(field);
    default:
      break;
  }

  NOTREACHED();
  return nullptr;
}

std::ostream& operator<<(std::ostream& out, const ServerInfo::Stats& value) {
  return out << FORESTDB_STATS_DB_FILE_PATH_LABEL ":" << value.db_path << MARKER
             << FORESTDB_STATS_DB_FILE_SIZE_LABEL ":" << value.db_size << MARKER;
}

std::ostream& operator<<(std::ostream& out, const ServerInfo& value) {
  return out << value.ToString();
}

ServerInfo* MakeForestDBServerInfo(const std::string& content) {
  if (content.empty()) {
    return nullptr;
  }

  ServerInfo* result = new ServerInfo;
  static const std::vector<info_field_t> fields = DBTraits<FORESTDB>::GetInfoFields();
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
  str << FORESTDB_STATS_LABEL MARKER << stats_;
  return str.str();
}

uint32_t ServerInfo::GetVersion() const {
  return 0;
}

}  // namespace forestdb
}  // namespace core
}  // namespace fastonosql
