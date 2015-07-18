/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  LibMemcached
 *
 *  Copyright (C) 2011 Data Differential, http://datadifferential.com/
 *  Copyright (C) 2006-2009 Brian Aker
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *      * Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *
 *      * Redistributions in binary form must reproduce the above
 *  copyright notice, this list of conditions and the following disclaimer
 *  in the documentation and/or other materials provided with the
 *  distribution.
 *
 *      * The names of its contributors may not be used to endorse or
 *  promote products derived from this software without specific prior
 *  written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#pragma once


/* To hide the platform differences between MS Windows and Unix, I am
 * going to use the Microsoft way and #define the Microsoft-specific
 * functions to the unix way. Microsoft use a separate subsystem for sockets,
 * but Unix normally just use a filedescriptor on the same functions. It is
 * a lot easier to map back to the unix way with macros than going the other
 * way without side effect ;-)
 */
#if defined(WIN32) || defined(__MINGW32__)
# include "win32/wrappers.h"
# define get_socket_errno() WSAGetLastError()
#else
# include <unistd.h>
# define INVALID_SOCKET -1
# define SOCKET_ERROR -1
# define closesocket(a) close(a)
# define get_socket_errno() errno
#endif

#ifdef __cplusplus
static inline void memcached_close_socket(memcached_socket_t& socket_fd)
{
  closesocket(socket_fd);
  socket_fd= INVALID_SOCKET;
}
#endif

#ifndef HAVE_MSG_NOSIGNAL
# define MSG_NOSIGNAL 0
#endif

#ifndef HAVE_MSG_DONTWAIT
# define MSG_DONTWAIT 0
#endif

#ifndef HAVE_MSG_MORE
# define MSG_MORE 0
#endif


