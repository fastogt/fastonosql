/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

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

#include "core/db/ssdb/internal/commands_api.h"

#include <stddef.h>  // for size_t
#include <stdint.h>  // for int64_t, uint64_t

#include <map>     // for map, map<>::mapped_type
#include <memory>  // for __shared_ptr
#include <string>  // for string

#include <common/convert2string.h>  // for ConvertFromString
#include <common/macros.h>          // for UNUSED
#include <common/value.h>           // for Value, ErrorValue, etc

#include "core/db/ssdb/db_connection.h"  // for DBConnection
#include "core/db/ssdb/server_info.h"    // for ServerInfo, etc

#include "core/global.h"  // for FastoObject, etc

namespace fastonosql {
namespace core {
namespace ssdb {

common::Error CommandsApi::Info(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  ServerInfo::Stats statsout;
  common::Error err = ssdb->Info(argc == 1 ? argv[0] : 0, &statsout);
  if (err && err->IsError()) {
    return err;
  }

  ServerInfo sinf(statsout);
  common::StringValue* val = common::Value::CreateStringValue(sinf.ToString());
  FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::ScanSSDB(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  uint64_t limit;
  if (!common::ConvertFromString(argv[2], &limit)) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }
  std::vector<std::string> keysout;
  common::Error err = ssdb->Scan(argv[0], argv[1], limit, &keysout);
  if (err && err->IsError()) {
    return err;
  }

  common::ArrayValue* ar = common::Value::CreateArrayValue();
  for (size_t i = 0; i < keysout.size(); ++i) {
    common::StringValue* val = common::Value::CreateStringValue(keysout[i]);
    ar->Append(val);
  }
  FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::DBsize(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);
  UNUSED(argv);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t dbsize = 0;
  common::Error err = ssdb->DBsize(&dbsize);
  if (err && err->IsError()) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateUIntegerValue(dbsize);
  FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Auth(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  common::Error err = ssdb->Auth(argv[0]);
  if (err && err->IsError()) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Setx(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  ttl_t ttl;
  if (!common::ConvertFromString(argv[2], &ttl)) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }
  common::Error err = ssdb->Setx(argv[0], argv[1], ttl);
  if (err && err->IsError()) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Incr(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t incrby;
  if (!common::ConvertFromString(argv[1], &incrby)) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }
  int64_t ret = 0;
  common::Error err = ssdb->Incr(argv[0], incrby, &ret);
  if (err && err->IsError()) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateIntegerValue(ret);
  FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Rscan(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  uint64_t limit;
  if (!common::ConvertFromString(argv[2], &limit)) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }
  std::vector<std::string> keysout;
  common::Error err = ssdb->Rscan(argv[0], argv[1], limit, &keysout);
  if (err && err->IsError()) {
    return err;
  }

  common::ArrayValue* ar = common::Value::CreateArrayValue();
  for (size_t i = 0; i < keysout.size(); ++i) {
    common::StringValue* val = common::Value::CreateStringValue(keysout[i]);
    ar->Append(val);
  }
  FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::MultiGet(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysget;
  for (int i = 0; i < argc; ++i) {
    keysget.push_back(argv[i]);
  }

  std::vector<std::string> keysout;
  common::Error err = ssdb->MultiGet(keysget, &keysout);
  if (err && err->IsError()) {
    return err;
  }

  common::ArrayValue* ar = common::Value::CreateArrayValue();
  for (size_t i = 0; i < keysout.size(); ++i) {
    common::StringValue* val = common::Value::CreateStringValue(keysout[i]);
    ar->Append(val);
  }
  FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::MultiSet(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::map<std::string, std::string> keysset;
  for (int i = 0; i < argc; i += 2) {
    keysset[argv[i]] = argv[i + 1];
  }

  common::Error err = ssdb->MultiSet(keysset);
  if (err && err->IsError()) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::MultiDel(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysget;
  for (int i = 0; i < argc; ++i) {
    keysget.push_back(argv[i]);
  }

  common::Error err = ssdb->MultiDel(keysget);
  if (err && err->IsError()) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Hget(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::string ret;
  common::Error err = ssdb->Hget(argv[0], argv[1], &ret);
  if (err && err->IsError()) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue(ret);
  FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Hgetall(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysout;
  common::Error err = ssdb->Hgetall(argv[0], &keysout);
  if (err && err->IsError()) {
    return err;
  }

  common::ArrayValue* ar = common::Value::CreateArrayValue();
  for (size_t i = 0; i < keysout.size(); ++i) {
    common::StringValue* val = common::Value::CreateStringValue(keysout[i]);
    ar->Append(val);
  }
  FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Hset(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  common::Error err = ssdb->Hset(argv[0], argv[1], argv[2]);
  if (err && err->IsError()) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Hdel(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  common::Error err = ssdb->Hdel(argv[0], argv[1]);
  if (err && err->IsError()) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Hincr(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t incrby;
  if (!common::ConvertFromString(argv[2], &incrby)) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }
  int64_t res = 0;
  common::Error err = ssdb->Hincr(argv[0], argv[1], incrby, &res);
  if (err && err->IsError()) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateIntegerValue(res);
  FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Hsize(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t res = 0;
  common::Error err = ssdb->Hsize(argv[0], &res);
  if (err && err->IsError()) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateIntegerValue(res);
  FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Hclear(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t res = 0;
  common::Error err = ssdb->Hclear(argv[0], &res);
  if (err && err->IsError()) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateIntegerValue(res);
  FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Hkeys(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  uint64_t limit;
  if (!common::ConvertFromString(argv[3], &limit)) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }
  std::vector<std::string> keysout;
  common::Error err = ssdb->Hkeys(argv[0], argv[1], argv[2], limit, &keysout);
  if (err && err->IsError()) {
    return err;
  }

  common::ArrayValue* ar = common::Value::CreateArrayValue();
  for (size_t i = 0; i < keysout.size(); ++i) {
    common::StringValue* val = common::Value::CreateStringValue(keysout[i]);
    ar->Append(val);
  }
  FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Hscan(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  uint64_t limit;
  if (!common::ConvertFromString(argv[3], &limit)) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }
  std::vector<std::string> keysout;
  common::Error err = ssdb->Hscan(argv[0], argv[1], argv[2], limit, &keysout);
  if (err && err->IsError()) {
    return err;
  }

  common::ArrayValue* ar = common::Value::CreateArrayValue();
  for (size_t i = 0; i < keysout.size(); ++i) {
    common::StringValue* val = common::Value::CreateStringValue(keysout[i]);
    ar->Append(val);
  }
  FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Hrscan(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  uint64_t limit;
  if (!common::ConvertFromString(argv[3], &limit)) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }
  std::vector<std::string> keysout;
  common::Error err = ssdb->Hrscan(argv[0], argv[1], argv[2], limit, &keysout);
  if (err && err->IsError()) {
    return err;
  }

  common::ArrayValue* ar = common::Value::CreateArrayValue();
  for (size_t i = 0; i < keysout.size(); ++i) {
    common::StringValue* val = common::Value::CreateStringValue(keysout[i]);
    ar->Append(val);
  }
  FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::MultiHget(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysget;
  for (int i = 1; i < argc; ++i) {
    keysget.push_back(argv[i]);
  }

  std::vector<std::string> keysout;
  common::Error err = ssdb->MultiHget(argv[0], keysget, &keysout);
  if (err && err->IsError()) {
    return err;
  }

  common::ArrayValue* ar = common::Value::CreateArrayValue();
  for (size_t i = 0; i < keysout.size(); ++i) {
    common::StringValue* val = common::Value::CreateStringValue(keysout[i]);
    ar->Append(val);
  }
  FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::MultiHset(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::map<std::string, std::string> keys;
  for (int i = 1; i < argc; i += 2) {
    keys[argv[i]] = argv[i + 1];
  }

  common::Error err = ssdb->MultiHset(argv[0], keys);
  if (err && err->IsError()) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Zget(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t ret;
  common::Error err = ssdb->Zget(argv[0], argv[1], &ret);
  if (err && err->IsError()) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateIntegerValue(ret);
  FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Zset(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t score;
  if (!common::ConvertFromString(argv[2], &score)) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  common::Error err = ssdb->Zset(argv[0], argv[1], score);
  if (err && err->IsError()) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Zdel(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  common::Error err = ssdb->Zdel(argv[0], argv[1]);
  if (err && err->IsError()) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Zincr(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t incrby;
  if (!common::ConvertFromString(argv[2], &incrby)) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  int64_t ret = 0;
  common::Error err = ssdb->Zincr(argv[0], argv[1], incrby, &ret);
  if (err && err->IsError()) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateIntegerValue(ret);
  FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Zsize(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t res = 0;
  common::Error err = ssdb->Zsize(argv[0], &res);
  if (err && err->IsError()) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateIntegerValue(res);
  FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Zclear(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t res = 0;
  common::Error err = ssdb->Zclear(argv[0], &res);
  if (err && err->IsError()) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateIntegerValue(res);
  FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Zrank(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t res = 0;
  common::Error err = ssdb->Zrank(argv[0], argv[1], &res);
  if (err && err->IsError()) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateIntegerValue(res);
  FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Zrrank(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t res = 0;
  common::Error err = ssdb->Zrrank(argv[0], argv[1], &res);
  if (err && err->IsError()) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateIntegerValue(res);
  FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Zrange(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  uint64_t offset;
  if (!common::ConvertFromString(argv[1], &offset)) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  uint64_t limit;
  if (!common::ConvertFromString(argv[2], &limit)) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  std::vector<std::string> res;
  common::Error err = ssdb->Zrange(argv[0], offset, limit, &res);
  if (err && err->IsError()) {
    return err;
  }

  common::ArrayValue* ar = common::Value::CreateArrayValue();
  for (size_t i = 0; i < res.size(); ++i) {
    common::StringValue* val = common::Value::CreateStringValue(res[i]);
    ar->Append(val);
  }
  FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Zrrange(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  uint64_t offset;
  if (!common::ConvertFromString(argv[1], &offset)) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  uint64_t limit;
  if (!common::ConvertFromString(argv[2], &limit)) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  std::vector<std::string> res;
  common::Error err = ssdb->Zrrange(argv[0], offset, limit, &res);
  if (err && err->IsError()) {
    return err;
  }

  common::ArrayValue* ar = common::Value::CreateArrayValue();
  for (size_t i = 0; i < res.size(); ++i) {
    common::StringValue* val = common::Value::CreateStringValue(res[i]);
    ar->Append(val);
  }
  FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Zkeys(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> res;
  int64_t st;
  if (!common::ConvertFromString(argv[2], &st)) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  int64_t end;
  if (!common::ConvertFromString(argv[3], &end)) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  uint64_t limit;
  if (!common::ConvertFromString(argv[4], &limit)) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  common::Error err = ssdb->Zkeys(argv[0], argv[1], &st, &end, limit, &res);
  if (err && err->IsError()) {
    return err;
  }

  common::ArrayValue* ar = common::Value::CreateArrayValue();
  for (size_t i = 0; i < res.size(); ++i) {
    common::StringValue* val = common::Value::CreateStringValue(res[i]);
    ar->Append(val);
  }
  FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Zscan(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> res;
  int64_t st;
  if (!common::ConvertFromString(argv[2], &st)) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  int64_t end;
  if (!common::ConvertFromString(argv[3], &end)) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  uint64_t limit;
  if (!common::ConvertFromString(argv[4], &limit)) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }
  common::Error err = ssdb->Zscan(argv[0], argv[1], &st, &end, limit, &res);
  if (err && err->IsError()) {
    return err;
  }

  common::ArrayValue* ar = common::Value::CreateArrayValue();
  for (size_t i = 0; i < res.size(); ++i) {
    common::StringValue* val = common::Value::CreateStringValue(res[i]);
    ar->Append(val);
  }
  FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Zrscan(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> res;
  int64_t st;
  if (!common::ConvertFromString(argv[2], &st)) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  int64_t end;
  if (!common::ConvertFromString(argv[3], &end)) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  uint64_t limit;
  if (!common::ConvertFromString(argv[4], &limit)) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }
  common::Error err = ssdb->Zrscan(argv[0], argv[1], &st, &end, limit, &res);
  if (err && err->IsError()) {
    return err;
  }

  common::ArrayValue* ar = common::Value::CreateArrayValue();
  for (size_t i = 0; i < res.size(); ++i) {
    common::StringValue* val = common::Value::CreateStringValue(res[i]);
    ar->Append(val);
  }
  FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::MultiZget(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysget;
  for (int i = 1; i < argc; ++i) {
    keysget.push_back(argv[i]);
  }

  std::vector<std::string> res;
  common::Error err = ssdb->MultiZget(argv[0], keysget, &res);
  if (err && err->IsError()) {
    return err;
  }

  common::ArrayValue* ar = common::Value::CreateArrayValue();
  for (size_t i = 0; i < res.size(); ++i) {
    common::StringValue* val = common::Value::CreateStringValue(res[i]);
    ar->Append(val);
  }
  FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::MultiZset(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::map<std::string, int64_t> keysget;
  for (int i = 1; i < argc; i += 2) {
    int64_t val;
    if (!common::ConvertFromString(argv[i + 1], &val)) {
      keysget[argv[i]] = val;
    }
  }

  common::Error err = ssdb->MultiZset(argv[0], keysget);
  if (err && err->IsError()) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::MultiZdel(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysget;
  for (int i = 1; i < argc; ++i) {
    keysget.push_back(argv[i]);
  }

  common::Error err = ssdb->MultiZdel(argv[0], keysget);
  if (err && err->IsError()) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Qpush(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  common::Error err = ssdb->Qpush(argv[0], argv[1]);
  if (err && err->IsError()) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue("OK");
  FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Qpop(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::string ret;
  common::Error err = ssdb->Qpop(argv[0], &ret);
  if (err && err->IsError()) {
    return err;
  }

  common::StringValue* val = common::Value::CreateStringValue(ret);
  FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Qslice(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t begin;
  if (!common::ConvertFromString(argv[1], &begin)) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  int64_t end;
  if (!common::ConvertFromString(argv[2], &end)) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  std::vector<std::string> keysout;
  common::Error err = ssdb->Qslice(argv[0], begin, end, &keysout);
  if (err && err->IsError()) {
    return err;
  }

  common::ArrayValue* ar = common::Value::CreateArrayValue();
  for (size_t i = 0; i < keysout.size(); ++i) {
    common::StringValue* val = common::Value::CreateStringValue(keysout[i]);
    ar->Append(val);
  }
  FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Qclear(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t res = 0;
  common::Error err = ssdb->Qclear(argv[0], &res);
  if (err && err->IsError()) {
    return err;
  }

  common::FundamentalValue* val = common::Value::CreateIntegerValue(res);
  FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

}  // namespace ssdb
}  // namespace core
}  // namespace fastonosql
