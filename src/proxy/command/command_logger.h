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

#include <QObject>

#include <common/patterns/singleton_pattern.h>  // for LazySingleton

#include "global/global.h"

namespace fastonosql {
namespace proxy {

class CommandLogger : public QObject, public common::patterns::LazySingleton<CommandLogger> {
  friend class common::patterns::LazySingleton<CommandLogger>;
  Q_OBJECT
 public:
  void Print(FastoObjectCommandIPtr command);

 Q_SIGNALS:
  void Printed(FastoObjectCommandIPtr mess);

 private:
  CommandLogger();
};

inline void LOG_COMMAND(FastoObjectCommandIPtr command) {
  return CommandLogger::instance().Print(command);
}

}  // namespace proxy
}  // namespace fastonosql
