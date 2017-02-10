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

#include "core/logger.h"

#include <common/logger.h>
namespace {
fastonosql::core::LogWatcher* g_watcher = nullptr;
}

namespace fastonosql {
namespace core {

Logger::Logger() {}

void Logger::print(const char* mess, common::logging::LEVEL_LOG level, bool notify) {
  print(std::string(mess), level, notify);
}

void Logger::print(const std::string& mess, common::logging::LEVEL_LOG level, bool notify) {
  RUNTIME_LOG(level) << mess;
  if (g_watcher) {
    g_watcher(level, mess, notify);
  }
}

}  // namespace proxy
}  // namespace fastonosql

void SET_LOG_WATCHER(fastonosql::core::LogWatcher* watcher) {
  g_watcher = watcher;
}
