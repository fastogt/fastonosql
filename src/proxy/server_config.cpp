/*  Copyright (C) 2014-2020 FastoGT. All right reserved.

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
    along with FastoNoSQL. If not, see <http://www.gnu.org/licenses/>.
*/

#include "proxy/server_config.h"

#include <string>

#include <json-c/json_tokener.h>

#include <common/convert2string.h>
#include <common/protocols/json_rpc/json_rpc.h>
#include <common/protocols/json_rpc/json_rpc_response.h>

#include <common/system_info/system_info.h>

// methods
#define GET_VERSION_METHOD "version"
#define SEND_STATISTIC_METHOD "statistic"
#define ANONYMOUS_SEND_STATISTIC_METHOD "anonymous_statistic"
#define IS_SUBSCRIBED_METHOD "is_subscribed"
#define BAN_USER_METHOD "ban_user"

// subs state
#define SUBSCRIBED_LOGIN_FIELD "email"
#define SUBSCRIBED_PASSWORD_FIELD "password"

// ban
#define BAN_USER_LOGIN_FIELD "email"
#define BAN_USER_COLLISION_FIELD "collision_id"

// statistic
#define STATISTIC_EMAIL_FIELD "email"

#define STATISTIC_OS_FIELD "os"
#define STATISTIC_OS_NAME_FIELD "name"
#define STATISTIC_OS_VERSION_FIELD "version"
#define STATISTIC_OS_ARCH_FIELD "arch"

#define STATISTIC_PROJECT_FIELD "project"
#define STATISTIC_PROJECT_NAME_FIELD "name"
#define STATISTIC_PROJECT_BUILD_STRATEGY_FIELD "build_strategy"
#define STATISTIC_PROJECT_VERSION_FIELD "version"
#define STATISTIC_PROJECT_ARCH_FIELD "arch"

// subscription
#define USER_FIRST_NAME_FIELD "first_name"
#define USER_LAST_NAME_FIELD "last_name"
#define USER_SUBSCRIPTION_STATE_FIELD "subscription_state"
#define USER_TYPE_FIELD "type"
#define USER_EXEC_COUNT_FIELD "exec_count"
#define USER_EXPIRE_TIME_FIELD "expire_time"
#define USER_ID_FIELD "id"

