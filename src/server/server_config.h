#pragma once

#include <common/error.h>

#define SERVER_REQUESTS_PORT 5001

#define FASTONOSQL_HOST "fastonosql.com"
#define FASTOREDIS_HOST "fastoredis.com"

namespace fastonosql {
namespace server {

#ifndef IS_PUBLIC_BUILD
common::Error GenSubscriptionStateRequest(const std::string& login, const std::string& password, std::string* request);
common::Error ParseSubscriptionStateResponce(const std::string& data, bool* is_ok);
#endif

common::Error GenVersionRequest(std::string* request);
common::Error ParseVersionResponce(const std::string& data, std::string* version_str);

common::Error GenStatisticRequest(const std::string& login, uint32_t exec_count, std::string* request);
common::Error ParseSendStatisticResponce(const std::string& data, bool* is_sent);

}  // namespace server
}  // namespace fastonosql
