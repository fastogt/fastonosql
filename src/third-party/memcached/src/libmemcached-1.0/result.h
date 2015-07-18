/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Libmemcached library
 *
 *  Copyright (C) 2011 Data Differential, http://datadifferential.com/ 
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

#include <libmemcached-1.0/struct/result.h>

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Result Struct */
LIBMEMCACHED_API
void memcached_result_free(memcached_result_st *result);

LIBMEMCACHED_API
void memcached_result_reset(memcached_result_st *ptr);

LIBMEMCACHED_API
memcached_result_st *memcached_result_create(const memcached_st *ptr,
                                             memcached_result_st *result);

LIBMEMCACHED_API
const char *memcached_result_key_value(const memcached_result_st *self);

LIBMEMCACHED_API
size_t memcached_result_key_length(const memcached_result_st *self);

LIBMEMCACHED_API
const char *memcached_result_value(const memcached_result_st *self);

LIBMEMCACHED_API
char *memcached_result_take_value(memcached_result_st *self);

LIBMEMCACHED_API
size_t memcached_result_length(const memcached_result_st *self);

LIBMEMCACHED_API
uint32_t memcached_result_flags(const memcached_result_st *self);

LIBMEMCACHED_API
uint64_t memcached_result_cas(const memcached_result_st *self);

LIBMEMCACHED_API
memcached_return_t memcached_result_set_value(memcached_result_st *ptr, const char *value, size_t length);

LIBMEMCACHED_API
void memcached_result_set_flags(memcached_result_st *self, uint32_t flags);

LIBMEMCACHED_API
void memcached_result_set_expiration(memcached_result_st *self, time_t expiration);

#ifdef __cplusplus
} // extern "C"
#endif
