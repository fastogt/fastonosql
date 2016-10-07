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

#include "core/driver/root_locker.h"

#include <common/time.h>

#include "core/driver/idriver.h"

namespace fastonosql {
namespace core {

RootLocker::RootLocker(IDriver* parent, QObject* receiver, const std::string& text, bool silence)
    : parent_(parent),
      receiver_(receiver),
      tstart_(common::time::current_mstime()),
      silence_(silence) {
  DCHECK(parent_);

  root_ = FastoObject::createRoot(text, this);
  if (!silence_) {
    events::CommandRootCreatedEvent::value_type res(parent_, root_);
    IDriver::reply(receiver_, new events::CommandRootCreatedEvent(parent_, res));
  }
}

RootLocker::~RootLocker() {
  if (!silence_) {
    events::CommandRootCompleatedEvent::value_type res(parent_, tstart_, root_);
    IDriver::reply(receiver_, new events::CommandRootCompleatedEvent(parent_, res));
  }
}

FastoObjectIPtr RootLocker::root() const {
  return root_;
}

void RootLocker::addedChildren(FastoObjectIPtr child) {
  emit parent_->addedChild(child);
}

void RootLocker::updated(FastoObject* item, FastoObject::value_t val) {
  emit parent_->itemUpdated(item, val);
}

}  // namespace core
}  // namespace fastonosql
