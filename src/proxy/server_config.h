#pragma once

#include <common/error.h>

#include "proxy/user_info.h"

#define SERVER_REQUESTS_PORT 333

#define FASTONOSQL_HOST "fastonosql.com"
#define FASTOREDIS_HOST "fastoredis.com"

namespace fastonosql {
namespace proxy {

common::Error GenVersionRequest(std::string* request);
common::Error ParseVersionResponce(const std::string& data, uint32_t* version);

#if defined(PRO_VERSION)
common::Error GenSubscriptionStateRequest(const UserInfo& user_info, std::string* request);
common::Error ParseSubscriptionStateResponce(const std::string& data, UserInfo* result);

common::Error GenStatisticRequest(const std::string& login, const std::string& build_strategy, std::string* request);
common::Error ParseSendStatisticResponce(const std::string& data);

common::Error GenBanUserRequest(const UserInfo& user_info, user_id_t collision_id, std::string* request);
common::Error ParseGenBanUserResponce(const std::string& data);
#endif

}  // namespace proxy
}  // namespace fastonosql
