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

#include "core/connection_settings.h"

#include <sstream>
#include <string>

#include "common/qt/convert_string.h"
#include "common/utils.h"
#include "common/net/net.h"
#include "common/logger.h"

#include "core/settings_manager.h"

#ifdef BUILD_WITH_REDIS
#include "core/redis/redis_settings.h"
#include "core/redis/redis_cluster_settings.h"
#endif
#ifdef BUILD_WITH_MEMCACHED
#include "core/memcached/memcached_settings.h"
#endif
#ifdef BUILD_WITH_SSDB
#include "core/ssdb/ssdb_settings.h"
#endif
#ifdef BUILD_WITH_LEVELDB
#include "core/leveldb/leveldb_settings.h"
#endif
#ifdef BUILD_WITH_ROCKSDB
#include "core/rocksdb/rocksdb_settings.h"
#endif
#ifdef BUILD_WITH_UNQLITE
#include "core/unqlite/unqlite_settings.h"
#endif
#ifdef BUILD_WITH_LMDB
#include "core/lmdb/lmdb_settings.h"
#endif

#define LOGGING_REDIS_FILE_EXTENSION ".red"
#define LOGGING_MEMCACHED_FILE_EXTENSION ".mem"
#define LOGGING_SSDB_FILE_EXTENSION ".ssdb"
#define LOGGING_LEVELDB_FILE_EXTENSION ".levdb"
#define LOGGING_ROCKSDB_FILE_EXTENSION ".rocksdb"
#define LOGGING_UNQLITE_FILE_EXTENSION ".unq"
#define LOGGING_LMDB_FILE_EXTENSION ".lmdb"

namespace {
  const char magicNumber = 0x1E;
}