namespace fastonosql {
namespace proxy {

common::Error GenVersionRequest(std::string* request) {
  if (!request) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  json_object* command_json = nullptr;
  common::protocols::json_rpc::JsonRPCRequest req;
  req.id = common::protocols::json_rpc::null_json_rpc_id;
  req.method = GET_VERSION_METHOD;
  common::Error err = common::protocols::json_rpc::MakeJsonRPCRequest(req, &command_json);
  if (err) {
    DNOTREACHED();
    return err;
  }

  const char* command_json_string = json_object_get_string(command_json);
  *request = command_json_string;
  json_object_put(command_json);
  return common::Error();
}

common::Error ParseVersionResponse(const std::string& data, uint32_t* version) {
  if (data.empty() || !version) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::protocols::json_rpc::JsonRPCResponse jres;
  common::Error err = common::protocols::json_rpc::ParseJsonRPCResponse(data, &jres);
  if (err) {
    DNOTREACHED();
    return err;
  }

  if (jres.IsError()) {
    DNOTREACHED();
    return common::make_error(jres.error->message);
  }

  const std::string result_str = jres.message->result;
  if (result_str.empty()) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  *version = common::ConvertVersionNumberFromString(result_str);
  return common::Error();
}

common::Error GenAnonymousStatisticRequest(std::string* request) {
  if (!request) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  json_object* stats_json = json_object_new_object();
  const common::system_info::SystemInfo inf = common::system_info::currentSystemInfo();

  json_object* os_json = json_object_new_object();
  const std::string os_name = inf.GetName();
  json_object_object_add(os_json, STATISTIC_OS_NAME_FIELD, json_object_new_string(os_name.c_str()));
  const std::string os_version = inf.GetVersion();
  json_object_object_add(os_json, STATISTIC_OS_VERSION_FIELD, json_object_new_string(os_version.c_str()));
  const std::string os_arch = inf.GetArch();
  json_object_object_add(os_json, STATISTIC_OS_ARCH_FIELD, json_object_new_string(os_arch.c_str()));
  json_object_object_add(stats_json, STATISTIC_OS_FIELD, os_json);

  json_object* project_json = json_object_new_object();
  json_object_object_add(project_json, STATISTIC_PROJECT_NAME_FIELD, json_object_new_string(PROJECT_NAME_TITLE));
  json_object_object_add(project_json, STATISTIC_PROJECT_VERSION_FIELD, json_object_new_string(PROJECT_VERSION));
  json_object_object_add(project_json, STATISTIC_PROJECT_ARCH_FIELD, json_object_new_string(PROJECT_ARCH));
  json_object_object_add(stats_json, STATISTIC_PROJECT_FIELD, project_json);

  json_object* command_json = nullptr;
  common::protocols::json_rpc::JsonRPCRequest req;
  req.id = common::protocols::json_rpc::null_json_rpc_id;
  req.method = ANONYMOUS_SEND_STATISTIC_METHOD;
  req.params = std::string(json_object_get_string(stats_json));
  json_object_put(stats_json);
  common::Error err = common::protocols::json_rpc::MakeJsonRPCRequest(req, &command_json);
  if (err) {
    DNOTREACHED();
    return err;
  }

  const char* command_json_string = json_object_get_string(command_json);
  *request = command_json_string;
  json_object_put(command_json);
  return common::Error();
}

common::Error GenStatisticRequest(const std::string& login, const std::string& build_strategy, std::string* request) {
  if (!request || login.empty() || build_strategy.empty()) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  json_object* stats_json = json_object_new_object();
  json_object_object_add(stats_json, STATISTIC_EMAIL_FIELD, json_object_new_string(login.c_str()));
  const common::system_info::SystemInfo inf = common::system_info::currentSystemInfo();

  json_object* os_json = json_object_new_object();
  const std::string os_name = inf.GetName();
  json_object_object_add(os_json, STATISTIC_OS_NAME_FIELD, json_object_new_string(os_name.c_str()));
  const std::string os_version = inf.GetVersion();
  json_object_object_add(os_json, STATISTIC_OS_VERSION_FIELD, json_object_new_string(os_version.c_str()));
  const std::string os_arch = inf.GetArch();
  json_object_object_add(os_json, STATISTIC_OS_ARCH_FIELD, json_object_new_string(os_arch.c_str()));
  json_object_object_add(stats_json, STATISTIC_OS_FIELD, os_json);

  json_object* project_json = json_object_new_object();
  json_object_object_add(project_json, STATISTIC_PROJECT_NAME_FIELD, json_object_new_string(PROJECT_NAME_TITLE));
  json_object_object_add(project_json, STATISTIC_PROJECT_BUILD_STRATEGY_FIELD,
                         json_object_new_string(build_strategy.c_str()));
  json_object_object_add(project_json, STATISTIC_PROJECT_VERSION_FIELD, json_object_new_string(PROJECT_VERSION));
  json_object_object_add(project_json, STATISTIC_PROJECT_ARCH_FIELD, json_object_new_string(PROJECT_ARCH));
  json_object_object_add(stats_json, STATISTIC_PROJECT_FIELD, project_json);

  json_object* command_json = nullptr;
  common::protocols::json_rpc::JsonRPCRequest req;
  req.id = common::protocols::json_rpc::null_json_rpc_id;
  req.method = SEND_STATISTIC_METHOD;
  req.params = std::string(json_object_get_string(stats_json));
  json_object_put(stats_json);
  common::Error err = common::protocols::json_rpc::MakeJsonRPCRequest(req, &command_json);
  if (err) {
    DNOTREACHED();
    return err;
  }

  const char* command_json_string = json_object_get_string(command_json);
  *request = command_json_string;
  json_object_put(command_json);
  return common::Error();
}

common::Error ParseSendStatisticResponse(const std::string& data) {
  if (data.empty()) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::protocols::json_rpc::JsonRPCResponse jres;
  return common::protocols::json_rpc::ParseJsonRPCResponse(data, &jres);
}

#if defined(PRO_VERSION) || defined(ENTERPRISE_VERSION)
common::Error GenSubscriptionStateRequest(const UserInfo& user_info, std::string* request) {
  if (!user_info.IsValid() || !request) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  const std::string login = user_info.GetLogin();
  const std::string password = user_info.GetPassword();
  json_object* cred_json = json_object_new_object();
  json_object_object_add(cred_json, SUBSCRIBED_LOGIN_FIELD, json_object_new_string(login.c_str()));
  json_object_object_add(cred_json, SUBSCRIBED_PASSWORD_FIELD, json_object_new_string(password.c_str()));

  json_object* is_subscribed_json = nullptr;
  common::protocols::json_rpc::JsonRPCRequest req;
  req.id = common::protocols::json_rpc::null_json_rpc_id;
  req.method = IS_SUBSCRIBED_METHOD;
  req.params = std::string(json_object_get_string(cred_json));
  common::Error err = common::protocols::json_rpc::MakeJsonRPCRequest(req, &is_subscribed_json);
  if (err) {
    DNOTREACHED();
    json_object_put(cred_json);
    return err;
  }

  const char* command_json_string = json_object_get_string(is_subscribed_json);
  *request = command_json_string;
  json_object_put(cred_json);
  json_object_put(is_subscribed_json);
  return common::Error();
}

common::Error ParseSubscriptionStateResponse(const std::string& data, UserInfo* update) {
  if (data.empty() || !update || !update->IsValid()) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::protocols::json_rpc::JsonRPCResponse jres;
  common::Error err = common::protocols::json_rpc::ParseJsonRPCResponse(data, &jres);
  if (err) {
    DNOTREACHED() << err->GetDescription();
    return err;
  }

  if (jres.IsError()) {
    return common::make_error(jres.error->message);
  }

  const std::string result_str = jres.message->result;
  if (result_str.empty()) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  json_object* obj = json_tokener_parse(result_str.c_str());
  if (!obj) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  UserInfo lres = *update;
  json_object* jfirst_name = nullptr;
  bool jfirst_name_exist = json_object_object_get_ex(obj, USER_FIRST_NAME_FIELD, &jfirst_name);
  if (!jfirst_name_exist) {
    DNOTREACHED();
    json_object_put(obj);
    return common::make_error_inval();
  }
  lres.SetFirstName(json_object_get_string(jfirst_name));

  json_object* jlast_name = nullptr;
  bool jlast_name_exist = json_object_object_get_ex(obj, USER_LAST_NAME_FIELD, &jlast_name);
  if (!jlast_name_exist) {
    DNOTREACHED();
    json_object_put(obj);
    return common::make_error_inval();
  }
  lres.SetLastName(json_object_get_string(jlast_name));

  json_object* jsubscription_state = nullptr;
  bool jsubscription_state_exist = json_object_object_get_ex(obj, USER_SUBSCRIPTION_STATE_FIELD, &jsubscription_state);
  if (!jsubscription_state_exist) {
    DNOTREACHED();
    json_object_put(obj);
    return common::make_error_inval();
  }
  proxy::UserInfo::SubscriptionState st =
      static_cast<proxy::UserInfo::SubscriptionState>(json_object_get_int(jsubscription_state));
  lres.SetSubscriptionState(st);

  json_object* jtype = nullptr;
  bool jtype_exist = json_object_object_get_ex(obj, USER_TYPE_FIELD, &jtype);
  if (!jtype_exist) {
    DNOTREACHED();
    json_object_put(obj);
    return common::make_error_inval();
  }
  const proxy::UserInfo::Type user_type = static_cast<proxy::UserInfo::Type>(json_object_get_int(jtype));
  lres.SetType(user_type);

  json_object* jexec_count = nullptr;
  bool jexec_count_exist = json_object_object_get_ex(obj, USER_EXEC_COUNT_FIELD, &jexec_count);
  if (!jexec_count_exist) {
    DNOTREACHED();
    json_object_put(obj);
    return common::make_error_inval();
  }
  int64_t exec_count = json_object_get_int64(jexec_count);
  lres.SetExecCount(static_cast<size_t>(exec_count));

  json_object* jexpire_time = nullptr;
  bool jexpire_time_exist = json_object_object_get_ex(obj, USER_EXPIRE_TIME_FIELD, &jexpire_time);
  if (!jexpire_time_exist) {
    DNOTREACHED();
    json_object_put(obj);
    return common::make_error_inval();
  }
  time_t expire_time = json_object_get_int64(jexpire_time);
  lres.SetExpireTime(expire_time);

  json_object* juser_id = nullptr;
  bool juser_id_exist = json_object_object_get_ex(obj, USER_ID_FIELD, &juser_id);
  if (!juser_id_exist) {
    DNOTREACHED();
    json_object_put(obj);
    return common::make_error_inval();
  }

  const proxy::user_id_t user_id = json_object_get_string(juser_id);
  lres.SetUserID(user_id);
  json_object_put(obj);
  *update = lres;
  return common::Error();
}

common::Error GenBanUserRequest(const UserInfo& user_info, user_id_t collision_id, std::string* request) {
  if (!request || !user_info.IsValid() || collision_id.empty()) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  json_object* ban_user_json = nullptr;
  common::protocols::json_rpc::JsonRPCRequest req;
  req.id = common::protocols::json_rpc::null_json_rpc_id;
  req.method = BAN_USER_METHOD;
  json_object* ban_json = json_object_new_object();
  const std::string login = user_info.GetLogin();
  json_object_object_add(ban_json, BAN_USER_LOGIN_FIELD, json_object_new_string(login.c_str()));
  json_object_object_add(ban_json, BAN_USER_COLLISION_FIELD, json_object_new_string(collision_id.c_str()));
  req.params = std::string(json_object_get_string(ban_json));
  json_object_put(ban_json);
  common::Error err = common::protocols::json_rpc::MakeJsonRPCRequest(req, &ban_user_json);
  if (err) {
    DNOTREACHED();
    return err;
  }

  const char* ban_user_json_str = json_object_get_string(ban_user_json);
  *request = ban_user_json_str;
  json_object_put(ban_user_json);
  return common::Error();
}

common::Error ParseGenBanUserResponse(const std::string& data) {
  if (data.empty()) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::protocols::json_rpc::JsonRPCResponse jres;
  return common::protocols::json_rpc::ParseJsonRPCResponse(data, &jres);
}
#endif

}  // namespace proxy
}  // namespace fastonosql
