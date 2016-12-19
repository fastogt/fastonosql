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

#include <stddef.h>  // for size_t
#include <stdint.h>  // for uint64_t

#include <string>  // for string
#include <vector>  // for vector

#include <common/error.h>   // for Error
#include <common/macros.h>  // for WARN_UNUSED_RESULT

#include "core/internal/cdb_connection.h"

#include "core/db/unqlite/config.h"
#include "proxy/db/unqlite/connection_settings.h"
#include "core/db/unqlite/server_info.h"

struct unqlite;

namespace fastonosql {
namespace core {
namespace unqlite {

typedef struct unqlite NativeConnection;

common::Error CreateConnection(const Config& config, NativeConnection** context);
common::Error CreateConnection(ConnectionSettings* settings, NativeConnection** context);
common::Error TestConnection(ConnectionSettings* settings);

class DBConnection : public core::internal::CDBConnection<NativeConnection, Config, UNQLITE> {
 public:
  typedef core::internal::CDBConnection<NativeConnection, Config, UNQLITE> base_class;
  explicit DBConnection(CDBConnectionClient* client);

  common::Error Info(const char* args, ServerInfo::Stats* statsout) WARN_UNUSED_RESULT;

 private:
  common::Error DelInner(const std::string& key) WARN_UNUSED_RESULT;
  common::Error SetInner(const std::string& key, const std::string& value) WARN_UNUSED_RESULT;
  common::Error GetInner(const std::string& key, std::string* ret_val) WARN_UNUSED_RESULT;

  virtual common::Error ScanImpl(uint64_t cursor_in,
                                 std::string pattern,
                                 uint64_t count_keys,
                                 std::vector<std::string>* keys_out,
                                 uint64_t* cursor_out) override;
  virtual common::Error KeysImpl(const std::string& key_start,
                                 const std::string& key_end,
                                 uint64_t limit,
                                 std::vector<std::string>* ret) override;
  virtual common::Error DBkcountImpl(size_t* size) override;
  virtual common::Error FlushDBImpl() override;
  virtual common::Error SelectImpl(const std::string& name, IDataBaseInfo** info) override;
  virtual common::Error DeleteImpl(const NKeys& keys, NKeys* deleted_keys) override;
  virtual common::Error SetImpl(const NDbKValue& key, NDbKValue* added_key) override;
  virtual common::Error GetImpl(const NKey& key, NDbKValue* loaded_key) override;
  virtual common::Error RenameImpl(const NKey& key, const std::string& new_key) override;
  virtual common::Error SetTTLImpl(const NKey& key, ttl_t ttl) override;
  virtual common::Error GetTTLImpl(const NKey& key, ttl_t* ttl) override;
  virtual common::Error QuitImpl() override;
};

}  // namespace unqlite
}  // namespace core
}  // namespace fastonosql
