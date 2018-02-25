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

namespace fastonosql {
namespace server {

// Json RPC error
enum JsonRPCErrorCode {
  JSON_RPC_PARSE_ERROR = -32700,       // 	Parse error
  JSON_RPC_INVALID_REQUEST = -32600,   // Invalid Request
  JSON_RPC_METHOD_NOT_FOUND = -32601,  // Method not found
  JSON_RPC_INVALID_PARAMS = -32602,    // Invalid params
  JSON_RPC_INTERNAL_ERROR = -32603,    // Internal error
  JSON_RPC_SERVER_ERROR = -32000,      // to -32099 	// Server error
  JSON_RPC_AUTH_ERROR = -32001,
  JSON_RPC_PASSWORD_MISSMATCH = -32002,
  JSON_RPC_INVALID_INPUT = -32003,
  JSON_RPC_ONLY_MESSAGE_ERROR = -32004  // should be removed
};

struct JsonRPCErrorTraits {
  static std::string GetTextFromErrorCode(JsonRPCErrorCode error);
};

class JsonRPCErrorValue : public common::ErrorBase<JsonRPCErrorCode, JsonRPCErrorTraits> {
 public:
  typedef common::ErrorBase<JsonRPCErrorCode, JsonRPCErrorTraits> base_class;
  explicit JsonRPCErrorValue(JsonRPCErrorCode error_code) : base_class(error_code), message_() {}
  JsonRPCErrorValue(JsonRPCErrorCode err, const std::string& message) : base_class(err), message_(message) {}

  std::string GetMessage() const { return message_; }

 private:
  std::string message_;
};
typedef common::Optional<JsonRPCErrorValue> JsonRPCError;

JsonRPCError make_jsonrpc_error_inval();
JsonRPCError make_jsonrpc_error(JsonRPCErrorCode err, const std::string& message = std::string());
JsonRPCError make_jsonrpc_message_error(const std::string& message);

}  // namespace server
}  // namespace fastonosql
