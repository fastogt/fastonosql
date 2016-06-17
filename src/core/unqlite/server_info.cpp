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

#include "core/unqlite/server_info.h"

#include <ostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <string>

#define MARKER "\r\n"

namespace fastonosql {
namespace core {
namespace {

const std::vector<Field> unqliteCommonFields = {
  Field(UNQLITE_FILE_NAME_LABEL, common::Value::TYPE_STRING)
};

}  // namespace

template<>
std::vector<common::Value::Type> DBTraits<UNQLITE>::supportedTypes() {
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
std::vector<info_field_t> DBTraits<UNQLITE>::infoFields() {
  return { std::make_pair(UNQLITE_STATS_LABEL, unqliteCommonFields) };
}

namespace unqlite {

ServerInfo::Stats::Stats()
  : file_name() {
}

ServerInfo::Stats::Stats(const std::string& common_text) {
  size_t pos = 0;
  size_t start = 0;

  while ((pos = common_text.find(MARKER, start)) != std::string::npos) {
    std::string line = common_text.substr(start, pos-start);
    size_t delem = line.find_first_of(':');
    std::string field = line.substr(0, delem);
    std::string value = line.substr(delem + 1);
    if (field == UNQLITE_FILE_NAME_LABEL) {
      file_name = value;
    }
    start = pos + 2;
  }
}

common::Value* ServerInfo::Stats::valueByIndex(unsigned char index) const {
  switch (index) {
  case 0:
    return new common::StringValue(file_name);
  default:
    NOTREACHED();
    break;
  }
  return nullptr;
}

ServerInfo::ServerInfo()
  : IServerInfo(UNQLITE) {
}

ServerInfo::ServerInfo(const Stats &stats)
  : IServerInfo(UNQLITE), stats_(stats) {
}

common::Value* ServerInfo::valueByIndexes(unsigned char property,
                                                 unsigned char field) const {
  switch (property) {
  case 0:
    return stats_.valueByIndex(field);
  default:
    NOTREACHED();
    break;
  }
  return nullptr;
}

std::ostream& operator<<(std::ostream& out, const ServerInfo::Stats& value) {
  return out << UNQLITE_FILE_NAME_LABEL":" << value.file_name << MARKER;
}

std::ostream& operator<<(std::ostream& out, const ServerInfo& value) {
  return out << value.toString();
}

ServerInfo* makeUnqliteServerInfo(const std::string& content) {
  if (content.empty()) {
    return nullptr;
  }

  ServerInfo* result = new ServerInfo;

  static const std::vector<info_field_t> fields = DBTraits<UNQLITE>::infoFields();
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


std::string ServerInfo::toString() const {
  std::stringstream str;
  str << UNQLITE_STATS_LABEL MARKER << stats_;
  return str.str();
}

uint32_t ServerInfo::version() const {
  return 0;
}

ServerInfo* makeUnqliteServerInfo(FastoObject* root) {
  std::string content = common::ConvertToString(root);
  return makeUnqliteServerInfo(content);
}

}  // namespace unqlite
}  // namespace core
}  // namespace fastonosql
