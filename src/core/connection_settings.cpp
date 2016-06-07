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
#include "core/redis/connection_settings.h"
#include "core/redis/cluster_settings.h"
#include "core/redis/sentinel_settings.h"
#define LOGGING_REDIS_FILE_EXTENSION ".red"
#endif
#ifdef BUILD_WITH_MEMCACHED
#include "core/memcached/connection_settings.h"
#define LOGGING_MEMCACHED_FILE_EXTENSION ".mem"
#endif
#ifdef BUILD_WITH_SSDB
#include "core/ssdb/connection_settings.h"
#define LOGGING_SSDB_FILE_EXTENSION ".ssdb"
#endif
#ifdef BUILD_WITH_LEVELDB
#include "core/leveldb/connection_settings.h"
#define LOGGING_LEVELDB_FILE_EXTENSION ".leveldb"
#endif
#ifdef BUILD_WITH_ROCKSDB
#include "core/rocksdb/connection_settings.h"
#define LOGGING_ROCKSDB_FILE_EXTENSION ".rocksdb"
#endif
#ifdef BUILD_WITH_UNQLITE
#include "core/unqlite/connection_settings.h"
#define LOGGING_UNQLITE_FILE_EXTENSION ".unq"
#endif
#ifdef BUILD_WITH_LMDB
#include "core/lmdb/connection_settings.h"
#define LOGGING_LMDB_FILE_EXTENSION ".lmdb"
#endif

namespace {
  const char magicNumber = 0x1E;
}

