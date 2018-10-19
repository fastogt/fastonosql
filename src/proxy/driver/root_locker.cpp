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

#include "proxy/driver/root_locker.h"

#include <QObject>

#include <common/time.h>  // for current_mstime

#include "proxy/driver/idriver.h"  // for IDriver
#include "proxy/events/events.h"   // for CommandRootCompleatedEvent, etc

namespace fastonosql {
namespace proxy {

RootLocker::RootLocker(IDriver* parent, QObject* receiver, const core::command_buffer_t& text, bool silence)
    : base_class(), parent_(parent), receiver_(receiver), tstart_(common::time::current_mstime()), silence_(silence) {
  CHECK(parent_);

  root_ = core::FastoObject::CreateRoot(text, this);
  if (!silence_) {
    events::CommandRootCreatedEvent::value_type res(parent_, root_);
    IDriver::Reply(receiver_, new events::CommandRootCreatedEvent(parent_, res));
  }
}

RootLocker::~RootLocker() {
  if (!silence_) {
    events::CommandRootCompleatedEvent::value_type res(parent_, tstart_, root_);
    IDriver::Reply(receiver_, new events::CommandRootCompleatedEvent(parent_, res));
  }
}

core::FastoObjectIPtr RootLocker::Root() const {
  return root_;
}

void RootLocker::ChildrenAdded(core::FastoObjectIPtr child) {
  emit parent_->ChildAdded(child);
}

void RootLocker::Updated(core::FastoObject* item, core::FastoObject::value_t val) {
  emit parent_->ItemUpdated(item, val);
}

}  // namespace proxy
}  // namespace fastonosql
