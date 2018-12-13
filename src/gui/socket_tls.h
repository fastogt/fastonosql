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

#include <common/net/isocket.h>
#include <common/net/socket_tcp.h>

typedef struct ssl_st SSL;

namespace common {
namespace net {

class SocketTls : public ISocket {
 public:
  explicit SocketTls(const HostAndPort& host);

  ErrnoError Connect(struct timeval* tv = nullptr) WARN_UNUSED_RESULT;

  ErrnoError Disconnect() WARN_UNUSED_RESULT;
  bool IsConnected() const;
  net::HostAndPort GetHost() const;
  bool IsValid() const override;

 private:
  ErrnoError WriteImpl(const void* data, size_t size, size_t* nwrite_out) override;
  ErrnoError ReadImpl(void* out_data, size_t max_size, size_t* nread_out) override;
  ErrnoError CloseImpl() override;

  ClientSocketTcp hs_;
  SSL* ssl_;
};

}  // namespace net
}  // namespace common
