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

#include <string>  // for string

#include <common/error.h>  // for Error

#include "core/connection.h"        // for Connection, ConnectionAllocatorTr...
#include "core/connection_types.h"  // for connectionTypes

namespace fastonosql {
namespace core {

template <typename NConnection, typename Config, connectionTypes ContType>
class DBConnection {
 public:
  typedef ConnectionAllocatorTraits<NConnection, Config> ConnectionAllocatorTrait;
  typedef Connection<ConnectionAllocatorTrait> dbconnection_t;
  typedef typename dbconnection_t::config_t config_t;
  static constexpr connectionTypes connection_t = ContType;

  DBConnection() : connection_(), interrupted_(false) {}

  connectionTypes connectionType() { return connection_t; }

  common::Error connect(const config_t& config) { return connection_.connect(config); }

  common::Error disconnect() { return connection_.disconnect(); }

  bool isConnected() const { return connection_.isConnected(); }

  void setInterrupted(bool interrupted) { interrupted_ = interrupted; }

  bool isInterrupted() const { return interrupted_; }

  std::string delimiter() const { return connection_.config_.delimiter; }

  std::string nsSeparator() const { return connection_.config_.ns_separator; }

  config_t config() const { return connection_.config_; }

 protected:
  dbconnection_t connection_;
  bool interrupted_;
};

}  // namespace core
}  // namespace fastonosql
