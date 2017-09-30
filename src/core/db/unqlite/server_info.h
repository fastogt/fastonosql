/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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

#include "core/server/iserver_info.h"

#define UNQLITE_STATS_LABEL "# Stats"

#define UNQLITE_DB_PATH_LABEL "file_name"

namespace fastonosql {
namespace core {
namespace unqlite {

class ServerInfo : public IServerInfo {
 public:
  // Compactions\nLevel  Files Size(MB) Time(sec) Read(MB)
  // Write(MB)\n
  struct Stats : IStateField {
    Stats();
    explicit Stats(const std::string& common_text);
    common::Value* GetValueByIndex(unsigned char index) const override;
    std::string db_path;
  } stats_;

  ServerInfo();
  explicit ServerInfo(const Stats& stats);

  virtual common::Value* GetValueByIndexes(unsigned char property, unsigned char field) const override;
  virtual std::string ToString() const override;
  virtual uint32_t GetVersion() const override;
};

std::ostream& operator<<(std::ostream& out, const ServerInfo& value);

ServerInfo* MakeUnqliteServerInfo(const std::string& content);

}  // namespace unqlite
}  // namespace core
}  // namespace fastonosql
