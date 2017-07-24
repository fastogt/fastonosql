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

#include <common/logger.h>
#include <common/patterns/singleton_pattern.h>  // for LazySingleton

namespace fastonosql {
namespace core {

typedef void LogWatcher(common::logging::LEVEL_LOG level, const std::string& message, bool notify);

class Logger : public common::patterns::LazySingleton<Logger> {
  friend class common::patterns::LazySingleton<Logger>;

 public:
  void print(const char* mess, common::logging::LEVEL_LOG level, bool notify);
  void print(const std::string& mess, common::logging::LEVEL_LOG level, bool notify);

 private:
  Logger();
};

}  // namespace core
}  // namespace fastonosql

void SET_LOG_WATCHER(fastonosql::core::LogWatcher* watcher);

template <typename T>
inline void LOG_CORE_MSG(T mess, common::logging::LEVEL_LOG level, bool notify) {
  return fastonosql::core::Logger::Instance().print(mess, level, notify);
}
