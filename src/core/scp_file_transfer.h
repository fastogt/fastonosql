#pragma once

#include "global/global.h"

namespace fastonosql
{
    int scp_send_file(const std::string& username, const std::string& password, const std::string& loclfile, const std::string& host, const std::string& scppath) WARN_UNUSED_RESULT;
}