namespace fastonosql {
namespace core {

ConnectionSettingsPath::ConnectionSettingsPath()
  : path_() {
}

ConnectionSettingsPath::ConnectionSettingsPath(const std::string& path)
  : path_(path) {
}

ConnectionSettingsPath::ConnectionSettingsPath(const common::file_system::ascii_string_path &path)
  : path_(path) {
}

bool ConnectionSettingsPath::equals(const ConnectionSettingsPath& path) const {
  return path_.equals(path.path_);
}

std::string ConnectionSettingsPath::name() const {
  return path_.fileName();
}

std::string ConnectionSettingsPath::directory() const {
  return path_.directory();
}

std::string ConnectionSettingsPath::toString() const {
  return common::convertToString(path_);
}

ConnectionSettingsPath ConnectionSettingsPath::root() {
  static common::file_system::ascii_string_path root(common::file_system::get_separator_string<char>());
  return ConnectionSettingsPath(root);
}

IConnectionSettings::IConnectionSettings(const connection_path_t& connectionPath, connectionTypes type)
  : connection_path_(connectionPath), type_(type), msinterval_(0) {
}

IConnectionSettings::~IConnectionSettings() {
}

void IConnectionSettings::setPath(const connection_path_t& path) {
  connection_path_ = path;
}

IConnectionSettings::connection_path_t IConnectionSettings::path() const {
  return connection_path_;
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
  return common::MemSPrintf("%d,%s,%" PRIu32, type_, connection_path_.toString(), msinterval_);
}

IConnectionSettingsBase::IConnectionSettingsBase(const connection_path_t& connectionPath, connectionTypes type)
  : IConnectionSettings(connectionPath, type), hash_() {
  setConnectionPathAndUpdateHash(connectionPath);
}

IConnectionSettingsBase::~IConnectionSettingsBase() {
}

void IConnectionSettingsBase::setConnectionPathAndUpdateHash(const connection_path_t& name) {
  setPath(name);
  std::string path = connection_path_.toString();
  common::buffer_t bcon = common::convertFromString<common::buffer_t>(path);
  uint64_t v = common::utils::hash::crc64(0, bcon);
  hash_ = common::convertToString(v);
}

std::string IConnectionSettingsBase::hash() const {
  return hash_;
}

std::string IConnectionSettingsBase::loggingPath() const {
  std::string logDir = common::convertToString(SettingsManager::instance().loggingDirectory());
  std::string prefix = logDir + hash();
#ifdef BUILD_WITH_REDIS
  if (type_ == REDIS) {
    return prefix + LOGGING_REDIS_FILE_EXTENSION;
  }
#endif
#ifdef BUILD_WITH_MEMCACHED
  if (type_ == MEMCACHED) {
    return prefix + LOGGING_MEMCACHED_FILE_EXTENSION;
  }
#endif
#ifdef BUILD_WITH_SSDB
  if (type_ == SSDB) {
    return prefix + LOGGING_SSDB_FILE_EXTENSION;
  }
#endif
#ifdef BUILD_WITH_LEVELDB
  if (type_ == LEVELDB) {
    return prefix + LOGGING_LEVELDB_FILE_EXTENSION;
  }
#endif
#ifdef BUILD_WITH_ROCKSDB
  if (type_ == ROCKSDB) {
    return prefix + LOGGING_ROCKSDB_FILE_EXTENSION;
  }
#endif
#ifdef BUILD_WITH_UNQLITE
  if (type_ == UNQLITE) {
    return prefix + LOGGING_UNQLITE_FILE_EXTENSION;
  }
#endif

  NOTREACHED();
  return std::string();
}

IConnectionSettingsBase* IConnectionSettingsBase::createFromType(connectionTypes type,
                                                                 const connection_path_t& conName) {
#ifdef BUILD_WITH_REDIS
  if (type == REDIS) {
    return new redis::ConnectionSettings(conName);
  }
#endif
#ifdef BUILD_WITH_MEMCACHED
  if (type == MEMCACHED) {
    return new memcached::ConnectionSettings(conName);
  }
#endif
#ifdef BUILD_WITH_SSDB
  if (type == SSDB) {
    return new ssdb::ConnectionSettings(conName);
  }
#endif
#ifdef BUILD_WITH_LEVELDB
  if (type == LEVELDB) {
    return new leveldb::ConnectionSettings(conName);
  }
#endif
#ifdef BUILD_WITH_ROCKSDB
  if (type == ROCKSDB) {
    return new rocksdb::ConnectionSettings(conName);
  }
#endif
#ifdef BUILD_WITH_UNQLITE
  if (type == UNQLITE) {
    return new unqlite::ConnectionSettings(conName);
  }
#endif
#ifdef BUILD_WITH_LMDB
  if (type == LMDB) {
    return new lmdb::ConnectionSettings(conName);
  }
#endif

  DNOTREACHED();
  return nullptr;
}

IConnectionSettingsBase* IConnectionSettingsBase::fromString(const std::string& val) {
  if (val.empty()) {
    return nullptr;
  }

  IConnectionSettingsBase* result = nullptr;
  size_t len = val.size();
  uint8_t commaCount = 0;
  std::string elText;

  for (size_t i = 0; i < len; ++i ) {
    char ch = val[i];
    if (ch == ',') {
      if (commaCount == 0) {
        connectionTypes crT = static_cast<connectionTypes>(elText[0] - 48);
        result = createFromType(crT, connection_path_t());
        if (!result) {
          return nullptr;
        }
      } else if (commaCount == 1) {
        connection_path_t path(elText);
        result->setConnectionPathAndUpdateHash(path);
      } else if (commaCount == 2) {
        uint32_t msTime = common::convertFromString<uint32_t>(elText);
        result->setLoggingMsTimeInterval(msTime);
        if (!isRemoteType(result->type())) {
          result->setCommandLine(val.substr(i+1));
          break;
        }
      } else if (commaCount == 3) {
        result->setCommandLine(elText);
        if (IConnectionSettingsRemoteSSH* remote = dynamic_cast<IConnectionSettingsRemoteSSH*>(result)) {
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

bool isCanSSHConnection(connectionTypes type) {
  return type == REDIS;
}

std::string IConnectionSettingsBase::toString() const {
  std::stringstream str;
  str << IConnectionSettings::toString() << ',' << commandLine();
  std::string res = str.str();
  return res;
}

IConnectionSettingsLocal::IConnectionSettingsLocal(const connection_path_t& connectionPath,
                                                   connectionTypes type)
  : IConnectionSettingsBase(connectionPath, type) {
  CHECK(!isRemoteType(type));
}

////===================== IConnectionSettingsRemote =====================////

IConnectionSettingsRemote::IConnectionSettingsRemote(const connection_path_t& connectionPath,
                                                     connectionTypes type)
  : IConnectionSettingsBase(connectionPath, type) {
  CHECK(isRemoteType(type));
}

IConnectionSettingsRemote::~IConnectionSettingsRemote() {
}

std::string IConnectionSettingsRemote::fullAddress() const {
  return common::convertToString(host());
}

IConnectionSettingsRemote* IConnectionSettingsRemote::createFromType(connectionTypes type,
                                                                     const connection_path_t& conName,
                                                                     const common::net::hostAndPort& host) {
  IConnectionSettingsRemote* remote = nullptr;
#ifdef BUILD_WITH_REDIS
  if (type == REDIS) {
    remote = new redis::ConnectionSettings(conName);
  }
#endif
#ifdef BUILD_WITH_MEMCACHED
  if (type == MEMCACHED) {
    remote = new memcached::ConnectionSettings(conName);
  }
#endif
#ifdef BUILD_WITH_SSDB
  if (type == SSDB) {
    remote = new ssdb::ConnectionSettings(conName);
  }
#endif
  if (!remote) {
    NOTREACHED();
    return nullptr;
  }

  remote->setHost(host);
  return remote;
}

IConnectionSettingsRemoteSSH::IConnectionSettingsRemoteSSH(const connection_path_t& connectionName,
                                                           connectionTypes type)
  : IConnectionSettingsRemote(connectionName, type), ssh_info_() {
}

IConnectionSettingsRemoteSSH* IConnectionSettingsRemoteSSH::createFromType(connectionTypes type,
                                                                     const connection_path_t& conName,
                                                                     const common::net::hostAndPort& host) {
  IConnectionSettingsRemoteSSH* remote = nullptr;
#ifdef BUILD_WITH_REDIS
  if (type == REDIS) {
    remote = new redis::ConnectionSettings(conName);
  }
#endif
  if (!remote) {
    NOTREACHED();
    return nullptr;
  }

  remote->setHost(host);
  return remote;
}

std::string IConnectionSettingsRemoteSSH::toString() const {
  std::stringstream str;
  str << IConnectionSettingsBase::toString() << ',' << common::convertToString(ssh_info_);
  std::string res = str.str();
  return res;
}

SSHInfo IConnectionSettingsRemoteSSH::sshInfo() const {
  return ssh_info_;
}

void IConnectionSettingsRemoteSSH::setSshInfo(const SSHInfo& info) {
  ssh_info_ = info;
}

const char* commandLineHelpText(connectionTypes type) {
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
           "<b>-ns &lt;separator&gt;</b>    Namespace separator.<br/>"
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
             "<b>-d &lt;delimiter&gt;</b>     Multi-bulk delimiter in for raw formatting (default: \\n).<br/>"
             "<b>-ns &lt;separator&gt;</b>    Namespace separator.<br/>";
  }
  if (type == SSDB) {
      return "<b>Usage: [OPTIONS] [cmd [arg [arg ...]]]</b><br/>"
             "<b>-u &lt;username&gt;</b>      Username to use when connecting to the server.<br/>"
             "<b>-a &lt;password&gt;</b>      Password to use when connecting to the server.<br/>"
             "<b>-d &lt;delimiter&gt;</b>     Multi-bulk delimiter in for raw formatting (default: \\n).<br/>"
             "<b>-ns &lt;separator&gt;</b>    Namespace separator.<br/>";
  }
  if (type == LEVELDB) {
      return "<b>Usage: [OPTIONS] [cmd [arg [arg ...]]]</b><br/>"
             "<b>-f &lt;db&gt;</b>            File path to database.<br/>"
             "<b>-c </b>            Create database if missing.<br/>"
             "<b>-d &lt;delimiter&gt;</b>     Multi-bulk delimiter in for raw formatting (default: \\n).<br/>"
             "<b>-ns &lt;separator&gt;</b>    Namespace separator.<br/>";
  }
  if (type == ROCKSDB) {
      return "<b>Usage: [OPTIONS] [cmd [arg [arg ...]]]</b><br/>"
             "<b>-f &lt;db&gt;</b>            File path to database.<br/>"
             "<b>-c </b>            Create database if missing.<br/>"
             "<b>-d &lt;delimiter&gt;</b>     Multi-bulk delimiter in for raw formatting (default: \\n).<br/>"
             "<b>-ns &lt;separator&gt;</b>    Namespace separator.<br/>";
  }
  if (type == UNQLITE) {
      return "<b>Usage: [OPTIONS] [cmd [arg [arg ...]]]</b><br/>"
             "<b>-f &lt;db&gt;</b>            File path to database.<br/>"
             "<b>-c </b>            Create database if missing.<br/>"
             "<b>-d &lt;delimiter&gt;</b>     Multi-bulk delimiter in for raw formatting (default: \\n).<br/>"
             "<b>-ns &lt;separator&gt;</b>    Namespace separator.<br/>";
  }
  if (type == LMDB) {
      return "<b>Usage: [OPTIONS] [cmd [arg [arg ...]]]</b><br/>"
             "<b>-f &lt;db&gt;</b>            File path to database.<br/>"
             "<b>-c </b>            Create database if missing.<br/>"
             "<b>-d &lt;delimiter&gt;</b>     Multi-bulk delimiter in for raw formatting (default: \\n).<br/>"
             "<b>-ns &lt;separator&gt;</b>    Namespace separator.<br/>";
  }

  NOTREACHED();
  return nullptr;
}

std::string defaultCommandLine(connectionTypes type) {
#ifdef BUILD_WITH_REDIS
  if (type == REDIS) {
    redis::Config r;
    return common::convertToString(r);
  }
#endif
#ifdef BUILD_WITH_MEMCACHED
  if (type == MEMCACHED) {
    memcached::Config r;
    return common::convertToString(r);
  }
#endif
#ifdef BUILD_WITH_SSDB
  if (type == SSDB) {
    ssdb::Config r;
    return common::convertToString(r);
  }
#endif
#ifdef BUILD_WITH_LEVELDB
  if (type == LEVELDB) {
    leveldb::Config r;
    r.options.create_if_missing = true;
    return common::convertToString(r);
  }
#endif
#ifdef BUILD_WITH_ROCKSDB
  if (type == ROCKSDB) {
    rocksdb::Config r;
    r.options.create_if_missing = true;
    return common::convertToString(r);
  }
#endif
#ifdef BUILD_WITH_UNQLITE
  if (type == UNQLITE) {
    unqlite::Config r;
    r.create_if_missing = true;
    return common::convertToString(r);
  }
#endif
#ifdef BUILD_WITH_LMDB
  if (type == LMDB) {
    lmdb::Config r;
    r.create_if_missing = true;
    return common::convertToString(r);
  }
#endif

  NOTREACHED();
  return std::string();
}

IClusterSettingsBase::IClusterSettingsBase(const connection_path_t& connectionPath, connectionTypes type)
  : IConnectionSettings(connectionPath, type) {
}

IClusterSettingsBase::cluster_nodes_t IClusterSettingsBase::nodes() const {
  return clusters_nodes_;
}

void IClusterSettingsBase::addNode(IConnectionSettingsBaseSPtr node) {
  if (!node) {
    NOTREACHED();
    return;
  }

  clusters_nodes_.push_back(node);
}

IClusterSettingsBase* IClusterSettingsBase::createFromType(connectionTypes type,
                                                           const connection_path_t& conName) {
#ifdef BUILD_WITH_REDIS
  if (type == REDIS) {
    return new redis::ClusterSettings(conName);
  }
#endif

  NOTREACHED();
  return nullptr;
}

IClusterSettingsBase* IClusterSettingsBase::fromString(const std::string& val) {
  if (val.empty()) {
    return nullptr;
  }

  IClusterSettingsBase* result = nullptr;
  size_t len = val.size();

  uint8_t commaCount = 0;
  std::string elText;

  for (size_t i = 0; i < len; ++i) {
    char ch = val[i];
    if (ch == ',') {
      if (commaCount == 0) {
        int crT = elText[0] - 48;
        result = createFromType((connectionTypes)crT, connection_path_t());
        if (!result) {
          return nullptr;
        }
      } else if (commaCount == 1) {
        connection_path_t path(elText);
        result->setPath(path);
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
    IConnectionSettingsRemote* remote = dynamic_cast<IConnectionSettingsRemote*>(cur.get());  // +
    CHECK(remote);
    if (remote->host() == host) {
      return cur;
    }
  }

  return IConnectionSettingsBaseSPtr();
}

SentinelSettings::SentinelSettings()
  : sentinel(), sentinel_nodes() {
}

std::string sentinelSettingsToString(const SentinelSettings& sent) {
  std::stringstream str;
  std::string sent_raw = sent.sentinel->toString();
  str << common::utils::base64::encode64(sent_raw) << ',';

  std::string sents_raw;
  for (size_t i = 0; i < sent.sentinel_nodes.size(); ++i) {
    IConnectionSettingsBaseSPtr serv = sent.sentinel_nodes[i];
    if (serv) {
      sents_raw += magicNumber;
      sents_raw += serv->toString();
    }
  }

  str << common::utils::base64::encode64(sents_raw);
  std::string res = str.str();
  return res;
}

bool sentinelSettingsfromString(const std::string& text, SentinelSettings* sent) {
  if (text.empty() || !sent) {
    return false;
  }

  SentinelSettings result;
  size_t len = text.size();

  uint8_t commaCount = 0;
  std::string elText;
  for (size_t i = 0; i < len; ++i) {
    char ch = text[i];
    if (ch == ',' || i == len - 1) {
      if (commaCount == 0) {
        std::string sent_raw = common::utils::base64::decode64(elText);
        IConnectionSettingsBaseSPtr sent(IConnectionSettingsBase::fromString(sent_raw));
        if (!sent) {
          return false;
        }

        result.sentinel = sent;

        std::string serText;
        std::string raw_sent = common::utils::base64::decode64(text.substr(i + 1));
        len = raw_sent.length();
        for (size_t j = 0; j < len; ++j) {
          ch = raw_sent[j];
          if (ch == magicNumber || j == len - 1) {
            IConnectionSettingsBaseSPtr ser(IConnectionSettingsBase::fromString(serText));
            if (ser) {
              result.sentinel_nodes.push_back(ser);
            }
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

  *sent = result;
  return true;
}

//

ISentinelSettingsBase::ISentinelSettingsBase(const connection_path_t& connectionName, connectionTypes type)
  : IConnectionSettings(connectionName, type), sentinel_nodes_() {
}

ISentinelSettingsBase::sentinel_connections_t ISentinelSettingsBase::sentinels() const {
  return sentinel_nodes_;
}

void ISentinelSettingsBase::addSentinel(sentinel_connection_t sent) {
  sentinel_nodes_.push_back(sent);
}

ISentinelSettingsBase* ISentinelSettingsBase::createFromType(connectionTypes type,
                                                           const connection_path_t& conName) {
#ifdef BUILD_WITH_REDIS
  if (type == REDIS) {
    return new redis::SentinelSettings(conName);
  }
#endif

  NOTREACHED();
  return nullptr;
}

ISentinelSettingsBase* ISentinelSettingsBase::fromString(const std::string& val) {
  if (val.empty()) {
    return nullptr;
  }

  ISentinelSettingsBase* result = nullptr;
  size_t len = val.size();

  uint8_t commaCount = 0;
  std::string elText;

  for (size_t i = 0; i < len; ++i) {
    char ch = val[i];
    if (ch == ',') {
      if (commaCount == 0) {
        int crT = elText[0] - 48;
        result = createFromType((connectionTypes)crT, connection_path_t());
        if (!result) {
          return nullptr;
        }
      } else if (commaCount == 1) {
        connection_path_t path(elText);
        result->setPath(path);
      } else if (commaCount == 2) {
        uint32_t msTime = common::convertFromString<uint32_t>(elText);
        result->setLoggingMsTimeInterval(msTime);

        std::string serText;
        for (size_t j = i + 2; j < len; ++j) {
          ch = val[j];
          if (ch == magicNumber || j == len - 1) {
            SentinelSettings sent;
            bool res = sentinelSettingsfromString(serText, &sent);
            if (res) {
              result->addSentinel(sent);
            }
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

std::string ISentinelSettingsBase::toString() const {
  std::stringstream str;
  str << IConnectionSettings::toString() << ',';
  for (size_t i = 0; i < sentinel_nodes_.size(); ++i) {
    sentinel_connection_t sent = sentinel_nodes_[i]; 
    str << magicNumber << sentinelSettingsToString(sent);
  }

  std::string res = str.str();
  return res;
}

}  // namespace core
}  // namespace fastonosql
