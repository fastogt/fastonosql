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

#include "gui/socket_tls.h"

#include <openssl/err.h>
#include <openssl/ssl.h>

namespace common {
namespace net {

SocketTls::SocketTls(const HostAndPort& host) : hs_(host), ssl_(nullptr) {}

common::ErrnoError SocketTls::Connect(struct timeval* tv) {
  common::net::ClientSocketTcp hs(hs_.GetHost());
  common::ErrnoError err = hs.Connect(tv);
  if (err) {
    return err;
  }

#if OPENSSL_VERSION_NUMBER < 0x10100000L
  const SSL_METHOD* method = TLSv1_2_client_method();
#else
  const SSL_METHOD* method = TLS_client_method();
#endif
  if (!method) {
    hs.Disconnect();
    return common::make_errno_error_inval();
  }

  SSL_CTX* ctx = SSL_CTX_new(method);
  if (!ctx) {
    hs.Disconnect();
    return common::make_errno_error_inval();
  }

  SSL* ssl = SSL_new(ctx);
  SSL_CTX_free(ctx);
  ctx = nullptr;
  if (!ssl) {
    SSL_free(ssl);
    hs.Disconnect();
    return common::make_errno_error_inval();
  }

  SSL_set_fd(ssl, hs.GetFd());
  int e = SSL_connect(ssl);
  if (e < 0) {
    int err = SSL_get_error(ssl, e);
    char* str = ERR_error_string(err, nullptr);
    SSL_free(ssl);
    hs.Disconnect();
    return common::make_errno_error(str, EINTR);
  }

  hs_.SetInfo(hs.GetInfo());
  ssl_ = ssl;
  return common::ErrnoError();
}

common::ErrnoError SocketTls::Disconnect() {
  return Close();
}

bool SocketTls::IsConnected() const {
  return hs_.IsConnected();
}

common::net::HostAndPort SocketTls::GetHost() const {
  return hs_.GetHost();
}

bool SocketTls::IsValid() const {
  return hs_.IsValid();
}

common::ErrnoError SocketTls::WriteImpl(const void* data, size_t size, size_t* nwrite_out) {
  int len = SSL_write(ssl_, data, size);
  if (len < 0) {
    int err = SSL_get_error(ssl_, len);
    char* str = ERR_error_string(err, nullptr);
    return common::make_errno_error(str, EINTR);
  }

  *nwrite_out = len;
  return common::ErrnoError();
}

common::ErrnoError SocketTls::ReadImpl(void* out_data, size_t max_size, size_t* nread_out) {
  int len = SSL_read(ssl_, out_data, max_size);
  if (len < 0) {
    int err = SSL_get_error(ssl_, len);
    char* str = ERR_error_string(err, nullptr);
    return common::make_errno_error(str, EINTR);
  }

  *nread_out = len;
  return common::ErrnoError();
}

common::ErrnoError SocketTls::CloseImpl() {
  if (ssl_) {
    SSL_free(ssl_);
    ssl_ = nullptr;
  }

  return hs_.Close();
}

}  // namespace net
}  // namespace common
