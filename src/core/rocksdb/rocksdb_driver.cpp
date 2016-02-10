/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    SiteOnYourDevice is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SiteOnYourDevice is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SiteOnYourDevice.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "core/rocksdb/rocksdb_driver.h"

#include <vector>
#include <string>

#include <rocksdb/db.h>

#include "common/sprintf.h"
#include "common/utils.h"
#include "fasto/qt/logger.h"

#include "core/command_logger.h"

#include "core/rocksdb/rocksdb_config.h"
#include "core/rocksdb/rocksdb_infos.h"

#define INFO_REQUEST "INFO"
#define GET_KEY_PATTERN_1ARGS_S "GET %s"
#define SET_KEY_PATTERN_2ARGS_SS "PUT %s %s"

#define GET_KEYS_PATTERN_1ARGS_I "KEYS a z %d"
#define DELETE_KEY_PATTERN_1ARGS_S "DEL %s"
#define GET_SERVER_TYPE ""
#define ROCKSDB_HEADER_STATS    "\n** Compaction Stats [default] **\n"\
                                "Level    Files   Size(MB) Score Read(GB)  Rn(GB) Rnp1(GB) "\
                                "Write(GB) Wnew(GB) Moved(GB) W-Amp Rd(MB/s) Wr(MB/s) "\
                                "Comp(sec) Comp(cnt) Avg(sec) "\
                                "Stall(cnt)  KeyIn KeyDrop\n"\
                                "--------------------------------------------------------------------"\
                                "-----------------------------------------------------------"\
                                "--------------------------------------\n"
namespace fastonosql {
namespace {

common::Error createConnection(const rocksdbConfig& config, rocksdb::DB** context) {
  DCHECK(*context == NULL);

  rocksdb::DB* lcontext = NULL;
  rocksdb::Status st = rocksdb::DB::Open(config.options, config.dbname, &lcontext);
  if (!st.ok()) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "Fail open database: %s!", st.ToString());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  *context = lcontext;
  return common::Error();
}

common::Error createConnection(RocksdbConnectionSettings* settings, rocksdb::DB** context) {
  if (!settings) {
    return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
  }

  rocksdbConfig config = settings->info();
  return createConnection(config, context);
}

}  // namesapce

common::Error testConnection(RocksdbConnectionSettings* settings) {
  rocksdb::DB* ldb = NULL;
  common::Error er = createConnection(settings, &ldb);
  if (er && er->isError()) {
    return er;
  }

  delete ldb;
  return common::Error();
}

struct RocksdbDriver::pimpl {
  pimpl()
    : rocksdb_(NULL) {
  }

  bool isConnected() const {
    if (!rocksdb_) {
      return false;
    }

    return true;
  }

  common::Error connect() {
    if (isConnected()) {
      return common::Error();
    }

    clear();
    init();

    rocksdb::DB* context = NULL;
    common::Error er = createConnection(config_, &context);
    if (er && er->isError()) {
      return er;
    }

    rocksdb_ = context;
    return common::Error();
  }

  common::Error disconnect() {
    if (!isConnected()) {
      return common::Error();
    }

    clear();
    return common::Error();
  }

  common::Error info(const char* args, RocksdbServerInfo::Stats& statsout) {
    // sstables
    // stats
    // char prop[1024] = {0};
    // common::SNPrintf(prop, sizeof(prop), "rocksdb.%s", args ? args : "stats");

    std::string rets;
    bool isok = rocksdb_->GetProperty("rocksdb.stats", &rets);
    if (!isok) {
      return common::make_error_value("info function failed", common::ErrorValue::E_ERROR);
    }

    if (rets.size() > sizeof(ROCKSDB_HEADER_STATS)) {
      const char * retsc = rets.c_str() + sizeof(ROCKSDB_HEADER_STATS);
      char* p2 = strtok((char*)retsc, " ");
      int pos = 0;
      while (p2) {
        switch (pos++) {
          case 0:
            statsout.compactions_level = atoi(p2);
            break;
          case 1:
            statsout.file_size_mb = atoi(p2);
            break;
          case 2:
            statsout.time_sec = atoi(p2);
            break;
          case 3:
            statsout.read_mb = atoi(p2);
            break;
          case 4:
            statsout.write_mb = atoi(p2);
            break;
          default:
            break;
          }
          p2 = strtok(0, " ");
        }
    }

    return common::Error();
  }

