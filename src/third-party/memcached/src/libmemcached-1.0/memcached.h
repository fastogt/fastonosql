/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Libmemcached library
 *
 *  Copyright (C) 2011 Data Differential, http://datadifferential.com/
 *  Copyright (C) 2006-2009 Brian Aker All rights reserved.
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

/* This seems to be required for older compilers @note http://stackoverflow.com/questions/8132399/how-to-printf-uint64-t  */
#ifndef __STDC_FORMAT_MACROS
#  define __STDC_FORMAT_MACROS
#endif

#ifdef __cplusplus
#  include <cinttypes>
#  include <cstddef>
#  include <cstdlib>
#else
#  include <inttypes.h>
#  include <stddef.h>
#  include <stdlib.h>
#  include <stdbool.h>
#endif

#include <sys/types.h>

#include <libmemcached-1.0/visibility.h>
#include <libmemcached-1.0/configure.h>
#include <libmemcached-1.0/platform.h>

#include <libmemcached-1.0/limits.h>
#include <libmemcached-1.0/defaults.h>

#include <libmemcached-1.0/types/behavior.h>
#include <libmemcached-1.0/types/callback.h>
#include <libmemcached-1.0/types/connection.h>
#include <libmemcached-1.0/types/hash.h>
#include <libmemcached-1.0/types/return.h>
#include <libmemcached-1.0/types/server_distribution.h>

#include <libmemcached-1.0/return.h>

#include <libmemcached-1.0/types.h>
#include <libmemcached-1.0/callbacks.h>
#include <libmemcached-1.0/alloc.h>
#include <libmemcached-1.0/triggers.h>

#include <libhashkit-1.0/hashkit.h>

#include <libmemcached-1.0/struct/callback.h>
#include <libmemcached-1.0/struct/string.h>
#include <libmemcached-1.0/struct/result.h>
#include <libmemcached-1.0/struct/allocator.h>
#include <libmemcached-1.0/struct/sasl.h>
#include <libmemcached-1.0/struct/memcached.h>
#include <libmemcached-1.0/struct/server.h>
#include <libmemcached-1.0/struct/stat.h>

#include <libmemcached-1.0/basic_string.h>
#include <libmemcached-1.0/error.h>
#include <libmemcached-1.0/stats.h>

// Everything above this line must be in the order specified.
#include <libmemcached-1.0/allocators.h>
#include <libmemcached-1.0/analyze.h>
#include <libmemcached-1.0/auto.h>
#include <libmemcached-1.0/behavior.h>
#include <libmemcached-1.0/callback.h>
#include <libmemcached-1.0/delete.h>
#include <libmemcached-1.0/dump.h>
#include <libmemcached-1.0/encoding_key.h>
#include <libmemcached-1.0/exist.h>
#include <libmemcached-1.0/fetch.h>
#include <libmemcached-1.0/flush.h>
#include <libmemcached-1.0/flush_buffers.h>
#include <libmemcached-1.0/get.h>
#include <libmemcached-1.0/hash.h>
#include <libmemcached-1.0/options.h>
#include <libmemcached-1.0/parse.h>
#include <libmemcached-1.0/quit.h>
#include <libmemcached-1.0/result.h>
#include <libmemcached-1.0/server.h>
#include <libmemcached-1.0/server_list.h>
#include <libmemcached-1.0/storage.h>
#include <libmemcached-1.0/strerror.h>
#include <libmemcached-1.0/touch.h>
#include <libmemcached-1.0/verbosity.h>
#include <libmemcached-1.0/version.h>
#include <libmemcached-1.0/sasl.h>

#include <libmemcached-1.0/deprecated_types.h>

#ifdef __cplusplus
extern "C" {
#endif

LIBMEMCACHED_API
void memcached_servers_reset(memcached_st *ptr);

LIBMEMCACHED_API
memcached_st *memcached_create(memcached_st *ptr);

LIBMEMCACHED_API
memcached_st *memcached(const char *string, size_t string_length);

LIBMEMCACHED_API
void memcached_free(memcached_st *ptr);

LIBMEMCACHED_API
memcached_return_t memcached_reset(memcached_st *ptr);

LIBMEMCACHED_API
void memcached_reset_last_disconnected_server(memcached_st *ptr);

LIBMEMCACHED_API
memcached_st *memcached_clone(memcached_st *clone, const memcached_st *ptr);

LIBMEMCACHED_API
void *memcached_get_user_data(const memcached_st *ptr);

LIBMEMCACHED_API
void *memcached_set_user_data(memcached_st *ptr, void *data);

LIBMEMCACHED_API
memcached_return_t memcached_push(memcached_st *destination, const memcached_st *source);

LIBMEMCACHED_API
const memcached_instance_st * memcached_server_instance_by_position(const memcached_st *ptr, uint32_t server_key);

LIBMEMCACHED_API
uint32_t memcached_server_count(const memcached_st *);

LIBMEMCACHED_API
uint64_t memcached_query_id(const memcached_st *);

#ifdef __cplusplus
} // extern "C"
#endif
