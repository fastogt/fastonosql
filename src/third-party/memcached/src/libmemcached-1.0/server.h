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

#include <libmemcached-1.0/struct/server.h>

#ifdef __cplusplus
extern "C" {
#endif

LIBMEMCACHED_API
memcached_return_t memcached_server_cursor(const memcached_st *ptr,
                                           const memcached_server_fn *callback,
                                           void *context,
                                           uint32_t number_of_callbacks);

LIBMEMCACHED_API
  const memcached_instance_st * memcached_server_by_key(memcached_st *ptr,
                                                       const char *key,
                                                       size_t key_length,
                                                       memcached_return_t *error);

LIBMEMCACHED_API
void memcached_server_error_reset(memcached_server_st *ptr);

LIBMEMCACHED_API
void memcached_server_free(memcached_server_st *ptr);

LIBMEMCACHED_API
const memcached_instance_st * memcached_server_get_last_disconnect(const memcached_st *ptr);


LIBMEMCACHED_API
memcached_return_t memcached_server_add_udp(memcached_st *ptr,
                                            const char *hostname,
                                            in_port_t port);
LIBMEMCACHED_API
memcached_return_t memcached_server_add_unix_socket(memcached_st *ptr,
                                                    const char *filename);
LIBMEMCACHED_API
memcached_return_t memcached_server_add(memcached_st *ptr,
                                        const char *hostname, in_port_t port);

LIBMEMCACHED_API
memcached_return_t memcached_server_add_udp_with_weight(memcached_st *ptr,
                                                        const char *hostname,
                                                        in_port_t port,
                                                        uint32_t weight);
LIBMEMCACHED_API
memcached_return_t memcached_server_add_unix_socket_with_weight(memcached_st *ptr,
                                                                const char *filename,
                                                                uint32_t weight);
LIBMEMCACHED_API
memcached_return_t memcached_server_add_with_weight(memcached_st *ptr, const char *hostname,
                                                    in_port_t port,
                                                    uint32_t weight);

/**
  Operations on Single Servers.
*/
LIBMEMCACHED_API
uint32_t memcached_server_response_count(const memcached_instance_st * self);

LIBMEMCACHED_API
const char *memcached_server_name(const memcached_instance_st * self);

LIBMEMCACHED_API
in_port_t memcached_server_port(const memcached_instance_st * self);

LIBMEMCACHED_API
in_port_t memcached_server_srcport(const memcached_instance_st * self);

LIBMEMCACHED_API
void memcached_instance_next_retry(const memcached_instance_st * self, const time_t absolute_time);

LIBMEMCACHED_API
const char *memcached_server_type(const memcached_instance_st * ptr);

LIBMEMCACHED_API
uint8_t memcached_server_major_version(const memcached_instance_st * ptr);

LIBMEMCACHED_API
uint8_t memcached_server_minor_version(const memcached_instance_st * ptr);

LIBMEMCACHED_API
uint8_t memcached_server_micro_version(const memcached_instance_st * ptr);

#ifdef __cplusplus
} // extern "C"
#endif