  common::Error dbsize(size_t& size) WARN_UNUSED_RESULT {
    rocksdb::ReadOptions ro;
    rocksdb::Iterator* it = rocksdb_->NewIterator(ro);
    size_t sz = 0;
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
      sz++;
    }

    rocksdb::Status st = it->status();
    delete it;

    if (!st.ok()) {
      char buff[1024] = {0};
      common::SNPrintf(buff, sizeof(buff), "Couldn't determine DBSIZE error: %s", st.ToString());
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }
    size = sz;
    return common::Error();
  }

  ~pimpl() {
    clear();
  }

  rocksdbConfig config_;

  std::string currentDbName() const {
    rocksdb::ColumnFamilyHandle* fam = rocksdb_->DefaultColumnFamily();
    if (fam) {
      return fam->GetName();
    }

    return "default";
  }

  common::Error execute_impl(int argc, char **argv, FastoObject* out) {
    if (strcasecmp(argv[0], "info") == 0) {
      if (argc > 2) {
        return common::make_error_value("Invalid info input argument", common::ErrorValue::E_ERROR);
      }

      RocksdbServerInfo::Stats statsout;
      common::Error er = info(argc == 2 ? argv[1] : 0, statsout);
      if (!er) {
        common::StringValue *val = common::Value::createStringValue(RocksdbServerInfo(statsout).toString());
        FastoObject* child = new FastoObject(out, val, config_.delimiter);
        out->addChildren(child);
      }
      return er;
    } else if (strcasecmp(argv[0], "get") == 0) {
      if (argc != 2) {
        return common::make_error_value("Invalid get input argument", common::ErrorValue::E_ERROR);
      }

      std::string ret;
      common::Error er = get(argv[1], &ret);
      if (!er) {
        common::StringValue *val = common::Value::createStringValue(ret);
        FastoObject* child = new FastoObject(out, val, config_.delimiter);
        out->addChildren(child);
      }
      return er;
    } else if (strcasecmp(argv[0], "dbsize") == 0) {
      if (argc != 1) {
        return common::make_error_value("Invalid dbsize input argument", common::ErrorValue::E_ERROR);
      }

      size_t ret = 0;
      common::Error er = dbsize(ret);
      if (!er) {
        common::FundamentalValue *val = common::Value::createUIntegerValue(ret);
        FastoObject* child = new FastoObject(out, val, config_.delimiter);
        out->addChildren(child);
      }
      return er;
    } else if (strcasecmp(argv[0], "mget") == 0) {
      if (argc < 2) {
        return common::make_error_value("Invalid mget input argument", common::ErrorValue::E_ERROR);
      }

      std::vector<rocksdb::Slice> keysget;
      for (size_t i = 1; i < argc; ++i) {
        keysget.push_back(argv[i]);
      }

      std::vector<std::string> keysout;
      common::Error er = mget(keysget, &keysout);
      if (!er) {
        common::ArrayValue* ar = common::Value::createArrayValue();
        for (size_t i = 0; i < keysout.size(); ++i) {
          common::StringValue *val = common::Value::createStringValue(keysout[i]);
          ar->append(val);
        }
        FastoObjectArray* child = new FastoObjectArray(out, ar, config_.delimiter);
        out->addChildren(child);
      }
      return er;
    } else if (strcasecmp(argv[0], "merge") == 0) {
      if (argc != 3) {
        return common::make_error_value("Invalid merge input argument", common::ErrorValue::E_ERROR);
      }

      common::Error er = merge(argv[1], argv[2]);
      if (!er) {
        common::StringValue *val = common::Value::createStringValue("STORED");
        FastoObject* child = new FastoObject(out, val, config_.delimiter);
        out->addChildren(child);
      }
      return er;
    } else if (strcasecmp(argv[0], "put") == 0) {
      if (argc != 3) {
        return common::make_error_value("Invalid put input argument", common::ErrorValue::E_ERROR);
      }

      common::Error er = put(argv[1], argv[2]);
      if (!er) {
        common::StringValue *val = common::Value::createStringValue("STORED");
        FastoObject* child = new FastoObject(out, val, config_.delimiter);
        out->addChildren(child);
      }
      return er;
    } else if (strcasecmp(argv[0], "del") == 0) {
      if (argc != 2) {
        return common::make_error_value("Invalid del input argument", common::ErrorValue::E_ERROR);
      }

      common::Error er = del(argv[1]);
      if (!er) {
        common::StringValue *val = common::Value::createStringValue("DELETED");
        FastoObject* child = new FastoObject(out, val, config_.delimiter);
        out->addChildren(child);
      }
      return er;
    } else if (strcasecmp(argv[0], "keys") == 0) {
      if (argc != 4) {
        return common::make_error_value("Invalid keys input argument", common::ErrorValue::E_ERROR);
      }

      std::vector<std::string> keysout;
      common::Error er = keys(argv[1], argv[2], atoll(argv[3]), &keysout);
      if (!er) {
        common::ArrayValue* ar = common::Value::createArrayValue();
        for (size_t i = 0; i < keysout.size(); ++i) {
          common::StringValue *val = common::Value::createStringValue(keysout[i]);
          ar->append(val);
        }
        FastoObjectArray* child = new FastoObjectArray(out, ar, config_.delimiter);
        out->addChildren(child);
      }
      return er;
    } else {
      char buff[1024] = {0};
      common::SNPrintf(buff, sizeof(buff), "Not supported command: %s", argv[0]);
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }
  }

 private:
  common::Error get(const std::string& key, std::string* ret_val) {
    rocksdb::ReadOptions ro;
    rocksdb::Status st = rocksdb_->Get(ro, key, ret_val);
    if (!st.ok()) {
      char buff[1024] = {0};
      common::SNPrintf(buff, sizeof(buff), "get function error: %s", st.ToString());
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }

    return common::Error();
  }

  common::Error mget(const std::vector<rocksdb::Slice>& keys, std::vector<std::string> *ret) {
    rocksdb::ReadOptions ro;
    std::vector<rocksdb::Status> sts = rocksdb_->MultiGet(ro, keys, ret);
    for (size_t i = 0; i < sts.size(); ++i) {
      rocksdb::Status st = sts[i];
      if (st.ok()) {
        return common::Error();
      }
    }

    return common::make_error_value("mget function unknown error", common::ErrorValue::E_ERROR);
  }

  common::Error merge(const std::string& key, const std::string& value) {
    rocksdb::WriteOptions wo;
    rocksdb::Status st = rocksdb_->Merge(wo, key, value);
    if (!st.ok()) {
      char buff[1024] = {0};
      common::SNPrintf(buff, sizeof(buff), "merge function error: %s", st.ToString());
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }

    return common::Error();
  }

  common::Error put(const std::string& key, const std::string& value) {
    rocksdb::WriteOptions wo;
    rocksdb::Status st = rocksdb_->Put(wo, key, value);
    if (!st.ok()) {
      char buff[1024] = {0};
      common::SNPrintf(buff, sizeof(buff), "put function error: %s", st.ToString());
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }

    return common::Error();
  }

  common::Error del(const std::string& key) {
    rocksdb::WriteOptions wo;
    rocksdb::Status st = rocksdb_->Delete(wo, key);
    if (!st.ok()) {
      char buff[1024] = {0};
      common::SNPrintf(buff, sizeof(buff), "del function error: %s", st.ToString());
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }
    return common::Error();
  }

  common::Error keys(const std::string &key_start, const std::string &key_end,
                     uint64_t limit, std::vector<std::string> *ret) {
    ret->clear();

    rocksdb::ReadOptions ro;
    rocksdb::Iterator* it = rocksdb_->NewIterator(ro);  // keys(key_start, key_end, limit, ret);
    for (it->Seek(key_start); it->Valid() && it->key().ToString() < key_end; it->Next()) {
      std::string key = it->key().ToString();
      if (ret->size() <= limit) {
        ret->push_back(key);
      } else {
        break;
      }
    }

    rocksdb::Status st = it->status();
    delete it;

    if (!st.ok()) {
      char buff[1024] = {0};
      common::SNPrintf(buff, sizeof(buff), "Keys function error: %s", st.ToString());
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }
    return common::Error();
  }

  void init() {
  }

  void clear() {
    delete rocksdb_;
    rocksdb_ = NULL;
  }

  rocksdb::DB* rocksdb_;
};

