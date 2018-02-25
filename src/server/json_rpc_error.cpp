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

#include "server/json_rpc_error.h"

#include <common/sprintf.h>

namespace fastonosql {
namespace server {

std::string JsonRPCErrorTraits::GetTextFromErrorCode(JsonRPCErrorCode error) {
  if (error == JSON_RPC_PARSE_ERROR) {
    return "Parse error";
  } else if (error == JSON_RPC_INVALID_REQUEST) {
    return "Invalid Request";
  } else if (error == JSON_RPC_METHOD_NOT_FOUND) {
    return "Method not found";
  } else if (error == JSON_RPC_INVALID_PARAMS) {
    return "Invalid params";
  } else if (error == JSON_RPC_INTERNAL_ERROR) {
    return "Internal error";
  } else if (error == JSON_RPC_SERVER_ERROR) {
    return "Server error";
  } else if (error == JSON_RPC_AUTH_ERROR) {
    return "Authentification error";
  } else if (error == JSON_RPC_PASSWORD_MISSMATCH) {
    return "Password missmatch";
  } else if (error == JSON_RPC_INVALID_INPUT) {
    return "Invalid input";
  } else if (error == JSON_RPC_ONLY_MESSAGE_ERROR) {
    return "Response error";
  }
  return common::MemSPrintf("Unknown json rpc error code: %d.", static_cast<int>(error));
}

JsonRPCError make_jsonrpc_error_inval() {
  return JsonRPCErrorValue(JSON_RPC_INVALID_INPUT);
}

JsonRPCError make_jsonrpc_error(JsonRPCErrorCode err, const std::string& message) {
  return JsonRPCErrorValue(err, message);
}

JsonRPCError make_jsonrpc_message_error(const std::string& message) {
  return JsonRPCErrorValue(JSON_RPC_ONLY_MESSAGE_ERROR, message);
}

}  // namespace server
}  // namespace fastonosql
