#include "server/server_config.h"

#include <json-c/json_object.h>
#include <json-c/json_tokener.h>

#include <common/sprintf.h>
#include <common/system_info/system_info.h>  // for SystemInfo, etc

#define JSONRPC_FIELD "jsonrpc"
#define JSONRPC_METHOD_FIELD "method"
#define JSONRPC_PARAMS_FIELD "params"
#define JSONRPC_ID_FIELD "id"
#define JSONRPC_VERSION "2.0"

#define JSONRPC_ERROR_FIELD "error"
#define JSONRPC_ERROR_CODE_FIELD "code"
#define JSONRPC_ERROR_MESSAGE_FIELD "message"
#define JSONRPC_RESULT_FIELD "result"

// methods
#define GET_VERSION_METHOD "version"
#define SEND_STATISTIC_METHOD "statistic"
#define IS_SUBSCRIBED_METHOD "is_subscribed"

#define SUBSCRIBED_LOGIN_FIELD "email"
#define SUBSCRIBED_PASSWORD_FIELD "password"

#define STATISTIC_OS_FIELD "os"
#define STATISTIC_OS_NAME_FIELD "name"
#define STATISTIC_OS_VERSION_FIELD "version"
#define STATISTIC_OS_ARCH_FIELD "arch"

#define STATISTIC_PROJECT_FIELD "project"
#define STATISTIC_PROJECT_NAME_FIELD "name"
#define STATISTIC_PROJECT_VERSION_FIELD "version"
#define STATISTIC_PROJECT_ARCH_FIELD "arch"
#define STATISTIC_OWNER_FIELD "owner"
#define STATISTIC_PROJECT_EXEC_COUNT_FIELD "exec_count"

namespace {

json_object* GenerateJsonRpcCommand(const char* method, json_object* param = NULL) {
  json_object* command_json = json_object_new_object();
  json_object_object_add(command_json, JSONRPC_FIELD, json_object_new_string(JSONRPC_VERSION));
  json_object_object_add(command_json, JSONRPC_METHOD_FIELD, json_object_new_string(method));
  json_object_object_add(command_json, JSONRPC_ID_FIELD, NULL);
  if (param) {
    json_object_object_add(command_json, JSONRPC_PARAMS_FIELD, param);
  }
  return command_json;
}

}  // namespace