namespace fastonosql {

IConnectionSettings::IConnectionSettings(const std::string& connectionName, connectionTypes type)
  : connection_name_(connectionName), type_(type), msinterval_(0) {
}

void IConnectionSettings::setName(const std::string& name) {
  connection_name_ = name;
}

IConnectionSettings::~IConnectionSettings() {
}

std::string IConnectionSettings::name() const {
  return connection_name_;
}

connectionTypes IConnectionSettings::type() const {
  return type_;
}

bool IConnectionSettings::loggingEnabled() const {
  return msinterval_ != 0;
}

uint32_t IConnectionSettings::loggingMsTimeInterval() const {
  return msinterval_;
}

void IConnectionSettings::setLoggingMsTimeInterval(uint32_t mstime) {
  msinterval_ = mstime;
}

std::string IConnectionSettings::toString() const {
  char buff[1024] = {0};
  common::SNPrintf(buff, sizeof(buff), "%d,%s,%" PRIu32, type_, connection_name_, msinterval_);
  return buff;
}

IConnectionSettingsBase::IConnectionSettingsBase(const std::string &connectionName, connectionTypes type)
  : IConnectionSettings(connectionName, type), hash_() {
  setConnectionNameAndUpdateHash(connectionName);
}

IConnectionSettingsBase::~IConnectionSettingsBase() {
}

void IConnectionSettingsBase::setConnectionNameAndUpdateHash(const std::string& name) {
  using namespace common::utils;
  setName(name);
  common::buffer_type bcon = common::convertFromString<common::buffer_type>(connection_name_);
  uint64_t v = hash::crc64(0, bcon);
  hash_ = common::convertToString(v);
}

std::string IConnectionSettingsBase::hash() const {
  return hash_;
}

std::string IConnectionSettingsBase::loggingPath() const {
  std::string logDir = common::convertToString(SettingsManager::instance().loggingDirectory());
  std::string ext;
  if (type_ == REDIS) {
    ext = LOGGING_REDIS_FILE_EXTENSION;
  } else if (type_ == MEMCACHED) {
    ext = LOGGING_MEMCACHED_FILE_EXTENSION;
  } else if (type_ == SSDB) {
    ext = LOGGING_SSDB_FILE_EXTENSION;
  } else if (type_ == LEVELDB) {
    ext = LOGGING_LEVELDB_FILE_EXTENSION;
  } else if (type_ == ROCKSDB) {
    ext = LOGGING_ROCKSDB_FILE_EXTENSION;
  } else if (type_ == UNQLITE) {
    ext = LOGGING_UNQLITE_FILE_EXTENSION;
  } else {
    NOTREACHED();
  }
  return logDir + hash() + ext;
}

IConnectionSettingsBase* IConnectionSettingsBase::createFromType(connectionTypes type,
                                                                 const std::string& conName) {
#ifdef BUILD_WITH_REDIS
  if (type == REDIS) {
    return new redis::RedisConnectionSettings(conName);
  }
#endif
#ifdef BUILD_WITH_MEMCACHED
  if (type == MEMCACHED) {
    return new memcached::MemcachedConnectionSettings(conName);
  }
#endif
#ifdef BUILD_WITH_SSDB
  if (type == SSDB) {
    return new ssdb::SsdbConnectionSettings(conName);
  }
#endif
#ifdef BUILD_WITH_LEVELDB
  if (type == LEVELDB) {
    return new leveldb::LeveldbConnectionSettings(conName);
  }
#endif
#ifdef BUILD_WITH_ROCKSDB
  if (type == ROCKSDB) {
    return new rocksdb::RocksdbConnectionSettings(conName);
  }
#endif
#ifdef BUILD_WITH_UNQLITE
  if (type == UNQLITE) {
    return new unqlite::UnqliteConnectionSettings(conName);
  }
#endif
#ifdef BUILD_WITH_LMDB
  if (type == LMDB) {
    return new lmdb::LmdbConnectionSettings(conName);
  }
#endif
  NOTREACHED();
  return nullptr;
}

IConnectionSettingsBase* IConnectionSettingsBase::fromString(const std::string &val) {
  if (val.empty()) {
    return nullptr;
  }

  IConnectionSettingsBase *result = nullptr;

  size_t len = val.size();

  uint8_t commaCount = 0;
  std::string elText;

  for (size_t i = 0; i < len; ++i ) {
    char ch = val[i];
    if (ch == ',') {
      if (commaCount == 0) {
        int crT = elText[0] - 48;
        result = createFromType((connectionTypes)crT, std::string());
        if (!result) {
          return nullptr;
        }
      } else if (commaCount == 1) {
        result->setConnectionNameAndUpdateHash(elText);
      } else if (commaCount == 2) {
        uint32_t msTime = common::convertFromString<uint32_t>(elText);
        result->setLoggingMsTimeInterval(msTime);
        if (!isRemoteType(result->type())) {
          result->setCommandLine(val.substr(i+1));
          break;
        }
      } else if (commaCount == 3) {
        result->setCommandLine(elText);
        IConnectionSettingsRemote * remote = dynamic_cast<IConnectionSettingsRemote *>(result);
        if (remote) {
          SSHInfo sinf(val.substr(i + 1));
          remote->setSshInfo(sinf);
        }
        break;
      }
      commaCount++;
      elText.clear();
    } else {
      elText += ch;
    }
  }
  return result;
}

bool isRemoteType(connectionTypes type) {
  return type == REDIS || type == MEMCACHED || type == SSDB;
}

std::string IConnectionSettingsBase::toString() const {
  DCHECK(type_ != DBUNKNOWN);

  std::stringstream str;
  str << IConnectionSettings::toString() << ',' << commandLine();
  std::string res = str.str();
  return res;
}

IConnectionSettingsLocal::IConnectionSettingsLocal(const std::string& connectionName,
                                                   connectionTypes type)
  : IConnectionSettingsBase(connectionName, type) {
  CHECK(!isRemoteType(type));
}

////===================== IConnectionSettingsRemote =====================////

IConnectionSettingsRemote::IConnectionSettingsRemote(const std::string& connectionName,
                                                     connectionTypes type)
  : IConnectionSettingsBase(connectionName, type), ssh_info_() {
  CHECK(isRemoteType(type));
}

IConnectionSettingsRemote::~IConnectionSettingsRemote() {
}

std::string IConnectionSettingsRemote::fullAddress() const {
  const common::net::hostAndPort h = host();
  return common::convertToString(h);
}

IConnectionSettingsRemote* IConnectionSettingsRemote::createFromType(connectionTypes type,
                                                                     const std::string& conName,
                                                                     const common::net::hostAndPort& host) {
  IConnectionSettingsRemote* remote = nullptr;
#ifdef BUILD_WITH_REDIS
  if (type == REDIS) {
    remote = new redis::RedisConnectionSettings(conName);
  }
#endif
#ifdef BUILD_WITH_MEMCACHED
  if (type == MEMCACHED) {
    remote = new memcached::MemcachedConnectionSettings(conName);
  }
#endif
#ifdef BUILD_WITH_SSDB
  if (type == SSDB) {
    remote = new ssdb::SsdbConnectionSettings(conName);
  }
#endif
  if (!remote) {
    NOTREACHED();
    return nullptr;
  }

  remote->setHost(host);
  return remote;
}

std::string IConnectionSettingsRemote::toString() const {
  DCHECK(type_ != DBUNKNOWN);

  std::stringstream str;
  str << IConnectionSettingsBase::toString() << ',' << common::convertToString(ssh_info_);
  std::string res = str.str();
  return res;
}

SSHInfo IConnectionSettingsRemote::sshInfo() const {
  return ssh_info_;
}

void IConnectionSettingsRemote::setSshInfo(const SSHInfo& info) {
  ssh_info_ = info;
}

const char* useHelpText(connectionTypes type) {
  if (type == DBUNKNOWN) {
    return nullptr;
  }
  if (type == REDIS) {
    return "<b>Usage: [OPTIONS] [cmd [arg [arg ...]]]</b><br/>"
           "<b>-h &lt;hostname&gt;</b>      Server hostname (default: 127.0.0.1).<br/>"
           "<b>-p &lt;port&gt;</b>          Server port (default: 6379).<br/>"
           "<b>-s &lt;socket&gt;</b>        Server socket (overrides hostname and port).<br/>"
           "<b>-a &lt;password&gt;</b>      Password to use when connecting to the server.<br/>"
           "<b>-r &lt;repeat&gt;</b>        Execute specified command N times.<br/>"
           "<b>-i &lt;interval&gt;</b>      When <b>-r</b> is used, waits &lt;interval&gt; seconds per command.<br/>"
           "                   It is possible to specify sub-second times like <b>-i</b> 0.1.<br/>"
           "<b>-n &lt;db&gt;</b>            Database number.<br/>"
           "<b>-d &lt;delimiter&gt;</b>     Multi-bulk delimiter in for raw formatting (default: \\n).<br/>"
           "<b>-c</b>                 Enable cluster mode (follow -ASK and -MOVED redirections).<br/>"
           "<b>--latency</b>          Enter a special mode continuously sampling latency.<br/>"
           "<b>--latency-history</b>  Like <b>--latency</b> but tracking latency changes over time.<br/>"
           "                   Default time interval is 15 sec. Change it using <b>-i</b>.<br/>"
           "<b>--slave</b>            Simulate a slave showing commands received from the master.<br/>"
           "<b>--rdb &lt;filename&gt;</b>   Transfer an RDB dump from remote server to local file.<br/>"
           /*"<b>--pipe</b>             Transfer raw Redis protocol from stdin to server.<br/>"
           "<b>--pipe-timeout &lt;n&gt;</b> In <b>--pipe mode</b>, abort with error if after sending all data.<br/>"
           "                   no reply is received within &lt;n&gt; seconds.<br/>"
           "                   Default timeout: %d. Use 0 to wait forever.<br/>"*/
           "<b>--bigkeys</b>          Sample Redis keys looking for big keys.<br/>"
           "<b>--scan</b>             List all keys using the SCAN command.<br/>"
           "<b>--pattern &lt;pat&gt;</b>    Useful with <b>--scan</b> to specify a SCAN pattern.<br/>"
           "<b>--intrinsic-latency &lt;sec&gt;</b> Run a test to measure intrinsic system latency.<br/>"
           "                   The test will run for the specified amount of seconds.<br/>"
           "<b>--eval &lt;file&gt;</b>      Send an EVAL command using the Lua script at <b>&lt;file&gt;</b>.";
  }
  if (type == MEMCACHED) {
      return "<b>Usage: [OPTIONS] [cmd [arg [arg ...]]]</b><br/>"
             "<b>-h &lt;hostname&gt;</b>      Server hostname (default: 127.0.0.1).<br/>"
             "<b>-p &lt;port&gt;</b>          Server port (default: 11211).<br/>"
             "<b>-u &lt;username&gt;</b>      Username to use when connecting to the server.<br/>"
             "<b>-a &lt;password&gt;</b>      Password to use when connecting to the server.<br/>"
             "<b>-d &lt;delimiter&gt;</b>     Multi-bulk delimiter in for raw formatting (default: \\n).<br/>";
  }
  if (type == SSDB) {
      return "<b>Usage: [OPTIONS] [cmd [arg [arg ...]]]</b><br/>"
             "<b>-u &lt;username&gt;</b>      Username to use when connecting to the server.<br/>"
             "<b>-a &lt;password&gt;</b>      Password to use when connecting to the server.<br/>"
             "<b>-d &lt;delimiter&gt;</b>     Multi-bulk delimiter in for raw formatting (default: \\n).<br/>";
  }
  if (type == LEVELDB) {
      return "<b>Usage: [OPTIONS] [cmd [arg [arg ...]]]</b><br/>"
             "<b>-f &lt;db&gt;</b>            File path to database.<br/>"
             "<b>-c </b>            Create database if missing.<br/>"
             "<b>-d &lt;delimiter&gt;</b>     Multi-bulk delimiter in for raw formatting (default: \\n).<br/>";
  }
  if (type == ROCKSDB) {
      return "<b>Usage: [OPTIONS] [cmd [arg [arg ...]]]</b><br/>"
             "<b>-f &lt;db&gt;</b>            File path to database.<br/>"
             "<b>-c </b>            Create database if missing.<br/>"
             "<b>-d &lt;delimiter&gt;</b>     Multi-bulk delimiter in for raw formatting (default: \\n).<br/>";
  }
  if (type == UNQLITE) {
      return "<b>Usage: [OPTIONS] [cmd [arg [arg ...]]]</b><br/>"
             "<b>-f &lt;db&gt;</b>            File path to database.<br/>"
             "<b>-c </b>            Create database if missing.<br/>"
             "<b>-d &lt;delimiter&gt;</b>     Multi-bulk delimiter in for raw formatting (default: \\n).<br/>";
  }
  if (type == LMDB) {
      return "<b>Usage: [OPTIONS] [cmd [arg [arg ...]]]</b><br/>"
             "<b>-f &lt;db&gt;</b>            File path to database.<br/>"
             "<b>-c </b>            Create database if missing.<br/>"
             "<b>-d &lt;delimiter&gt;</b>     Multi-bulk delimiter in for raw formatting (default: \\n).<br/>";
  }

  NOTREACHED();
  return nullptr;
}

std::string defaultCommandLine(connectionTypes type) {
#ifdef BUILD_WITH_REDIS
  if (type == REDIS) {
    redis::RedisConfig r;
    return common::convertToString(r);
  }
#endif
#ifdef BUILD_WITH_MEMCACHED
  if (type == MEMCACHED) {
    memcached::MemcachedConfig r;
    return common::convertToString(r);
  }
#endif
#ifdef BUILD_WITH_SSDB
  if (type == SSDB) {
    ssdb::SsdbConfig r;
    return common::convertToString(r);
  }
#endif
#ifdef BUILD_WITH_LEVELDB
  if (type == LEVELDB) {
    leveldb::LeveldbConfig r;
    r.options.create_if_missing = true;
    return common::convertToString(r);
  }
#endif
#ifdef BUILD_WITH_ROCKSDB
  if (type == ROCKSDB) {
    rocksdb::RocksdbConfig r;
    r.options.create_if_missing = true;
    return common::convertToString(r);
  }
#endif
#ifdef BUILD_WITH_UNQLITE
  if (type == UNQLITE) {
    unqlite::UnqliteConfig r;
    r.create_if_missing = true;
    return common::convertToString(r);
  }
#endif
#ifdef BUILD_WITH_LMDB
  if (type == LMDB) {
    lmdb::LmdbConfig r;
    r.create_if_missing = true;
    return common::convertToString(r);
  }
#endif

  return std::string();
}

IClusterSettingsBase::IClusterSettingsBase(const std::string& connectionName, connectionTypes type)
  : IConnectionSettings(connectionName, type) {
}

IClusterSettingsBase::cluster_connection_type IClusterSettingsBase::nodes() const {
  return clusters_nodes_;
}

IConnectionSettingsBaseSPtr IClusterSettingsBase::root() const {
  if (clusters_nodes_.empty()) {
    return IConnectionSettingsBaseSPtr();
  }

  return clusters_nodes_[0];
}

void IClusterSettingsBase::addNode(IConnectionSettingsBaseSPtr node) {
  if (!node) {
    return;
  }

  clusters_nodes_.push_back(node);
}

IClusterSettingsBase* IClusterSettingsBase::createFromType(connectionTypes type,
                                                           const std::string& conName) {
#ifdef BUILD_WITH_REDIS
  if (type == REDIS) {
    return new redis::RedisClusterSettings(conName);
  }
#endif

  NOTREACHED();
  return nullptr;
}

IClusterSettingsBase* IClusterSettingsBase::fromString(const std::string& val) {
  if (val.empty()) {
    return nullptr;
  }

  IClusterSettingsBase *result = nullptr;
  size_t len = val.size();

  uint8_t commaCount = 0;
  std::string elText;

  for (size_t i = 0; i < len; ++i) {
    char ch = val[i];
    if (ch == ',') {
      if (commaCount == 0) {
        int crT = elText[0] - 48;
        result = createFromType((connectionTypes)crT);
        if (!result) {
          return nullptr;
        }
      } else if (commaCount == 1) {
        result->setName(elText);
      } else if (commaCount == 2) {
        uint32_t msTime = common::convertFromString<uint32_t>(elText);
        result->setLoggingMsTimeInterval(msTime);
        std::string serText;
        for (size_t j = i + 2; j < len; ++j) {
          ch = val[j];
          if (ch == magicNumber || j == len - 1) {
            IConnectionSettingsBaseSPtr ser(IConnectionSettingsBase::fromString(serText));
            result->addNode(ser);
            serText.clear();
          } else {
            serText += ch;
          }
        }
        break;
      }
      commaCount++;
      elText.clear();
    } else {
      elText += ch;
    }
  }

  return result;
}

std::string IClusterSettingsBase::toString() const {
  DCHECK(type_ != DBUNKNOWN);

  std::stringstream str;
  str << IConnectionSettings::toString() << ',';
  for (size_t i = 0; i < clusters_nodes_.size(); ++i) {
    IConnectionSettingsBaseSPtr serv = clusters_nodes_[i];
    if (serv) {
      str << magicNumber << serv->toString();
    }
  }

  std::string res = str.str();
  return res;
}

IConnectionSettingsBaseSPtr IClusterSettingsBase::findSettingsByHost(const common::net::hostAndPort& host) const {
  for (size_t i = 0; i < clusters_nodes_.size(); ++i) {
    IConnectionSettingsBaseSPtr cur = clusters_nodes_[i];
    IConnectionSettingsRemote * remote = dynamic_cast<IConnectionSettingsRemote *>(cur.get());
    if (remote && remote->host() == host) {
      return cur;
    }
  }

  return IConnectionSettingsBaseSPtr();
}

}  // namespace fastonosql
