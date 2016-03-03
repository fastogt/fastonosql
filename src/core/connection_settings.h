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

#include "core/connection_types.h"
#include "core/ssh_info.h"

namespace fastonosql {

class IConnectionSettings {
 public:
  virtual ~IConnectionSettings();

  std::string name() const;
  void setName(const std::string& name);

  connectionTypes type() const;

  bool loggingEnabled() const;

  uint32_t loggingMsTimeInterval() const;
  void setLoggingMsTimeInterval(uint32_t mstime);

  virtual std::string toString() const;
  virtual IConnectionSettings* clone() const = 0;

 protected:
  IConnectionSettings(const std::string& connectionName, connectionTypes type);

  std::string connection_name_;
  const connectionTypes type_;
  uint32_t msinterval_;
};

bool isRemoteType(connectionTypes type);

class IConnectionSettingsBase
  : public IConnectionSettings {
 public:
  virtual ~IConnectionSettingsBase();
  std::string hash() const;

  std::string loggingPath() const;

  void setConnectionNameAndUpdateHash(const std::string& name);

  virtual std::string commandLine() const = 0;
  virtual void setCommandLine(const std::string& line) = 0;

  virtual std::string fullAddress() const = 0;

  static IConnectionSettingsBase* createFromType(connectionTypes type, const std::string& conName);
  static IConnectionSettingsBase* fromString(const std::string& val);

  virtual std::string toString() const;
  virtual IConnectionSettingsBase* clone() const = 0;

 protected:
  IConnectionSettingsBase(const std::string& connectionName, connectionTypes type);

 private:
  using IConnectionSettings::setName;

  std::string hash_;
};

class IConnectionSettingsLocal
  : public IConnectionSettingsBase {
 public:
  virtual std::string path() const = 0;

 protected:
  IConnectionSettingsLocal(const std::string& connectionName, connectionTypes type);
};

class IConnectionSettingsRemote
  : public IConnectionSettingsBase {
 public:
  virtual ~IConnectionSettingsRemote();

  virtual void setHost(const common::net::hostAndPort& host) = 0;
  virtual common::net::hostAndPort host() const = 0;

  virtual std::string commandLine() const = 0;
  virtual void setCommandLine(const std::string& line) = 0;

  virtual std::string fullAddress() const;

  static IConnectionSettingsRemote* createFromType(connectionTypes type, const std::string& conName,
                                                   const common::net::hostAndPort& host);

  virtual std::string toString() const;

  SSHInfo sshInfo() const;
  void setSshInfo(const SSHInfo& info);

 protected:
  IConnectionSettingsRemote(const std::string& connectionName, connectionTypes type);

 private:
  SSHInfo ssh_info_;
};

const char* useHelpText(connectionTypes type);
std::string defaultCommandLine(connectionTypes type);

typedef common::shared_ptr<IConnectionSettingsBase> IConnectionSettingsBaseSPtr;

class IClusterSettingsBase
  : public IConnectionSettings {
 public:
  typedef std::vector<IConnectionSettingsBaseSPtr> cluster_connection_type;
  cluster_connection_type nodes() const;

  void addNode(IConnectionSettingsBaseSPtr node);

  static IClusterSettingsBase* createFromType(connectionTypes type,
                                              const std::string& conName = std::string());
  static IClusterSettingsBase* fromString(const std::string& val);

  virtual std::string toString() const;
  virtual IClusterSettingsBase* clone() const = 0;

  IConnectionSettingsBaseSPtr findSettingsByHost(const common::net::hostAndPort& host) const;

 protected:
  IClusterSettingsBase(const std::string& connectionName, connectionTypes type);

 private:
  cluster_connection_type clusters_nodes_;  // first element is root!!!
};

typedef common::shared_ptr<IClusterSettingsBase> IClusterSettingsBaseSPtr;

}  // namespace fastonosql
