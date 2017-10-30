/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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

#include "core/connection_types.h"  // for connectionTypes

#include "core/internal/connection.h"  // for Connection, ConnectionAllocatorTr...

namespace fastonosql {
namespace core {
namespace internal {

template <typename NConnection, typename Config, connectionTypes ContType>
class DBConnection {
 public:
  typedef ConnectionAllocatorTraits<NConnection, Config> ConnectionAllocatorTrait;
  typedef Connection<ConnectionAllocatorTrait> dbconnection_t;
  typedef typename dbconnection_t::config_t config_t;
  typedef typename dbconnection_t::handle_t nconnection_t;
  static constexpr connectionTypes connection_t = ContType;

  DBConnection() : connection_(), interrupted_(false) {}
  virtual ~DBConnection() {}

  static connectionTypes GetConnectionType() { return connection_t; }

  virtual common::Error Connect(const config_t& config) WARN_UNUSED_RESULT { return connection_.Connect(config); }
  virtual common::Error Disconnect() WARN_UNUSED_RESULT { return connection_.Disconnect(); }

  virtual bool IsAuthenticated() const { return IsConnected(); }
  virtual bool IsConnected() const { return connection_.IsConnected(); }

  common::Error TestIsConnected() const {
    if (!IsConnected()) {
      return common::make_error("Not connected");
    }

    return common::Error();
  }

  common::Error TestIsAuthenticated() const {
    common::Error err = TestIsConnected();
    if (err) {
      return err;
    }

    if (!IsAuthenticated()) {
      return common::make_error("Not autentificated");
    }

    return common::Error();
  }

  void SetInterrupted(bool interrupted) { interrupted_ = interrupted; }

  bool IsInterrupted() const { return interrupted_; }

  std::string GetDelimiter() const {
    config_t conf = GetConfig();
    if (conf) {
      return conf->delimiter;
    }

    return Config::default_delimiter;
  }

 protected:
  config_t GetConfig() const { return connection_.config_; }

  dbconnection_t connection_;
  bool interrupted_;
};

}  // namespace internal
}  // namespace core
}  // namespace fastonosql
