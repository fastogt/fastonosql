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

#include "core/driver/first_child_update_root_locker.h"

#include <common/intrusive_ptr.h>       // for intrusive_ptr, operator==
#include <common/macros.h>              // for NOTREACHED
#include <common/value.h>               // for Value, etc

namespace fastonosql {
namespace core {

FirstChildUpdateRootLocker::FirstChildUpdateRootLocker(IDriver* parent,
                                                       QObject* receiver,
                                                       const std::string& text,
                                                       bool silence,
                                                       const std::vector<std::string>& commands)
    : RootLocker(parent, receiver, text, silence), commands_(commands), watched_cmds_() {}

void FirstChildUpdateRootLocker::ChildrenAdded(FastoObjectIPtr child) {
  auto val = child->Value();
  if (child->Type() == common::Value::TYPE_COMMAND) {
    if (watched_cmds_.size() == commands_.size()) {
      return;
    }

    watched_cmds_.push_back(child);
    RootLocker::ChildrenAdded(child);
    return;
  }

  FastoObjectIPtr watched_child = FindCmdChildNode(child);
  if (!watched_child) {
    return;
  }

  if (watched_child == child) {
    RootLocker::ChildrenAdded(child);
    return;
  }

  watched_child->SetValue(val);
}

FastoObjectIPtr FirstChildUpdateRootLocker::FindCmdChildNode(FastoObjectIPtr child) const {
  FastoObject* parent = child->Parent();
  if (parent->Type() != common::Value::TYPE_COMMAND) {
    NOTREACHED();
    return FastoObjectIPtr();
  }

  FastoObjectCommand* cmd = dynamic_cast<FastoObjectCommand*>(parent);
  FastoObjectIPtr watched_cmd = FindWatchedCmd(cmd);
  if (!watched_cmd) {
    NOTREACHED();
    return FastoObjectIPtr();
  }

  auto childs = watched_cmd->Childrens();
  if (childs.empty()) {
    NOTREACHED();
    return FastoObjectIPtr();
  }

  return childs[0];
}

FastoObjectIPtr FirstChildUpdateRootLocker::FindWatchedCmd(FastoObjectCommand* cmd) const {
  for (auto child_cmd : watched_cmds_) {
    FastoObjectCommand* wcmd = dynamic_cast<FastoObjectCommand*>(child_cmd.get());
    if (wcmd->InputCommand() == cmd->InputCommand()) {
      return wcmd;
    }
  }

  return FastoObjectIPtr();
}

}  // namespace core
}  // namespace fastonosql
