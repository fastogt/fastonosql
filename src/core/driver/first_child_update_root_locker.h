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

#include <string>  // for string
#include <vector>  // for vector

#include "core/driver/root_locker.h"  // for RootLocker

#include "global/global.h"  // for FastoObjectIPtr, etc

class QObject;
namespace fastonosql {
namespace core {
class IDriver;
}
}

namespace fastonosql {
namespace core {

class FirstChildUpdateRootLocker : public RootLocker {
 public:
  FirstChildUpdateRootLocker(IDriver* parent,
                             QObject* receiver,
                             const std::string& text,
                             bool silence,
                             const std::vector<std::string>& commands);

 private:
  // notification of execute events
  virtual void ChildrenAdded(FastoObjectIPtr child) override;

  FastoObjectIPtr findCmdChildNode(FastoObjectIPtr child) const;
  FastoObjectIPtr findWatchdCmd(FastoObjectCommand* cmd) const;

  const std::vector<std::string> commands_;
  std::vector<FastoObjectIPtr> watched_cmds_;
};

}  // namespace core
}  // namespace fastonosql
