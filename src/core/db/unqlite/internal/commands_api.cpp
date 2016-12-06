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

#include "core/db/unqlite/internal/commands_api.h"

#include <memory>  // for __shared_ptr

#include <common/value.h>  // for ErrorValue, StringValue, etc

#include "core/db/unqlite/db_connection.h"  // for DBConnection
#include "core/db/unqlite/server_info.h"    // for ServerInfo, etc

#include "global/global.h"  // for FastoObject

namespace fastonosql {
namespace core {
namespace unqlite {

common::Error CommandsApi::Info(internal::CommandHandler* handler,
                                int argc,
                                const char** argv,
                                FastoObject* out) {
  DBConnection* unq = static_cast<DBConnection*>(handler);
  ServerInfo::Stats statsout;
  common::Error err = unq->Info(argc == 1 ? argv[0] : nullptr, &statsout);
  if (err && err->isError()) {
    return err;
  }

  ServerInfo uinf(statsout);
  common::StringValue* val = common::Value::createStringValue(uinf.ToString());
  FastoObject* child = new FastoObject(out, val, unq->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

}  // namespace unqlite
}  // namespace core
}  // namespace fastonosql
