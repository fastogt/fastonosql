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

#include "proxy/driver/first_child_update_root_locker.h"

#include <common/intrusive_ptr.h>  // for intrusive_ptr, operator==
#include <common/macros.h>         // for NOTREACHED
#include <common/value.h>          // for Value, etc

namespace fastonosql {
namespace proxy {

FirstChildUpdateRootLocker::FirstChildUpdateRootLocker(IDriver* parent,
                                                       QObject* receiver,
                                                       const core::command_buffer_t& text,
                                                       bool silence,
                                                       const std::vector<core::command_buffer_t>& commands)
    : RootLocker(parent, receiver, text, silence), commands_(commands), watched_cmds_() {}

void FirstChildUpdateRootLocker::ChildrenAdded(core::FastoObjectIPtr child) {
  auto val = child->Value();
  core::FastoObjectCommand* cmd = dynamic_cast<core::FastoObjectCommand*>(child.get());
  if (cmd) {
    if (watched_cmds_.size() == commands_.size()) {
      return;
    }

    watched_cmds_.push_back(child);
    RootLocker::ChildrenAdded(child);
    return;
  }

  core::FastoObjectIPtr watched_child = FindCmdChildNode(child);
  if (!watched_child) {
    return;
  }

  if (watched_child == child) {
    RootLocker::ChildrenAdded(child);
    return;
  }

  watched_child->SetValue(val);
}

core::FastoObjectIPtr FirstChildUpdateRootLocker::FindCmdChildNode(core::FastoObjectIPtr child) const {
  core::FastoObject* parent = child->Parent();
  core::FastoObjectCommand* cmd = dynamic_cast<core::FastoObjectCommand*>(parent);
  if (!cmd) {
    NOTREACHED();
    return core::FastoObjectIPtr();
  }

  core::FastoObjectIPtr watched_cmd = FindWatchedCmd(cmd);
  if (!watched_cmd) {
    NOTREACHED();
    return core::FastoObjectIPtr();
  }

  auto childs = watched_cmd->Childrens();
  if (childs.empty()) {
    NOTREACHED();
    return core::FastoObjectIPtr();
  }

  return childs[0];
}

core::FastoObjectIPtr FirstChildUpdateRootLocker::FindWatchedCmd(core::FastoObjectCommand* cmd) const {
  for (auto child_cmd : watched_cmds_) {
    core::FastoObjectCommand* wcmd = dynamic_cast<core::FastoObjectCommand*>(child_cmd.get());
    if (wcmd->InputCommand() == cmd->InputCommand()) {
      return wcmd;
    }
  }

  return core::FastoObjectIPtr();
}

}  // namespace proxy
}  // namespace fastonosql
