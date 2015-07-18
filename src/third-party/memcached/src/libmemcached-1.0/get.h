/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Libmemcached library
 *
 *  Copyright (C) 2011 Data Differential, http://datadifferential.com/
 *  Copyright (C) 2010 Brian Aker All rights reserved.
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

/* Public defines */
LIBMEMCACHED_API
char *memcached_get(memcached_st *ptr,
                    const char *key, size_t key_length,
                    size_t *value_length,
                    uint32_t *flags,
                    memcached_return_t *error);

LIBMEMCACHED_API
memcached_return_t memcached_mget(memcached_st *ptr,
                                  const char * const *keys,
                                  const size_t *key_length,
                                  size_t number_of_keys);

LIBMEMCACHED_API
char *memcached_get_by_key(memcached_st *ptr,
                           const char *group_key, size_t group_key_length,
                           const char *key, size_t key_length,
                           size_t *value_length,
                           uint32_t *flags,
                           memcached_return_t *error);

LIBMEMCACHED_API
memcached_return_t memcached_mget_by_key(memcached_st *ptr,
                                         const char *group_key,
                                         size_t group_key_length,
                                         const char * const *keys,
                                         const size_t *key_length,
                                         const size_t number_of_keys);

LIBMEMCACHED_API
char *memcached_fetch(memcached_st *ptr,
                      char *key,
                      size_t *key_length,
                      size_t *value_length,
                      uint32_t *flags,
                      memcached_return_t *error);

LIBMEMCACHED_API
memcached_result_st *memcached_fetch_result(memcached_st *ptr,
                                            memcached_result_st *result,
                                            memcached_return_t *error);

LIBMEMCACHED_API
memcached_return_t memcached_mget_execute(memcached_st *ptr,
                                          const char * const *keys,
                                          const size_t *key_length,
                                          const size_t number_of_keys,
                                          memcached_execute_fn *callback,
                                          void *context,
                                          const uint32_t number_of_callbacks);

LIBMEMCACHED_API
memcached_return_t memcached_mget_execute_by_key(memcached_st *ptr,
                                                 const char *group_key,
                                                 size_t group_key_length,
                                                 const char * const *keys,
                                                 const size_t *key_length,
                                                 size_t number_of_keys,
                                                 memcached_execute_fn *callback,
                                                 void *context,
                                                 const uint32_t number_of_callbacks);

#ifdef __cplusplus
}
#endif
