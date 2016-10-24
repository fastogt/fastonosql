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

#include <stdint.h>  // for uint32_t

#include <memory>  // for shared_ptr
#include <string>  // for string
#include <vector>  // for vector

#include <common/file_system.h>  // for ascii_string_path, etc
#include <common/net/types.h>    // for HostAndPort

#include "core/connection_types.h"  // for connectionTypes
#include "core/ssh_info.h"          // for SSHInfo

namespace fastonosql {
namespace core {

static const char magicNumber = 0x1E;

class ConnectionSettingsPath {
 public:
  ConnectionSettingsPath();
  explicit ConnectionSettingsPath(const std::string& path);

  std::string Name() const;
  std::string Directory() const;
  bool Equals(const ConnectionSettingsPath& path) const;
  std::string ToString() const;
  static ConnectionSettingsPath Root();

 private:
  explicit ConnectionSettingsPath(const common::file_system::ascii_string_path& path);
  common::file_system::ascii_string_path path_;
};

inline bool operator==(const ConnectionSettingsPath& r, const ConnectionSettingsPath& l) {
  return r.Equals(l);
}

class IConnectionSettings : public common::ClonableBase<IConnectionSettings> {
 public:
  typedef ConnectionSettingsPath connection_path_t;
  virtual ~IConnectionSettings();

  connection_path_t Path() const;
  void SetPath(const connection_path_t& path);

  connectionTypes Type() const;

  bool IsLoggingEnabled() const;

  uint32_t LoggingMsTimeInterval() const;
  void SetLoggingMsTimeInterval(uint32_t mstime);

  virtual std::string ToString() const;
  virtual IConnectionSettings* Clone() const = 0;

 protected:
  IConnectionSettings(const connection_path_t& connectionPath, connectionTypes type);
  connection_path_t connection_path_;
  const connectionTypes type_;

 private:
  uint32_t msinterval_;
};

class IConnectionSettingsBase : public IConnectionSettings {
 public:
  virtual ~IConnectionSettingsBase();
  std::string Hash() const;

  std::string LoggingPath() const;

  void SetConnectionPathAndUpdateHash(const connection_path_t& name);

  virtual std::string CommandLine() const = 0;
  virtual void SetCommandLine(const std::string& line) = 0;

  virtual std::string FullAddress() const = 0;

  static IConnectionSettingsBase* CreateFromType(connectionTypes type,
                                                 const connection_path_t& conName);
  static IConnectionSettingsBase* FromString(const std::string& val);

  virtual std::string ToString() const override;
  virtual IConnectionSettingsBase* Clone() const = 0;

 protected:
  IConnectionSettingsBase(const connection_path_t& connectionPath, connectionTypes type);

 private:
  using IConnectionSettings::SetPath;
  std::string hash_;
};

class IConnectionSettingsLocal : public IConnectionSettingsBase {
 public:
  virtual std::string DBpath() const = 0;

 protected:
  IConnectionSettingsLocal(const connection_path_t& connectionPath, connectionTypes type);
};

class IConnectionSettingsRemote : public IConnectionSettingsBase {
 public:
  virtual ~IConnectionSettingsRemote();

  virtual void SetHost(const common::net::HostAndPort& host) = 0;
  virtual common::net::HostAndPort Host() const = 0;

  virtual std::string CommandLine() const = 0;
  virtual void SetCommandLine(const std::string& line) = 0;

  virtual std::string FullAddress() const;

  static IConnectionSettingsRemote* CreateFromType(connectionTypes type,
                                                   const connection_path_t& conName,
                                                   const common::net::HostAndPort& host);

 protected:
  IConnectionSettingsRemote(const connection_path_t& connectionPath, connectionTypes type);
};

class IConnectionSettingsRemoteSSH : public IConnectionSettingsRemote {
 public:
  struct SSHInfo SSHInfo() const;
  void SetSSHInfo(const struct SSHInfo& info);

  virtual std::string ToString() const override;

  static IConnectionSettingsRemoteSSH* createFromType(connectionTypes type,
                                                      const connection_path_t& conName,
                                                      const common::net::HostAndPort& host);

 protected:
  IConnectionSettingsRemoteSSH(const connection_path_t& connectionName, connectionTypes type);

 private:
  struct SSHInfo ssh_info_;
};

typedef common::shared_ptr<IConnectionSettingsBase> IConnectionSettingsBaseSPtr;

}  // namespace core
}  // namespace fastonosql
