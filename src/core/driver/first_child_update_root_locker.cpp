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

namespace fastonosql {
namespace core {

FirstChildUpdateRootLocker::FirstChildUpdateRootLocker(IDriver* parent,
                                                       QObject* receiver,
                                                       const std::string& text,
                                                       bool silence,
                                                       const std::vector<std::string>& commands)
    : RootLocker(parent, receiver, text, silence), commands_(commands), watched_cmds_() {}

void FirstChildUpdateRootLocker::addedChildren(FastoObjectIPtr child) {
  auto val = child->value();
  if (child->type() == common::Value::TYPE_COMMAND) {
    if (watched_cmds_.size() == commands_.size()) {
      return;
    }

    watched_cmds_.push_back(child);
    RootLocker::addedChildren(child);
    return;
  }

  FastoObjectIPtr watched_child = findCmdChildNode(child);
  if (!watched_child) {
    return;
  }

  if (watched_child == child) {
    RootLocker::addedChildren(child);
    return;
  }

  watched_child->setValue(val);
}

FastoObjectIPtr FirstChildUpdateRootLocker::findCmdChildNode(FastoObjectIPtr child) const {
  FastoObject* parent = child->parent();
  if (parent->type() != common::Value::TYPE_COMMAND) {
    NOTREACHED();
    return FastoObjectIPtr();
  }

  FastoObjectCommand* cmd = dynamic_cast<FastoObjectCommand*>(parent);
  FastoObjectIPtr watched_cmd = findWatchdCmd(cmd);
  if (!watched_cmd) {
    NOTREACHED();
    return FastoObjectIPtr();
  }

  auto childs = watched_cmd->childrens();
  if (childs.empty()) {
    NOTREACHED();
    return FastoObjectIPtr();
  }

  return childs[0];
}

FastoObjectIPtr FirstChildUpdateRootLocker::findWatchdCmd(FastoObjectCommand* cmd) const {
  for (auto child_cmd : watched_cmds_) {
    FastoObjectCommand* wcmd = dynamic_cast<FastoObjectCommand*>(child_cmd.get());
    if (wcmd->inputCommand() == cmd->inputCommand()) {
      return wcmd;
    }
  }

  return FastoObjectIPtr();
}

}  // namespace core
}  // namespace fastonosql