namespace fastonosql {
namespace server {

namespace {
JsonRPCError GetJsonRpcResult(json_object* rpc, std::string* result) {
  CHECK(rpc && result);

  json_object* jrpc = NULL;
  json_bool jrpc_exists = json_object_object_get_ex(rpc, JSONRPC_FIELD, &jrpc);
  if (!jrpc_exists) {
    return make_jsonrpc_error(JSON_RPC_INTERNAL_ERROR);
  }

  json_object* jerror = NULL;
  json_bool jerror_exists = json_object_object_get_ex(rpc, JSONRPC_ERROR_FIELD, &jerror);
  if (jerror_exists && json_object_get_type(jerror) != json_type_null) {
    json_object* jerror_code = NULL;
    json_bool jerror_code_exists = json_object_object_get_ex(jerror, JSONRPC_ERROR_CODE_FIELD, &jerror_code);

    json_object* jerror_message = NULL;
    json_bool jerror_message_exists = json_object_object_get_ex(jerror, JSONRPC_ERROR_MESSAGE_FIELD, &jerror_message);
    if (jerror_message_exists && jerror_code_exists) {
      std::string error_str = json_object_get_string(jerror_message);
      JsonRPCErrorCode err_code = static_cast<JsonRPCErrorCode>(json_object_get_int(jerror_code));
      return make_jsonrpc_error(err_code, error_str);
    }

    return make_jsonrpc_message_error(json_object_get_string(jerror));
  }

  json_object* jresult = NULL;
  json_bool jresult_exists = json_object_object_get_ex(rpc, JSONRPC_RESULT_FIELD, &jresult);
  if (!jresult_exists) {
    return make_jsonrpc_error_inval();
  }

  const char* result_str = json_object_get_string(jresult);
  *result = result_str;
  return JsonRPCError();
}

}  // namespace

#ifndef IS_PUBLIC_BUILD
common::Error GenSubscriptionStateRequest(const std::string& login, const std::string& password, std::string* request) {
  if (login.empty() || password.empty() || !request) {
    return common::make_error_inval();
  }

  json_object* cred_json = json_object_new_object();
  json_object_object_add(cred_json, SUBSCRIBED_LOGIN_FIELD, json_object_new_string(login.c_str()));
  json_object_object_add(cred_json, SUBSCRIBED_PASSWORD_FIELD, json_object_new_string(password.c_str()));

  json_object* is_subscribed_json = GenerateJsonRpcCommand(IS_SUBSCRIBED_METHOD, cred_json);
  const char* command_json_string = json_object_get_string(is_subscribed_json);
  *request = command_json_string;
  json_object_put(is_subscribed_json);
  return common::Error();
}

JsonRPCError ParseSubscriptionStateResponce(const std::string& data) {
  if (data.empty()) {
    return make_jsonrpc_error_inval();
  }

  const char* data_ptr = data.c_str();
  json_object* jdata = json_tokener_parse(data_ptr);
  if (!jdata) {
    return make_jsonrpc_error_inval();
  }

  std::string ok_str;
  JsonRPCError err = GetJsonRpcResult(jdata, &ok_str);
  json_object_put(jdata);
  return err;
}
#endif

common::Error GenVersionRequest(std::string* request) {
  if (!request) {
    return common::make_error_inval();
  }

  json_object* command_json = GenerateJsonRpcCommand(GET_VERSION_METHOD);
  const char* command_json_string = json_object_get_string(command_json);
  *request = command_json_string;
  json_object_put(command_json);
  return common::Error();
}

JsonRPCError ParseVersionResponce(const std::string& data, std::string* version_str) {
  if (data.empty() || !version_str) {
    return make_jsonrpc_error_inval();
  }

  const char* data_ptr = data.c_str();
  json_object* jdata = json_tokener_parse(data_ptr);
  if (!jdata) {
    return make_jsonrpc_error_inval();
  }

  JsonRPCError err = GetJsonRpcResult(jdata, version_str);
  json_object_put(jdata);
  return err;
}

common::Error GenStatisticRequest(const std::string& login, uint32_t exec_count, std::string* request) {
  if (!request || login.empty()) {
    return common::make_error_inval();
  }

  json_object* stats_json = json_object_new_object();
  common::system_info::SystemInfo inf = common::system_info::currentSystemInfo();

  json_object* os_json = json_object_new_object();
  const std::string os_name = inf.GetName();
  json_object_object_add(os_json, STATISTIC_OS_NAME_FIELD, json_object_new_string(os_name.c_str()));
  const std::string os_version = inf.GetVersion();
  json_object_object_add(os_json, STATISTIC_OS_VERSION_FIELD, json_object_new_string(os_version.c_str()));
  const std::string os_arch = inf.GetArch();
  json_object_object_add(os_json, STATISTIC_OS_ARCH_FIELD, json_object_new_string(os_arch.c_str()));
  json_object_object_add(stats_json, STATISTIC_OS_FIELD, os_json);

  json_object* project_json = json_object_new_object();
  json_object_object_add(project_json, STATISTIC_PROJECT_NAME_FIELD, json_object_new_string(PROJECT_NAME));
  json_object_object_add(project_json, STATISTIC_PROJECT_VERSION_FIELD, json_object_new_string(PROJECT_VERSION));
  json_object_object_add(project_json, STATISTIC_PROJECT_ARCH_FIELD, json_object_new_string(PROJECT_ARCH));
#ifndef IS_PUBLIC_BUILD
  json_object_object_add(project_json, STATISTIC_OWNER_FIELD, json_object_new_string(login.c_str()));
#endif
  json_object_object_add(project_json, STATISTIC_PROJECT_EXEC_COUNT_FIELD,
                         json_object_new_int64(static_cast<int64_t>(exec_count)));
  json_object_object_add(stats_json, STATISTIC_PROJECT_FIELD, project_json);

  json_object* command_json = GenerateJsonRpcCommand(SEND_STATISTIC_METHOD, stats_json);
  const char* command_json_string = json_object_get_string(command_json);
  *request = command_json_string;
  json_object_put(command_json);
  return common::Error();
}

JsonRPCError ParseSendStatisticResponce(const std::string& data) {
  if (data.empty()) {
    return make_jsonrpc_error_inval();
  }

  const char* data_ptr = data.c_str();
  json_object* jdata = json_tokener_parse(data_ptr);
  if (!jdata) {
    return make_jsonrpc_error_inval();
  }

  std::string ok_str;
  JsonRPCError err = GetJsonRpcResult(jdata, &ok_str);
  json_object_put(jdata);
  return err;
}

}  // namespace server
}  // namespace fastonosql