RocksdbDriver::RocksdbDriver(IConnectionSettingsBaseSPtr settings)
  : IDriver(settings, ROCKSDB), impl_(new pimpl) {
}

RocksdbDriver::~RocksdbDriver() {
  interrupt();
  stop();
  delete impl_;
}

bool RocksdbDriver::isConnected() const {
  return impl_->isConnected();
}

bool RocksdbDriver::isAuthenticated() const {
  return impl_->isConnected();
}

// ============== commands =============//
common::Error RocksdbDriver::commandDeleteImpl(CommandDeleteKey* command,
                                               std::string* cmdstring) const {
  if (!command || !cmdstring) {
    return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
  }

  char patternResult[1024] = {0};
  NDbKValue key = command->key();
  common::SNPrintf(patternResult, sizeof(patternResult),
                   DELETE_KEY_PATTERN_1ARGS_S, key.keyString());

  *cmdstring = patternResult;
  return common::Error();
}

common::Error RocksdbDriver::commandLoadImpl(CommandLoadKey* command,
                                             std::string* cmdstring) const {
  if (!command || !cmdstring) {
    return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
  }

  char patternResult[1024] = {0};
  NDbKValue key = command->key();
  common::SNPrintf(patternResult, sizeof(patternResult), GET_KEY_PATTERN_1ARGS_S, key.keyString());

  *cmdstring = patternResult;
  return common::Error();
}

