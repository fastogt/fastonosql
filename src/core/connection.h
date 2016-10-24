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

#include <memory>  // for __shared_ptr

#include <common/error.h>   // for Error
#include <common/macros.h>  // for WARN_UNUSED_RESULT, etc
#include <common/value.h>   // for ErrorValue

namespace fastonosql {
namespace core {

template <typename H, typename C>
struct ConnectionAllocatorTraits {
  typedef H handle_t;
  typedef C config_t;

  static common::Error Connect(const config_t& config, handle_t** hout);  // allocate handle
  static common::Error Disconnect(handle_t** handle);                     // deallocate handle
  static bool IsConnected(handle_t* handle);
};

template <typename ConnectionAllocatorTraits>
class Connection {
 public:
  typedef ConnectionAllocatorTraits traits_t;
  typedef typename traits_t::config_t config_t;
  typedef typename traits_t::handle_t handle_t;

  Connection() : config_(), handle_(nullptr) {}

  ~Connection() {
    common::Error err = Disconnect();
    if (err && err->isError()) {
      DNOTREACHED();
    }
  }

  bool IsConnected() const { return traits_t::IsConnected(handle_); }

  common::Error Connect(const config_t& config) WARN_UNUSED_RESULT {
    if (IsConnected()) {
      return common::Error();
    }

    handle_t* handle = nullptr;
    common::Error err = traits_t::Connect(config, &handle);
    if (err && err->isError()) {
      return err;
    }

    config_ = config;
    handle_ = handle;
    return common::Error();
  }

  common::Error Disconnect() WARN_UNUSED_RESULT {
    if (!IsConnected()) {
      return common::Error();
    }

    common::Error err = traits_t::Disconnect(&handle_);
    if (err && err->isError()) {
      return err;
    }

    config_ = config_t();
    handle_ = nullptr;
    return common::Error();
  }

  config_t config_;
  handle_t* handle_;
};

}  // namespace core
}  // namespace fastonosql
