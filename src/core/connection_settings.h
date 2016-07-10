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

#pragma once

#include <string>
#include <vector>

#include "common/smart_ptr.h"
#include "common/net/net.h"
#include "common/file_system.h"

#include "core/connection_types.h"
#include "core/ssh_info.h"

namespace fastonosql {
namespace core {

class ConnectionSettingsPath {
 public:
  ConnectionSettingsPath();
  explicit ConnectionSettingsPath(const std::string& path);

  std::string name() const;
  std::string directory() const;
  bool equals(const ConnectionSettingsPath& path) const;
  std::string toString() const;
  static ConnectionSettingsPath root();

 private:
  explicit ConnectionSettingsPath(const common::file_system::ascii_string_path& path);
  common::file_system::ascii_string_path path_;
};

inline bool operator == (const ConnectionSettingsPath& r, const ConnectionSettingsPath& l) {
  return r.equals(l);
}

class IConnectionSettings {
 public:
  typedef ConnectionSettingsPath connection_path_t;
  virtual ~IConnectionSettings();

  connection_path_t path() const;
  void setPath(const connection_path_t& path);

  connectionTypes type() const;

  bool loggingEnabled() const;

  uint32_t loggingMsTimeInterval() const;
  void setLoggingMsTimeInterval(uint32_t mstime);

  virtual std::string toString() const;
  virtual IConnectionSettings* clone() const = 0;

 protected:
  IConnectionSettings(const connection_path_t& connectionPath, connectionTypes type);
  connection_path_t connection_path_;
  const connectionTypes type_;

 private:
  uint32_t msinterval_;
};

bool isRemoteType(connectionTypes type);
bool isCanSSHConnection(connectionTypes type);

class IConnectionSettingsBase
  : public IConnectionSettings {
 public:
  virtual ~IConnectionSettingsBase();
  std::string hash() const;

  std::string loggingPath() const;

  void setConnectionPathAndUpdateHash(const connection_path_t& name);

  virtual std::string commandLine() const = 0;
  virtual void setCommandLine(const std::string& line) = 0;

  virtual std::string fullAddress() const = 0;

  static IConnectionSettingsBase* createFromType(connectionTypes type, const connection_path_t& conName);
  static IConnectionSettingsBase* fromString(const std::string& val);

  virtual std::string toString() const;
  virtual IConnectionSettingsBase* clone() const = 0;

 protected:
  IConnectionSettingsBase(const connection_path_t& connectionPath, connectionTypes type);

 private:
  using IConnectionSettings::setPath;
  std::string hash_;
};

class IConnectionSettingsLocal
  : public IConnectionSettingsBase {
 public:
  virtual std::string dbpath() const = 0;

 protected:
  IConnectionSettingsLocal(const connection_path_t& connectionPath, connectionTypes type);
};

class IConnectionSettingsRemote
  : public IConnectionSettingsBase {
 public:
  virtual ~IConnectionSettingsRemote();

  virtual void setHost(const common::net::HostAndPort& host) = 0;
  virtual common::net::HostAndPort host() const = 0;

  virtual std::string commandLine() const = 0;
  virtual void setCommandLine(const std::string& line) = 0;

  virtual std::string fullAddress() const;

  static IConnectionSettingsRemote* createFromType(connectionTypes type, const connection_path_t& conName,
                                                   const common::net::HostAndPort& host);

 protected:
  IConnectionSettingsRemote(const connection_path_t& connectionPath, connectionTypes type);
};

class IConnectionSettingsRemoteSSH
  : public IConnectionSettingsRemote {
 public:
  SSHInfo sshInfo() const;
  void setSshInfo(const SSHInfo& info);

  virtual std::string toString() const;

  static IConnectionSettingsRemoteSSH* createFromType(connectionTypes type, const connection_path_t& conName,
                                                   const common::net::HostAndPort& host);

 protected:
  IConnectionSettingsRemoteSSH(const connection_path_t& connectionName, connectionTypes type);

 private:
  SSHInfo ssh_info_;
};

const char* commandLineHelpText(connectionTypes type);
std::string defaultCommandLine(connectionTypes type);

typedef common::shared_ptr<IConnectionSettingsBase> IConnectionSettingsBaseSPtr;

class IClusterSettingsBase
  : public IConnectionSettings {
 public:
  typedef IConnectionSettingsBaseSPtr cluster_node_t;
  typedef std::vector<cluster_node_t> cluster_nodes_t;
  cluster_nodes_t nodes() const;

  void addNode(IConnectionSettingsBaseSPtr node);

  static IClusterSettingsBase* createFromType(connectionTypes type, const connection_path_t& connectionPath);
  static IClusterSettingsBase* fromString(const std::string& val);

  virtual std::string toString() const;
  virtual IClusterSettingsBase* clone() const = 0;

  virtual IConnectionSettingsBaseSPtr findSettingsByHost(const common::net::HostAndPort& host) const;

 protected:
  IClusterSettingsBase(const connection_path_t& connectionName, connectionTypes type);

 private:
  cluster_nodes_t clusters_nodes_;
};

typedef common::shared_ptr<IClusterSettingsBase> IClusterSettingsBaseSPtr;

struct SentinelSettings {
  typedef std::vector<IConnectionSettingsBaseSPtr> sentinel_nodes_t;
  SentinelSettings();

  IConnectionSettingsBaseSPtr sentinel;
  sentinel_nodes_t sentinel_nodes;
};

std::string sentinelSettingsToString(const SentinelSettings& sent);
bool sentinelSettingsfromString(const std::string& text, SentinelSettings* sent);

class ISentinelSettingsBase
  : public IConnectionSettings {
 public:
  typedef SentinelSettings sentinel_connection_t;
  typedef std::vector<sentinel_connection_t> sentinel_connections_t;

  sentinel_connections_t sentinels() const;
  void addSentinel(sentinel_connection_t sent);

  static ISentinelSettingsBase* createFromType(connectionTypes type, const connection_path_t& conName);
  static ISentinelSettingsBase* fromString(const std::string& val);

  virtual std::string toString() const;
  virtual ISentinelSettingsBase* clone() const = 0;

 protected:
  ISentinelSettingsBase(const connection_path_t& connectionName, connectionTypes type);

 private:
  sentinel_connections_t sentinel_nodes_;
};
typedef common::shared_ptr<ISentinelSettingsBase> ISentinelSettingsBaseSPtr;

}  // namespace core
}  // namespace fastonosql