common::Error RocksdbDriver::commandCreateImpl(CommandCreateKey* command,
                                               std::string* cmdstring) const {
  if (!command || !cmdstring) {
    return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
  }

  char patternResult[1024] = {0};
  NDbKValue key = command->key();
  NValue val = command->value();
  common::Value* rval = val.get();
  std::string key_str = key.keyString();
  std::string value_str = common::convertToString(rval, " ");
  common::SNPrintf(patternResult, sizeof(patternResult), SET_KEY_PATTERN_2ARGS_SS,
                   key_str, value_str);

  *cmdstring = patternResult;
  return common::Error();
}

common::Error RocksdbDriver::commandChangeTTLImpl(CommandChangeTTL* command,
                                                  std::string* cmdstring) const {
  if (!command || !cmdstring) {
    return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
  }

  char errorMsg[1024] = {0};
  common::SNPrintf(errorMsg, sizeof(errorMsg), "Sorry, but now " PROJECT_NAME_TITLE " not supported change ttl command for %s.", common::convertToString(connectionType()));
  return common::make_error_value(errorMsg, common::ErrorValue::E_ERROR);
}

// ============== commands =============//

common::net::hostAndPort RocksdbDriver::address() const {
  return common::net::hostAndPort();
}

std::string RocksdbDriver::outputDelemitr() const {
  return impl_->config_.delimiter;
}

const char* RocksdbDriver::versionApi() {
  return STRINGIZE(ROCKSDB_MAJOR) "." STRINGIZE(ROCKSDB_MINOR) "." STRINGIZE(ROCKSDB_PATCH);
}

void RocksdbDriver::initImpl() {
}

void RocksdbDriver::clearImpl() {
}

common::Error RocksdbDriver::executeImpl(int argc, char **argv, FastoObject* out) {
  return impl_->execute_impl(argc, argv, out);
}

common::Error RocksdbDriver::serverInfo(ServerInfo **info) {
  LOG_COMMAND(Command(INFO_REQUEST, common::Value::C_INNER));
  RocksdbServerInfo::Stats cm;
  common::Error err = impl_->info(NULL, cm);
  if (!err) {
    *info = new RocksdbServerInfo(cm);
  }

  return err;
}

common::Error RocksdbDriver::serverDiscoveryInfo(ServerInfo **sinfo,
                                                 ServerDiscoveryInfo **dinfo,
                                                 DataBaseInfo** dbinfo) {
  ServerInfo *lsinfo = NULL;
  common::Error er = serverInfo(&lsinfo);
  if (er && er->isError()) {
      return er;
  }

  FastoObjectIPtr root = FastoObject::createRoot(GET_SERVER_TYPE);
  FastoObjectCommand* cmd = createCommand<RocksdbCommand>(root,
                                                          GET_SERVER_TYPE, common::Value::C_INNER);
  er = execute(cmd);

  if (!er) {
    FastoObject::child_container_type ch = root->childrens();
    if (ch.size()) {
      // *dinfo = makeOwnRedisDiscoveryInfo(ch[0]);
    }
  }

  DataBaseInfo* ldbinfo = NULL;
  er = currentDataBaseInfo(&ldbinfo);
  if (er && er->isError()) {
    delete lsinfo;
    return er;
  }

  *sinfo = lsinfo;
  *dbinfo = ldbinfo;
  return er;
}

