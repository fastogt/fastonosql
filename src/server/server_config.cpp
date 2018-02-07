#include "server/server_config.h"

#include <json-c/json_object.h>
#include <json-c/json_tokener.h>

#include <common/system_info/system_info.h>  // for SystemInfo, etc

#define JSONRPC_FIELD "jsonrpc"
#define JSONRPC_METHOD_FIELD "method"
#define JSONRPC_PARAMS_FIELD "params"
#define JSONRPC_ID_FIELD "id"
#define JSONRPC_VERSION "2.0"

#define JSONRPC_ERROR_FIELD "error"
#define JSONRPC_RESULT_FIELD "result"

#define SUCCESS_RESULT "OK"

// methods
#define GET_VERSION_METHOD "version"
#define SEND_STATISTIC_METHOD "statistic"

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

common::Error GetJsonRpcResult(json_object* rpc, std::string* result) {
  if (!rpc || !result) {
    return common::make_error_inval();
  }

  json_object* jrpc = NULL;
  json_bool jrpc_exists = json_object_object_get_ex(rpc, JSONRPC_FIELD, &jrpc);
  if (!jrpc_exists) {
    return common::make_error_inval();
  }

  json_object* jerror = NULL;
  json_bool jerror_exists = json_object_object_get_ex(rpc, JSONRPC_ERROR_FIELD, &jerror);
  if (!jerror_exists) {
    return common::make_error_inval();
  }

  json_object* jresult = NULL;
  json_bool jresult_exists = json_object_object_get_ex(rpc, JSONRPC_RESULT_FIELD, &jresult);
  if (!jresult_exists) {
    return common::make_error_inval();
  }

  const char* result_str = json_object_get_string(jresult);
  *result = result_str;
  return common::Error();
}

}  // namespace

namespace fastonosql {
namespace server {

std::string GetVersionRequest() {
  json_object* command_json = GenerateJsonRpcCommand(GET_VERSION_METHOD);
  const char* command_json_string = json_object_get_string(command_json);
  std::string request = command_json_string;
  json_object_put(command_json);
  return request;
}

common::Error ParseVersionResponce(const std::string& data, std::string* version_str) {
  if (data.empty() || !version_str) {
    return common::make_error_inval();
  }

  const char* data_ptr = data.c_str();
  json_object* jdata = json_tokener_parse(data_ptr);
  if (!jdata) {
    return common::make_error_inval();
  }

  common::Error err = GetJsonRpcResult(jdata, version_str);
  json_object_put(jdata);
  return err;
}

std::string SendStatisticRequest(uint32_t exec_count) {
  json_object* stats_json = json_object_new_object();
  common::system_info::SystemInfo inf = common::system_info::currentSystemInfo();

  json_object* os_json = json_object_new_object();
  json_object_object_add(os_json, STATISTIC_OS_NAME_FIELD, json_object_new_string(inf.GetName().c_str()));
  json_object_object_add(os_json, STATISTIC_OS_VERSION_FIELD, json_object_new_string(inf.GetVersion().c_str()));
  json_object_object_add(os_json, STATISTIC_OS_ARCH_FIELD, json_object_new_string(inf.GetArch().c_str()));
  json_object_object_add(stats_json, STATISTIC_OS_FIELD, os_json);

  json_object* project_json = json_object_new_object();
  json_object_object_add(project_json, STATISTIC_PROJECT_NAME_FIELD, json_object_new_string(PROJECT_NAME));
  json_object_object_add(project_json, STATISTIC_PROJECT_VERSION_FIELD, json_object_new_string(PROJECT_VERSION));
  json_object_object_add(project_json, STATISTIC_PROJECT_ARCH_FIELD, json_object_new_string(PROJECT_ARCH));
#ifndef IS_PUBLIC_BUILD
  json_object_object_add(project_json, STATISTIC_OWNER_FIELD, json_object_new_string(USER_SPECIFIC_ID));
#endif
  json_object_object_add(project_json, STATISTIC_PROJECT_EXEC_COUNT_FIELD,
                         json_object_new_int64(static_cast<int64_t>(exec_count)));
  json_object_object_add(stats_json, STATISTIC_PROJECT_FIELD, project_json);

  json_object* command_json = GenerateJsonRpcCommand(SEND_STATISTIC_METHOD);
  const char* command_json_string = json_object_get_string(command_json);
  std::string request = command_json_string;
  json_object_put(command_json);
  return request;
}

common::Error ParseSendStatisticResponce(const std::string& data, bool* is_sent) {
  if (data.empty() || !is_sent) {
    return common::make_error_inval();
  }

  const char* data_ptr = data.c_str();
  json_object* jdata = json_tokener_parse(data_ptr);
  if (!jdata) {
    return common::make_error_inval();
  }

  std::string ok_str;
  common::Error err = GetJsonRpcResult(jdata, &ok_str);
  *is_sent = ok_str == SUCCESS_RESULT;
  json_object_put(jdata);
  return err;
}

}  // namespace server
}  // namespace fastonosql
