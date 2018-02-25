#pragma once

#include "server/json_rpc_error.h"

#define SERVER_REQUESTS_PORT 5001

#define FASTONOSQL_HOST "fastonosql.com"
#define FASTOREDIS_HOST "fastoredis.com"

namespace fastonosql {
namespace server {

#ifndef IS_PUBLIC_BUILD
common::Error GenSubscriptionStateRequest(const std::string& login, const std::string& password, std::string* request);
JsonRPCError ParseSubscriptionStateResponce(const std::string& data);
#endif

common::Error GenVersionRequest(std::string* request);
JsonRPCError ParseVersionResponce(const std::string& data, std::string* version_str);

common::Error GenStatisticRequest(const std::string& login, uint32_t exec_count, std::string* request);
JsonRPCError ParseSendStatisticResponce(const std::string& data);

}  // namespace server
}  // namespace fastonosql