common::Error RocksdbDriver::currentDataBaseInfo(DataBaseInfo** info) {
  std::string name = impl_->currentDbName();
  size_t size = 0;
  impl_->dbsize(size);
  *info = new RocksdbDataBaseInfo(name, true, size);
  return common::Error();
}

void RocksdbDriver::handleConnectEvent(events::ConnectRequestEvent *ev) {
  QObject *sender = ev->sender();
  notifyProgress(sender, 0);
      events::ConnectResponceEvent::value_type res(ev->value());
      RocksdbConnectionSettings *set = dynamic_cast<RocksdbConnectionSettings*>(settings_.get());
      if (set) {
        impl_->config_ = set->info();
  notifyProgress(sender, 25);
        common::Error er = impl_->connect();
        if (er && er->isError()) {
          res.setErrorInfo(er);
        }
  notifyProgress(sender, 75);
      }
      reply(sender, new events::ConnectResponceEvent(this, res));
  notifyProgress(sender, 100);
}

void RocksdbDriver::handleDisconnectEvent(events::DisconnectRequestEvent* ev) {
  QObject *sender = ev->sender();
  notifyProgress(sender, 0);
      events::DisconnectResponceEvent::value_type res(ev->value());
  notifyProgress(sender, 50);

      common::Error er = impl_->disconnect();
      if (er && er->isError()) {
          res.setErrorInfo(er);
      }

      reply(sender, new events::DisconnectResponceEvent(this, res));
  notifyProgress(sender, 100);
}

void RocksdbDriver::handleExecuteEvent(events::ExecuteRequestEvent* ev) {
  QObject *sender = ev->sender();
  notifyProgress(sender, 0);
      events::ExecuteRequestEvent::value_type res(ev->value());
      const char *inputLine = common::utils::c_strornull(res.text);

      common::Error er;
      if (inputLine) {
          size_t length = strlen(inputLine);
          int offset = 0;
          RootLocker lock = make_locker(sender, inputLine);
          FastoObjectIPtr outRoot = lock.root_;
          double step = 100.0f/length;
          for (size_t n = 0; n < length; ++n) {
              if (interrupt_) {
                  er.reset(new common::ErrorValue("Interrupted exec.",
                                                  common::ErrorValue::E_INTERRUPTED));
                  res.setErrorInfo(er);
                  break;
              }
              if (inputLine[n] == '\n' || n == length-1) {
                  notifyProgress(sender, step * n);
                  char command[128] = {0};
                  if (n == length-1) {
                    strcpy(command, inputLine + offset);
                  } else {
                    strncpy(command, inputLine + offset, n - offset);
                  }
                  offset = n + 1;
                  FastoObjectCommand* cmd = createCommand<RocksdbCommand>(outRoot,
                                                                          stableCommand(command),
                                                                          common::Value::C_USER);
                  er = execute(cmd);
                  if (er && er->isError()) {
                      res.setErrorInfo(er);
                      break;
                  }
              }
          }
      } else {
          er.reset(new common::ErrorValue("Empty command line.", common::ErrorValue::E_ERROR));
      }

      if (er) {  // E_INTERRUPTED
        LOG_ERROR(er, true);
      }
  notifyProgress(sender, 100);
}

void RocksdbDriver::handleCommandRequestEvent(events::CommandRequestEvent* ev) {
  QObject *sender = ev->sender();
  notifyProgress(sender, 0);
      events::CommandResponceEvent::value_type res(ev->value());
      std::string cmdtext;
      common::Error er = commandByType(res.cmd, &cmdtext);
      if (er && er->isError()) {
          res.setErrorInfo(er);
          reply(sender, new events::CommandResponceEvent(this, res));
          notifyProgress(sender, 100);
          return;
      }

      RootLocker lock = make_locker(sender, cmdtext);
      FastoObjectIPtr root = lock.root_;
      FastoObjectCommand* cmd = createCommand<RocksdbCommand>(root, cmdtext, common::Value::C_INNER);
  notifyProgress(sender, 50);
      er = execute(cmd);
      if (er && er->isError()) {
          res.setErrorInfo(er);
      }
      reply(sender, new events::CommandResponceEvent(this, res));
  notifyProgress(sender, 100);
}

