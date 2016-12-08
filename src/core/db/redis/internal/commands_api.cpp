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

#include "core/db/redis/internal/commands_api.h"

#include <string.h>  // for strncmp
#include <memory>    // for __shared_ptr

#include <common/value.h>  // for Value, ErrorValue, etc
#include <common/convert2string.h>

#include "core/db_key.h"

#include "core/db/redis/db_connection.h"

#include "global/global.h"

namespace fastonosql {
namespace core {
namespace redis {

common::Error CommandsApi::CommonExec(internal::CommandHandler* handler,
                                      int argc,
                                      const char** argv,
                                      FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(argc + 1, argv - 1, out);
}

common::Error CommandsApi::CommonExecOff2(internal::CommandHandler* handler,
                                          int argc,
                                          const char** argv,
                                          FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->CommonExec(argc + 2, argv - 2, out);
}

common::Error CommandsApi::Auth(internal::CommandHandler* handler,
                                int argc,
                                const char** argv,
                                FastoObject* out) {
  UNUSED(argc);
  DBConnection* red = static_cast<DBConnection*>(handler);
  common::Error err = red->Auth(argv[0]);
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, red->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Lpush(internal::CommandHandler* handler,
                                 int argc,
                                 const char** argv,
                                 FastoObject* out) {
  NKey key(argv[0]);
  common::ArrayValue* arr = common::Value::createArrayValue();
  for (int i = 1; i < argc; ++i) {
    arr->appendString(argv[i]);
  }

  DBConnection* redis = static_cast<DBConnection*>(handler);
  int list_len = 0;
  common::Error err = redis->Lpush(key, NValue(arr), &list_len);
  if (err && err->isError()) {
    return err;
  }

  common::FundamentalValue* val = common::Value::createIntegerValue(list_len);
  FastoObject* child = new FastoObject(out, val, redis->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Lrange(internal::CommandHandler* handler,
                                  int argc,
                                  const char** argv,
                                  FastoObject* out) {
  UNUSED(argc);

  NKey key(argv[0]);
  int start = common::ConvertFromString<int>(argv[1]);
  int stop = common::ConvertFromString<int>(argv[2]);
  DBConnection* redis = static_cast<DBConnection*>(handler);
  NDbKValue key_loaded;
  common::Error err = redis->Lrange(key, start, stop, &key_loaded);
  if (err && err->isError()) {
    return err;
  }

  NValue val = key_loaded.Value();
  common::Value* copy = val->deepCopy();
  FastoObject* child = new FastoObject(out, copy, redis->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Sadd(internal::CommandHandler* handler,
                                int argc,
                                const char** argv,
                                FastoObject* out) {
  NKey key(argv[0]);
  common::SetValue* set = common::Value::createSetValue();
  for (int i = 1; i < argc; ++i) {
    set->insert(argv[i]);
  }

  DBConnection* redis = static_cast<DBConnection*>(handler);
  int added_items = 0;
  common::Error err = redis->Sadd(key, NValue(set), &added_items);
  if (err && err->isError()) {
    return err;
  }

  common::FundamentalValue* val = common::Value::createIntegerValue(added_items);
  FastoObject* child = new FastoObject(out, val, redis->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Smembers(internal::CommandHandler* handler,
                                    int argc,
                                    const char** argv,
                                    FastoObject* out) {
  UNUSED(argc);

  NKey key(argv[0]);
  DBConnection* redis = static_cast<DBConnection*>(handler);
  NDbKValue key_loaded;
  common::Error err = redis->Smembers(key, &key_loaded);
  if (err && err->isError()) {
    return err;
  }

  NValue val = key_loaded.Value();
  common::Value* copy = val->deepCopy();
  FastoObject* child = new FastoObject(out, copy, redis->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Zadd(internal::CommandHandler* handler,
                                int argc,
                                const char** argv,
                                FastoObject* out) {
  NKey key(argv[0]);
  common::ZSetValue* zset = common::Value::createZSetValue();
  for (int i = 1; i < argc; i += 2) {
    std::string key = argv[i];
    std::string val = argv[i + 1];
    zset->insert(key, val);
  }

  DBConnection* redis = static_cast<DBConnection*>(handler);
  int added_items = 0;
  common::Error err = redis->Zadd(key, NValue(zset), &added_items);
  if (err && err->isError()) {
    return err;
  }

  common::FundamentalValue* val = common::Value::createIntegerValue(added_items);
  FastoObject* child = new FastoObject(out, val, redis->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Zrange(internal::CommandHandler* handler,
                                  int argc,
                                  const char** argv,
                                  FastoObject* out) {
  UNUSED(argc);

  NKey key(argv[0]);
  int start = common::ConvertFromString<int>(argv[1]);
  int stop = common::ConvertFromString<int>(argv[2]);
  bool ws = argc == 4 && strncmp(argv[3], "WITHSCORES", 10) == 0;
  DBConnection* redis = static_cast<DBConnection*>(handler);
  NDbKValue key_loaded;
  common::Error err = redis->Zrange(key, start, stop, ws, &key_loaded);
  if (err && err->isError()) {
    return err;
  }

  NValue val = key_loaded.Value();
  common::Value* copy = val->deepCopy();
  FastoObject* child = new FastoObject(out, copy, redis->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Hmset(internal::CommandHandler* handler,
                                 int argc,
                                 const char** argv,
                                 FastoObject* out) {
  NKey key(argv[0]);
  common::HashValue* hmset = common::Value::createHashValue();
  for (int i = 1; i < argc; i += 2) {
    std::string key = argv[i];
    std::string val = argv[i + 1];
    hmset->insert(key, val);
  }

  DBConnection* redis = static_cast<DBConnection*>(handler);
  common::Error err = redis->Hmset(key, NValue(hmset));
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, redis->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Hgetall(internal::CommandHandler* handler,
                                   int argc,
                                   const char** argv,
                                   FastoObject* out) {
  UNUSED(argc);

  NKey key(argv[0]);
  DBConnection* redis = static_cast<DBConnection*>(handler);
  NDbKValue key_loaded;
  common::Error err = redis->Hgetall(key, &key_loaded);
  if (err && err->isError()) {
    return err;
  }

  NValue val = key_loaded.Value();
  common::Value* copy = val->deepCopy();
  FastoObject* child = new FastoObject(out, copy, redis->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Persist(internal::CommandHandler* handler,
                                   int argc,
                                   const char** argv,
                                   FastoObject* out) {
  UNUSED(argc);
  NKey key(argv[0]);

  DBConnection* red = static_cast<DBConnection*>(handler);
  common::Error err = red->SetTTL(key, NO_TTL);
  if (err && err->isError()) {
    common::FundamentalValue* val = common::Value::createUIntegerValue(0);
    FastoObject* child = new FastoObject(out, val, red->Delimiter());
    out->AddChildren(child);
    return err;
  }

  common::FundamentalValue* val = common::Value::createUIntegerValue(1);
  FastoObject* child = new FastoObject(out, val, red->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::ExpireRedis(internal::CommandHandler* handler,
                                       int argc,
                                       const char** argv,
                                       FastoObject* out) {
  UNUSED(argc);
  NKey key(argv[0]);
  ttl_t ttl = common::ConvertFromString<ttl_t>(argv[1]);

  DBConnection* red = static_cast<DBConnection*>(handler);
  common::Error err = red->SetTTL(key, ttl);
  if (err && err->isError()) {
    common::FundamentalValue* val = common::Value::createUIntegerValue(0);
    FastoObject* child = new FastoObject(out, val, red->Delimiter());
    out->AddChildren(child);
    return err;
  }

  common::FundamentalValue* val = common::Value::createUIntegerValue(1);
  FastoObject* child = new FastoObject(out, val, red->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error CommandsApi::Monitor(internal::CommandHandler* handler,
                                   int argc,
                                   const char** argv,
                                   FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->Monitor(argc + 1, argv - 1, out);
}

common::Error CommandsApi::Subscribe(internal::CommandHandler* handler,
                                     int argc,
                                     const char** argv,
                                     FastoObject* out) {
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->Subscribe(argc + 1, argv - 1, out);
}

common::Error CommandsApi::Sync(internal::CommandHandler* handler,
                                int argc,
                                const char** argv,
                                FastoObject* out) {
  UNUSED(argc);
  UNUSED(argv);
  DBConnection* red = static_cast<DBConnection*>(handler);
  return red->SlaveMode(out);
}

}  // namespace redis
}  // namespace core
}  // namespace fastonosql
