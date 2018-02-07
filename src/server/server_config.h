#pragma once

#include <common/error.h>

#define SERVER_REQUESTS_PORT 5001

#define FASTONOSQL_HOST "fastonosql.com"
#define FASTOREDIS_HOST "fastoredis.com"

namespace fastonosql {
namespace server {

std::string GetVersionRequest();
common::Error ParseVersionResponce(const std::string& data, std::string* version_str);

std::string SendStatisticRequest(uint32_t exec_count);
common::Error ParseSendStatisticResponce(const std::string& data, bool* is_sent);

}  // namespace server
}  // namespace fastonosql
