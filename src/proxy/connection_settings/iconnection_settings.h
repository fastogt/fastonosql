/*  Copyright (C) 2014-2018 FastoGT. All right reserved.

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

#include <memory>

#include <fastonosql/core/connection_types.h>  // for core::ConnectionType

#include "proxy/connection_settings/connection_settings_path.h"
#include "proxy/types.h"

namespace fastonosql {
namespace proxy {

class IConnectionSettings : public common::ClonableBase<IConnectionSettings> {
 public:
  static const char default_ns_separator[];

  virtual ~IConnectionSettings() override;

  connection_path_t GetPath() const;
  void SetPath(const connection_path_t& path);

  core::ConnectionType GetType() const;

  bool IsHistoryEnabled() const;

  int GetLoggingMsTimeInterval() const;
  void SetLoggingMsTimeInterval(int mstime);

  virtual IConnectionSettings* Clone() const override = 0;

 protected:
  IConnectionSettings(const connection_path_t& connection_path, core::ConnectionType type);
  connection_path_t connection_path_;
  const core::ConnectionType type_;

 private:
  int msinterval_;
};

class IConnectionSettingsBase : public IConnectionSettings {
 public:
  NsDisplayStrategy GetNsDisplayStrategy() const;
  void SetNsDisplayStrategy(NsDisplayStrategy strategy);

  std::string GetNsSeparator() const;
  void SetNsSeparator(const std::string& ns);

  virtual ~IConnectionSettingsBase() override;
  std::string GetHash() const;

  std::string GetLoggingPath() const;

  void SetConnectionPathAndUpdateHash(const connection_path_t& name);

  virtual std::string GetDelimiter() const = 0;
  virtual void SetDelimiter(const std::string& delimiter) = 0;

  virtual std::string GetCommandLine() const = 0;
  virtual void SetCommandLine(const std::string& line) = 0;

  virtual std::string GetFullAddress() const = 0;

  virtual IConnectionSettingsBase* Clone() const override = 0;

  virtual void PrepareInGuiIfNeeded();

 protected:
  IConnectionSettingsBase(const connection_path_t& connection_path,
                          const std::string& log_directory,
                          core::ConnectionType type);

 private:
  using IConnectionSettings::SetPath;
  const std::string log_directory_;
  std::string hash_;

  std::string ns_separator_;
  NsDisplayStrategy ns_display_strategy_;
};

typedef std::shared_ptr<IConnectionSettingsBase> IConnectionSettingsBaseSPtr;

}  // namespace proxy
}  // namespace fastonosql
