/*
 * Libmemcached library
 *
 * Copyright (C) 2012 Data Differential, http://datadifferential.com/
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *
 *     * The names of its contributors may not be used to endorse or
 * promote products derived from this software without specific prior
 * written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#pragma once

#ifdef __cplusplus
# include <cerrno>
#else
# include <errno.h>
#endif

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif

#ifndef _WIN32_WINNT
# define _WIN32_WINNT 0x0501
#endif

#ifdef __MINGW32__
# if(_WIN32_WINNT >= 0x0501)
# else
#  undef _WIN32_WINNT
#  define _WIN32_WINNT 0x0501
# endif /* _WIN32_WINNT >= 0x0501 */
#endif /* __MINGW32__ */

#if defined(HAVE_WINSOCK2_H) && HAVE_WINSOCK2_H
# include <winsock2.h>
#endif

#if defined(HAVE_WS2TCPIP_H) && HAVE_WS2TCPIP_H
# include <ws2tcpip.h>
#endif

#if defined(HAVE_IO_H) && HAVE_IO_H
# include <io.h>
#endif

struct sockaddr_un
{
  short int sun_family;
  char sun_path[108];
};

static inline int translate_windows_error()
{
  int local_errno= WSAGetLastError();

  switch(local_errno) {
  case WSAEINVAL:
    local_errno= EINPROGRESS;
    break;
  case WSAEALREADY:
  case WSAEWOULDBLOCK:
    local_errno= EAGAIN;
    break;

  case WSAECONNREFUSED:
    local_errno= ECONNREFUSED;
    break;

  case WSAENETUNREACH:
    local_errno= ENETUNREACH;
    break;

  case WSAETIMEDOUT:
    local_errno= ETIMEDOUT;
    break;

  case WSAECONNRESET:
    local_errno= ECONNRESET;
    break;

  case WSAEADDRINUSE:
    local_errno= EADDRINUSE;
    break;

  case WSAEOPNOTSUPP:
    local_errno= EOPNOTSUPP;
    break;

  case WSAENOPROTOOPT:
    local_errno= ENOPROTOOPT;
    break;

  default:
    break;
  }

  return local_errno;
}
