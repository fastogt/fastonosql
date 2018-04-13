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

#include "core/db/redis_compatible/db_connection.h"

#include "core/db/redis/config.h"

namespace fastonosql {
namespace core {
namespace redis {

typedef redis_compatible::NativeConnection NativeConnection;

common::Error CreateConnection(const RConfig& config, NativeConnection** context);
common::Error TestConnection(const RConfig& config);
common::Error DiscoveryClusterConnection(const RConfig& config, std::vector<ServerDiscoveryClusterInfoSPtr>* infos);
common::Error DiscoverySentinelConnection(const RConfig& config, std::vector<ServerDiscoverySentinelInfoSPtr>* infos);

class DBConnection : public redis_compatible::DBConnection<RConfig, REDIS> {
 public:
  typedef redis_compatible::DBConnection<RConfig, REDIS> base_class;
  explicit DBConnection(CDBConnectionClient* client);

  common::Error GraphQuery(const commands_args_t& argv, FastoObject* out) WARN_UNUSED_RESULT;
  common::Error GraphExplain(const commands_args_t& argv, FastoObject* out) WARN_UNUSED_RESULT;
  common::Error GraphDelete(const commands_args_t& argv, FastoObject* out) WARN_UNUSED_RESULT;

  common::Error JsonSet(const NDbKValue& key, NDbKValue* added_key) WARN_UNUSED_RESULT;
  common::Error JsonGet(const NKey& key, NDbKValue* loaded_key) WARN_UNUSED_RESULT;

  // stream
  common::Error XAdd(const NDbKValue& key, NDbKValue* added_key, std::string* gen_id) WARN_UNUSED_RESULT;
  common::Error XRange(const NKey& key, NDbKValue* loaded_key, FastoObject* out) WARN_UNUSED_RESULT;

  bool IsInternalCommand(const std::string& command_name);

 private:
  common::Error JsonSetImpl(const NDbKValue& key, NDbKValue* added_key);
  common::Error JsonGetImpl(const NKey& key, NDbKValue* loaded_key);

  common::Error XAddImpl(const NDbKValue& key, NDbKValue* added_key, std::string* gen_id);
  common::Error XRangeImpl(const NKey& key, NDbKValue* loaded_key, FastoObject* out);

  virtual common::Error ModuleLoadImpl(const ModuleInfo& module) override;
  virtual common::Error ModuleUnLoadImpl(const ModuleInfo& module) override;
};

}  // namespace redis
}  // namespace core
}  // namespace fastonosql
