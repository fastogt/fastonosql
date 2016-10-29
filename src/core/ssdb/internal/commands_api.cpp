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

#include "core/ssdb/internal/commands_api.h"

#include <common/convert2string.h>

#include "core/db_key.h"

#include "core/ssdb/db_connection.h"

#include "global/global.h"

namespace fastonosql {
namespace core {
namespace ssdb {

common::Error info(internal::CommandHandler* handler,
                   int argc,
                   const char** argv,
                   FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  ServerInfo::Stats statsout;
  common::Error er = ssdb->Info(argc == 1 ? argv[0] : 0, &statsout);
  if (!er) {
    ServerInfo sinf(statsout);
    common::StringValue* val = common::Value::createStringValue(sinf.ToString());
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error dbsize(internal::CommandHandler* handler,
                     int argc,
                     const char** argv,
                     FastoObject* out) {
  UNUSED(argc);
  UNUSED(argv);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t dbsize = 0;
  common::Error er = ssdb->DBsize(&dbsize);
  if (!er) {
    common::FundamentalValue* val = common::Value::createUIntegerValue(dbsize);
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error auth(internal::CommandHandler* handler,
                   int argc,
                   const char** argv,
                   FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  common::Error er = ssdb->Auth(argv[0]);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error get(internal::CommandHandler* handler,
                  int argc,
                  const char** argv,
                  FastoObject* out) {
  UNUSED(argc);

  NKey key(argv[0]);
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  NDbKValue key_loaded;
  common::Error err = ssdb->Get(key, &key_loaded);
  if (err && err->isError()) {
    return err;
  }

  NValue val = key_loaded.Value();
  common::Value* copy = val->deepCopy();
  FastoObject* child = new FastoObject(out, copy, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error select(internal::CommandHandler* handler,
                     int argc,
                     const char** argv,
                     FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  common::Error err = ssdb->Select(argv[0], nullptr);
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error set(internal::CommandHandler* handler,
                  int argc,
                  const char** argv,
                  FastoObject* out) {
  UNUSED(argc);

  NKey key(argv[0]);
  NValue string_val(common::Value::createStringValue(argv[1]));
  NDbKValue kv(key, string_val);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  NDbKValue key_added;
  common::Error err = ssdb->Set(kv, &key_added);
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error setx(internal::CommandHandler* handler,
                   int argc,
                   const char** argv,
                   FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  common::Error er = ssdb->Setx(argv[0], argv[1], common::ConvertFromString<int>(argv[2]));
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error del(internal::CommandHandler* handler,
                  int argc,
                  const char** argv,
                  FastoObject* out) {
  NKeys keysdel;
  for (int i = 0; i < argc; ++i) {
    keysdel.push_back(NKey(argv[i]));
  }

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  NKeys keys_deleted;
  common::Error err = ssdb->Delete(keysdel, &keys_deleted);
  if (err && err->isError()) {
    return err;
  }

  common::FundamentalValue* val = common::Value::createUIntegerValue(keys_deleted.size());
  FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error rename(internal::CommandHandler* handler,
                     int argc,
                     const char** argv,
                     FastoObject* out) {
  UNUSED(argc);

  NKey key(argv[0]);
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  common::Error err = ssdb->Rename(key, argv[1]);
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error set_ttl(internal::CommandHandler* handler,
                      int argc,
                      const char** argv,
                      FastoObject* out) {
  UNUSED(out);
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  NKey key(argv[0]);
  ttl_t ttl = common::ConvertFromString<ttl_t>(argv[1]);
  common::Error er = ssdb->SetTTL(key, ttl);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error incr(internal::CommandHandler* handler,
                   int argc,
                   const char** argv,
                   FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t ret = 0;
  common::Error er = ssdb->Incr(argv[0], common::ConvertFromString<int64_t>(argv[1]), &ret);
  if (!er) {
    common::FundamentalValue* val = common::Value::createIntegerValue(ret);
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error keys(internal::CommandHandler* handler,
                   int argc,
                   const char** argv,
                   FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysout;
  common::Error er =
      ssdb->Keys(argv[0], argv[1], common::ConvertFromString<uint64_t>(argv[2]), &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error scan(internal::CommandHandler* handler,
                   int argc,
                   const char** argv,
                   FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysout;
  common::Error er =
      ssdb->Scan(argv[0], argv[1], common::ConvertFromString<uint64_t>(argv[2]), &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error rscan(internal::CommandHandler* handler,
                    int argc,
                    const char** argv,
                    FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysout;
  common::Error er =
      ssdb->Rscan(argv[0], argv[1], common::ConvertFromString<uint64_t>(argv[2]), &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error multi_get(internal::CommandHandler* handler,
                        int argc,
                        const char** argv,
                        FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysget;
  for (int i = 0; i < argc; ++i) {
    keysget.push_back(argv[i]);
  }

  std::vector<std::string> keysout;
  common::Error er = ssdb->MultiGet(keysget, &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error multi_set(internal::CommandHandler* handler,
                        int argc,
                        const char** argv,
                        FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::map<std::string, std::string> keysset;
  for (int i = 0; i < argc; i += 2) {
    keysset[argv[i]] = argv[i + 1];
  }

  common::Error er = ssdb->MultiSet(keysset);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error multi_del(internal::CommandHandler* handler,
                        int argc,
                        const char** argv,
                        FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysget;
  for (int i = 0; i < argc; ++i) {
    keysget.push_back(argv[i]);
  }

  common::Error er = ssdb->MultiDel(keysget);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error hget(internal::CommandHandler* handler,
                   int argc,
                   const char** argv,
                   FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::string ret;
  common::Error er = ssdb->Hget(argv[0], argv[1], &ret);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue(ret);
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error hgetall(internal::CommandHandler* handler,
                      int argc,
                      const char** argv,
                      FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysout;
  common::Error er = ssdb->Hgetall(argv[0], &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error hset(internal::CommandHandler* handler,
                   int argc,
                   const char** argv,
                   FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  common::Error er = ssdb->Hset(argv[0], argv[1], argv[2]);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error hdel(internal::CommandHandler* handler,
                   int argc,
                   const char** argv,
                   FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  common::Error er = ssdb->Hdel(argv[0], argv[1]);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error hincr(internal::CommandHandler* handler,
                    int argc,
                    const char** argv,
                    FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t res = 0;
  common::Error er =
      ssdb->Hincr(argv[0], argv[1], common::ConvertFromString<int64_t>(argv[2]), &res);
  if (!er) {
    common::FundamentalValue* val = common::Value::createIntegerValue(res);
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error hsize(internal::CommandHandler* handler,
                    int argc,
                    const char** argv,
                    FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t res = 0;
  common::Error er = ssdb->Hsize(argv[0], &res);
  if (!er) {
    common::FundamentalValue* val = common::Value::createIntegerValue(res);
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error hclear(internal::CommandHandler* handler,
                     int argc,
                     const char** argv,
                     FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t res = 0;
  common::Error er = ssdb->Hclear(argv[0], &res);
  if (!er) {
    common::FundamentalValue* val = common::Value::createIntegerValue(res);
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error hkeys(internal::CommandHandler* handler,
                    int argc,
                    const char** argv,
                    FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysout;
  common::Error er = ssdb->Hkeys(argv[0], argv[1], argv[2],
                                 common::ConvertFromString<uint64_t>(argv[3]), &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error hscan(internal::CommandHandler* handler,
                    int argc,
                    const char** argv,
                    FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysout;
  common::Error er = ssdb->Hscan(argv[0], argv[1], argv[2],
                                 common::ConvertFromString<uint64_t>(argv[3]), &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error hrscan(internal::CommandHandler* handler,
                     int argc,
                     const char** argv,
                     FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysout;
  common::Error er = ssdb->Hrscan(argv[0], argv[1], argv[2],
                                  common::ConvertFromString<uint64_t>(argv[3]), &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error multi_hget(internal::CommandHandler* handler,
                         int argc,
                         const char** argv,
                         FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysget;
  for (int i = 1; i < argc; ++i) {
    keysget.push_back(argv[i]);
  }

  std::vector<std::string> keysout;
  common::Error er = ssdb->MultiHget(argv[0], keysget, &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error multi_hset(internal::CommandHandler* handler,
                         int argc,
                         const char** argv,
                         FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::map<std::string, std::string> keys;
  for (int i = 1; i < argc; i += 2) {
    keys[argv[i]] = argv[i + 1];
  }

  common::Error er = ssdb->MultiHset(argv[0], keys);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error zget(internal::CommandHandler* handler,
                   int argc,
                   const char** argv,
                   FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t ret;
  common::Error er = ssdb->Zget(argv[0], argv[1], &ret);
  if (!er) {
    common::FundamentalValue* val = common::Value::createIntegerValue(ret);
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error zset(internal::CommandHandler* handler,
                   int argc,
                   const char** argv,
                   FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  common::Error er = ssdb->Zset(argv[0], argv[1], common::ConvertFromString<int64_t>(argv[2]));
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error zdel(internal::CommandHandler* handler,
                   int argc,
                   const char** argv,
                   FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  common::Error er = ssdb->Zdel(argv[0], argv[1]);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }
  return er;
}

common::Error zincr(internal::CommandHandler* handler,
                    int argc,
                    const char** argv,
                    FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t ret = 0;
  common::Error er =
      ssdb->Zincr(argv[0], argv[1], common::ConvertFromString<int64_t>(argv[2]), &ret);
  if (!er) {
    common::FundamentalValue* val = common::Value::createIntegerValue(ret);
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error zsize(internal::CommandHandler* handler,
                    int argc,
                    const char** argv,
                    FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t res = 0;
  common::Error er = ssdb->Zsize(argv[0], &res);
  if (!er) {
    common::FundamentalValue* val = common::Value::createIntegerValue(res);
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error zclear(internal::CommandHandler* handler,
                     int argc,
                     const char** argv,
                     FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t res = 0;
  common::Error er = ssdb->Zclear(argv[0], &res);
  if (!er) {
    common::FundamentalValue* val = common::Value::createIntegerValue(res);
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error zrank(internal::CommandHandler* handler,
                    int argc,
                    const char** argv,
                    FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t res = 0;
  common::Error er = ssdb->Zrank(argv[0], argv[1], &res);
  if (!er) {
    common::FundamentalValue* val = common::Value::createIntegerValue(res);
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error zrrank(internal::CommandHandler* handler,
                     int argc,
                     const char** argv,
                     FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t res = 0;
  common::Error er = ssdb->Zrrank(argv[0], argv[1], &res);
  if (!er) {
    common::FundamentalValue* val = common::Value::createIntegerValue(res);
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error zrange(internal::CommandHandler* handler,
                     int argc,
                     const char** argv,
                     FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> res;
  common::Error er = ssdb->Zrange(argv[0], common::ConvertFromString<uint64_t>(argv[1]),
                                  common::ConvertFromString<uint64_t>(argv[2]), &res);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < res.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(res[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error zrrange(internal::CommandHandler* handler,
                      int argc,
                      const char** argv,
                      FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> res;
  common::Error er = ssdb->Zrrange(argv[0], common::ConvertFromString<uint64_t>(argv[1]),
                                   common::ConvertFromString<uint64_t>(argv[2]), &res);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < res.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(res[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error zkeys(internal::CommandHandler* handler,
                    int argc,
                    const char** argv,
                    FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> res;
  int64_t st = common::ConvertFromString<int64_t>(argv[2]);
  int64_t end = common::ConvertFromString<int64_t>(argv[3]);
  common::Error er =
      ssdb->Zkeys(argv[0], argv[1], &st, &end, common::ConvertFromString<uint64_t>(argv[5]), &res);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < res.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(res[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error zscan(internal::CommandHandler* handler,
                    int argc,
                    const char** argv,
                    FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> res;
  int64_t st = common::ConvertFromString<int64_t>(argv[2]);
  int64_t end = common::ConvertFromString<int64_t>(argv[3]);
  common::Error er =
      ssdb->Zscan(argv[0], argv[1], &st, &end, common::ConvertFromString<uint64_t>(argv[4]), &res);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < res.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(res[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error zrscan(internal::CommandHandler* handler,
                     int argc,
                     const char** argv,
                     FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> res;
  int64_t st = common::ConvertFromString<int64_t>(argv[2]);
  int64_t end = common::ConvertFromString<int64_t>(argv[3]);
  common::Error er =
      ssdb->Zrscan(argv[0], argv[1], &st, &end, common::ConvertFromString<uint64_t>(argv[4]), &res);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < res.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(res[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error multi_zget(internal::CommandHandler* handler,
                         int argc,
                         const char** argv,
                         FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysget;
  for (int i = 1; i < argc; ++i) {
    keysget.push_back(argv[i]);
  }

  std::vector<std::string> res;
  common::Error er = ssdb->MultiZget(argv[0], keysget, &res);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < res.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(res[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error multi_zset(internal::CommandHandler* handler,
                         int argc,
                         const char** argv,
                         FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::map<std::string, int64_t> keysget;
  for (int i = 1; i < argc; i += 2) {
    keysget[argv[i]] = common::ConvertFromString<int64_t>(argv[i + 1]);
  }

  common::Error er = ssdb->MultiZset(argv[0], keysget);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error multi_zdel(internal::CommandHandler* handler,
                         int argc,
                         const char** argv,
                         FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysget;
  for (int i = 1; i < argc; ++i) {
    keysget.push_back(argv[i]);
  }

  common::Error er = ssdb->MultiZdel(argv[0], keysget);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error qpush(internal::CommandHandler* handler,
                    int argc,
                    const char** argv,
                    FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  common::Error er = ssdb->Qpush(argv[0], argv[1]);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error qpop(internal::CommandHandler* handler,
                   int argc,
                   const char** argv,
                   FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::string ret;
  common::Error er = ssdb->Qpop(argv[0], &ret);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue(ret);
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error qslice(internal::CommandHandler* handler,
                     int argc,
                     const char** argv,
                     FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t begin = common::ConvertFromString<int64_t>(argv[1]);
  int64_t end = common::ConvertFromString<int64_t>(argv[2]);

  std::vector<std::string> keysout;
  common::Error er = ssdb->Qslice(argv[0], begin, end, &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error qclear(internal::CommandHandler* handler,
                     int argc,
                     const char** argv,
                     FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t res = 0;
  common::Error er = ssdb->Qclear(argv[0], &res);
  if (!er) {
    common::FundamentalValue* val = common::Value::createIntegerValue(res);
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error dbkcount(internal::CommandHandler* handler,
                       int argc,
                       const char** argv,
                       FastoObject* out) {
  UNUSED(argc);
  UNUSED(argv);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  size_t dbkcount = 0;
  common::Error er = ssdb->DBkcount(&dbkcount);
  if (!er) {
    common::FundamentalValue* val = common::Value::createUIntegerValue(dbkcount);
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error help(internal::CommandHandler* handler,
                   int argc,
                   const char** argv,
                   FastoObject* out) {
  UNUSED(out);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  return ssdb->Help(argc - 1, argv + 1);
}

common::Error flushdb(internal::CommandHandler* handler,
                      int argc,
                      const char** argv,
                      FastoObject* out) {
  UNUSED(argc);
  UNUSED(argv);
  UNUSED(out);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  return ssdb->Flushdb();
}

}  // namespace ssdb
}  // namespace core
}  // namespace fastonosql
