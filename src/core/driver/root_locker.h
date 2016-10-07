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

#include <string>           // for string

#include <common/types.h>   // for time64_t

#include "global/global.h"  // for FastoObjectIPtr, etc

class QObject;
namespace fastonosql {
namespace core {
class IDriver;
}
}  // lines 32-32

namespace fastonosql {
namespace core {

class RootLocker : FastoObject::IFastoObjectObserver {
 public:
  RootLocker(IDriver* parent, QObject* receiver, const std::string& text, bool silence);
  ~RootLocker();

  FastoObjectIPtr root() const;

 private:
  // notification of execute events
  virtual void addedChildren(FastoObjectIPtr child) override;
  virtual void updated(FastoObject* item, FastoObject::value_t val) override;

  FastoObjectIPtr root_;
  IDriver* parent_;
  QObject* receiver_;
  const common::time64_t tstart_;
  const bool silence_;
};

}  // namespace core
}  // namespace fastonosql
