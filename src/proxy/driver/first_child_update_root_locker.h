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

#pragma once

#include "proxy/driver/root_locker.h"  // for RootLocker

class QObject;
namespace fastonosql {
namespace proxy {
class IDriver;
}
}  // namespace fastonosql

namespace fastonosql {
namespace proxy {

class FirstChildUpdateRootLocker : public RootLocker {
 public:
  FirstChildUpdateRootLocker(IDriver* parent,
                             QObject* receiver,
                             const core::command_buffer_t& text,
                             bool silence,
                             const std::vector<core::command_buffer_t>& commands);

 private:
  // notification of execute events
  virtual void ChildrenAdded(core::FastoObjectIPtr child) override;

  core::FastoObjectIPtr FindCmdChildNode(core::FastoObjectIPtr child) const;
  core::FastoObjectIPtr FindWatchedCmd(core::FastoObjectCommand* cmd) const;

  const std::vector<core::command_buffer_t> commands_;
  std::vector<core::FastoObjectIPtr> watched_cmds_;
};

}  // namespace proxy
}  // namespace fastonosql
