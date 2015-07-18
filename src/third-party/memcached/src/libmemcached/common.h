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

/*
  Common include file for libmemached
*/

#pragma once

#include <mem_config.h>

#ifdef __cplusplus
# include <cstddef>
# include <cstdio>
# include <cstdlib>
# include <cstring>
# include <ctime>
# include <cctype>
# include <cerrno>
# include <climits>
#else
# ifdef HAVE_STDDEF_H
#  include <stddef.h>
# endif
# ifdef HAVE_STDLIB_H
#  include <stdio.h>
# endif
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
# include <string.h>
# ifdef HAVE_TIME_H
#  include <time.h>
# endif
# ifdef HAVE_ERRNO_H
#  include <errno.h>
# endif
# ifdef HAVE_LIMITS_H
#  include <limits.h>
# endif
#endif

#ifdef HAVE_SYS_UN_H
# include <sys/un.h>
#endif

#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif

#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif

#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif

#ifdef HAVE_DLFCN_H
# include <dlfcn.h>
#endif

#if defined(_WIN32)
# include "libmemcached/windows.hpp"
#endif

#include <libmemcached-1.0/memcached.h>
#include "libmemcached/watchpoint.h"
#include "libmemcached/is.h"
typedef struct memcached_st Memcached;

#ifdef HAVE_POLL_H
# include <poll.h>
#else
# include "libmemcached/poll.h"
#endif

#ifdef __cplusplus
memcached_instance_st* memcached_instance_fetch(memcached_st *ptr, uint32_t server_key);
#endif

/* These are private not to be installed headers */
#include "libmemcached/error.hpp"
#include "libmemcached/memory.h"
#include "libmemcached/io.h"
#ifdef __cplusplus
# include "libmemcached/string.hpp"
# include "libmemcached/memcached/protocol_binary.h"
# include "libmemcached/io.hpp"
# include "libmemcached/udp.hpp"
# include "libmemcached/do.hpp"
# include "libmemcached/socket.hpp"
# include "libmemcached/connect.hpp"
# include "libmemcached/allocators.hpp"
# include "libmemcached/hash.hpp"
# include "libmemcached/quit.hpp"
# include "libmemcached/instance.hpp"
# include "libmemcached/server_instance.h"
# include "libmemcached/server.hpp"
# include "libmemcached/flag.hpp"
# include "libmemcached/behavior.hpp"
# include "libmemcached/sasl.hpp"
# include "libmemcached/server_list.hpp"
#endif

#include "libmemcached/internal.h"
#include "libmemcached/array.h"
#include "libmemcached/libmemcached_probes.h"
#include "libmemcached/byteorder.h"
#include "libmemcached/initialize_query.h"

#ifdef __cplusplus
# include "libmemcached/response.h"
# include "libmemcached/namespace.h"
#else
# include "libmemcached/virtual_bucket.h"
#endif

#ifdef __cplusplus
# include "libmemcached/backtrace.hpp"
# include "libmemcached/assert.hpp"
# include "libmemcached/server.hpp"
# include "libmemcached/key.hpp"
# include "libmemcached/encoding_key.h"
# include "libmemcached/result.h"
# include "libmemcached/version.hpp"
#endif

#include "libmemcached/continuum.hpp"

#if !defined(__GNUC__) || (__GNUC__ == 2 && __GNUC_MINOR__ < 96)

#define likely(x)       if((x))
#define unlikely(x)     if((x))

#else

#define likely(x)       if(__builtin_expect((x) != 0, 1))
#define unlikely(x)     if(__builtin_expect((x) != 0, 0))
#endif

#define MEMCACHED_BLOCK_SIZE 1024
#define MEMCACHED_DEFAULT_COMMAND_SIZE 350
#define SMALL_STRING_LEN 1024
#define HUGE_STRING_LEN 8196

#ifdef __cplusplus
extern "C" {
#endif

memcached_return_t run_distribution(memcached_st *ptr);

#ifdef __cplusplus
static inline void memcached_server_response_increment(memcached_instance_st* instance)
{
  instance->events(POLLIN);
  instance->cursor_active_++;
}
#endif

#define memcached_server_response_decrement(A) (A)->cursor_active_--
#define memcached_server_response_reset(A) (A)->cursor_active_=0

#define memcached_instance_response_increment(A) (A)->cursor_active_++
#define memcached_instance_response_decrement(A) (A)->cursor_active_--
#define memcached_instance_response_reset(A) (A)->cursor_active_=0

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
bool memcached_purge(memcached_instance_st*);
memcached_instance_st* memcached_instance_by_position(const memcached_st *ptr, uint32_t server_key);
#endif