void RocksdbDriver::handleLoadDatabaseInfosEvent(events::LoadDatabasesInfoRequestEvent* ev) {
  QObject *sender = ev->sender();
notifyProgress(sender, 0);
  events::LoadDatabasesInfoResponceEvent::value_type res(ev->value());
notifyProgress(sender, 50);
  res.databases.push_back(currentDatabaseInfo());
  reply(sender, new events::LoadDatabasesInfoResponceEvent(this, res));
notifyProgress(sender, 100);
}

void RocksdbDriver::handleLoadDatabaseContentEvent(events::LoadDatabaseContentRequestEvent *ev) {
  QObject *sender = ev->sender();
  notifyProgress(sender, 0);
    events::LoadDatabaseContentResponceEvent::value_type res(ev->value());
    char patternResult[1024] = {0};
    common::SNPrintf(patternResult, sizeof(patternResult), GET_KEYS_PATTERN_1ARGS_I, res.count_keys);
    FastoObjectIPtr root = FastoObject::createRoot(patternResult);
  notifyProgress(sender, 50);
    FastoObjectCommand* cmd = createCommand<RocksdbCommand>(root, patternResult,
                                                            common::Value::C_INNER);
    common::Error er = execute(cmd);
    if (er && er->isError()) {
        res.setErrorInfo(er);
    } else {
      FastoObject::child_container_type rchildrens = cmd->childrens();
      if (rchildrens.size()) {
          DCHECK_EQ(rchildrens.size(), 1);
          FastoObjectArray* array = dynamic_cast<FastoObjectArray*>(rchildrens[0]);
          if (!array) {
              goto done;
          }
          common::ArrayValue* ar = array->array();
          if (!ar) {
              goto done;
          }

          for (size_t i = 0; i < ar->size(); ++i) {
              std::string key;
              bool isok = ar->getString(i, &key);
              if (isok) {
                  NKey k(key);
                  NDbKValue ress(k, NValue());
                  res.keys.push_back(ress);
              }
          }
      }
    }
done:
  notifyProgress(sender, 75);
    reply(sender, new events::LoadDatabaseContentResponceEvent(this, res));
  notifyProgress(sender, 100);
}

void RocksdbDriver::handleSetDefaultDatabaseEvent(events::SetDefaultDatabaseRequestEvent* ev) {
  QObject *sender = ev->sender();
  notifyProgress(sender, 0);
      events::SetDefaultDatabaseResponceEvent::value_type res(ev->value());
  notifyProgress(sender, 50);
      reply(sender, new events::SetDefaultDatabaseResponceEvent(this, res));
  notifyProgress(sender, 100);
}

void RocksdbDriver::handleLoadServerInfoEvent(events::ServerInfoRequestEvent* ev) {
  QObject *sender = ev->sender();
  notifyProgress(sender, 0);
      events::ServerInfoResponceEvent::value_type res(ev->value());
  notifyProgress(sender, 50);
      LOG_COMMAND(Command(INFO_REQUEST, common::Value::C_INNER));
      RocksdbServerInfo::Stats cm;
      common::Error err = impl_->info(NULL, cm);
      if (err) {
          res.setErrorInfo(err);
      } else {
          ServerInfoSPtr mem(new RocksdbServerInfo(cm));
          res.setInfo(mem);
      }
  notifyProgress(sender, 75);
      reply(sender, new events::ServerInfoResponceEvent(this, res));
  notifyProgress(sender, 100);
}

void RocksdbDriver::handleProcessCommandLineArgs(events::ProcessConfigArgsRequestEvent* ev) {
}

ServerInfoSPtr RocksdbDriver::makeServerInfoFromString(const std::string& val) {
  ServerInfoSPtr res(makeRocksdbServerInfo(val));
  return res;
}

}  // namespace fastonosql
