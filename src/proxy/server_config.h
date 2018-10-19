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

#include <common/error.h>

#if defined(PRO_VERSION)
#include "proxy/user_info.h"
#endif

#define SERVER_REQUESTS_PORT 333

#define FASTONOSQL_HOST "fastonosql.com"
#define FASTOREDIS_HOST "fastoredis.com"

namespace fastonosql {
namespace proxy {

common::Error GenVersionRequest(std::string* request);
common::Error ParseVersionResponce(const std::string& data, uint32_t* version);

common::Error GenAnonymousStatisticRequest(std::string* request);
common::Error GenStatisticRequest(const std::string& login, const std::string& build_strategy, std::string* request);
common::Error ParseSendStatisticResponce(const std::string& data);

#if defined(PRO_VERSION)
common::Error GenSubscriptionStateRequest(const UserInfo& user_info, std::string* request);
common::Error ParseSubscriptionStateResponce(const std::string& data, UserInfo* update);

common::Error GenBanUserRequest(const UserInfo& user_info, user_id_t collision_id, std::string* request);
common::Error ParseGenBanUserResponce(const std::string& data);
#endif

}  // namespace proxy
}  // namespace fastonosql
