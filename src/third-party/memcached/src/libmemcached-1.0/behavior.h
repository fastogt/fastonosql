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

#ifdef __cplusplus
extern "C" {
#endif

LIBMEMCACHED_API
memcached_return_t memcached_behavior_set(memcached_st *ptr, const memcached_behavior_t flag, uint64_t data);

LIBMEMCACHED_API
uint64_t memcached_behavior_get(memcached_st *ptr, const memcached_behavior_t flag);

LIBMEMCACHED_API
memcached_return_t memcached_behavior_set_distribution(memcached_st *ptr, memcached_server_distribution_t type);

LIBMEMCACHED_API
memcached_server_distribution_t memcached_behavior_get_distribution(memcached_st *ptr);

LIBMEMCACHED_API
memcached_return_t memcached_behavior_set_key_hash(memcached_st *ptr, memcached_hash_t type);

LIBMEMCACHED_API
memcached_hash_t memcached_behavior_get_key_hash(memcached_st *ptr);

LIBMEMCACHED_API
memcached_return_t memcached_behavior_set_distribution_hash(memcached_st *ptr, memcached_hash_t type);

LIBMEMCACHED_API
memcached_hash_t memcached_behavior_get_distribution_hash(memcached_st *ptr);

LIBMEMCACHED_API
  const char *libmemcached_string_behavior(const memcached_behavior_t flag);

LIBMEMCACHED_API
  const char *libmemcached_string_distribution(const memcached_server_distribution_t flag);

LIBMEMCACHED_API
  memcached_return_t memcached_bucket_set(memcached_st *self,
                                          const uint32_t *host_map,
                                          const uint32_t *forward_map,
                                          const uint32_t buckets,
                                          const uint32_t replicas);

#ifdef __cplusplus
}
#endif
