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

#include "global/global.h"

#include "core/types.h"

#define LEVELDB_STATS_LABEL "# Stats"

#define LEVELDB_CAMPACTIONS_LEVEL_LABEL "compactions_level"
#define LEVELDB_FILE_SIZE_MB_LABEL "file_size_mb"
#define LEVELDB_TIME_SEC_LABEL "time_sec"
#define LEVELDB_READ_MB_LABEL "read_mb"
#define LEVELDB_WRITE_MB_LABEL "write_mb"

namespace fastonosql {
namespace core {
namespace leveldb {

class ServerInfo
  : public IServerInfo {
 public:
  // Compactions\nLevel  Files Size(MB) Time(sec) Read(MB) Write(MB)\n
  struct Stats
      : FieldByIndex {
    Stats();
    explicit Stats(const std::string& common_text);
    common::Value* valueByIndex(unsigned char index) const;

    uint32_t compactions_level;
    uint32_t file_size_mb;
    uint32_t time_sec;
    uint32_t read_mb;
    uint32_t write_mb;
  } stats_;

  ServerInfo();
  explicit ServerInfo(const Stats& stats);

  virtual common::Value* valueByIndexes(unsigned char property, unsigned char field) const;
  virtual std::string toString() const;
  virtual uint32_t version() const;
};

std::ostream& operator << (std::ostream& out, const ServerInfo& value);

ServerInfo* makeLeveldbServerInfo(const std::string& content);
ServerInfo* makeLeveldbServerInfo(FastoObject* root);

}  // namespace leveldb
}  // namespace core
}  // namespace fastonosql
