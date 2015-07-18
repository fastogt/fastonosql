/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  HashKit library
 *
 *  Copyright (C) 2011-2012 Data Differential, http://datadifferential.com/
 *  Copyright (C) 2009-2010 Brian Aker All rights reserved.
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


#if !defined(__cplusplus)
# include <stdbool.h>
#endif
#include <inttypes.h>
#include <sys/types.h>

#include <libhashkit-1.0/visibility.h>
#include <libhashkit-1.0/configure.h>
#include <libhashkit-1.0/types.h>
#include <libhashkit-1.0/has.h>
#include <libhashkit-1.0/algorithm.h>
#include <libhashkit-1.0/behavior.h>
#include <libhashkit-1.0/digest.h>
#include <libhashkit-1.0/function.h>
#include <libhashkit-1.0/str_algorithm.h>
#include <libhashkit-1.0/strerror.h>
#include <libhashkit-1.0/string.h>

struct hashkit_st
{
  struct hashkit_function_st {
    hashkit_hash_fn function;
    void *context;
  } base_hash, distribution_hash;

  struct {
    bool is_base_same_distributed:1;
  } flags;

  struct {
    bool is_allocated:1;
  } options;

  void *_key;
};

#ifdef __cplusplus
extern "C" {
#endif

HASHKIT_API
  hashkit_st *hashkit_create(hashkit_st *hash);

HASHKIT_API
  hashkit_st *hashkit_clone(hashkit_st *destination, const hashkit_st *ptr);

HASHKIT_API
  bool hashkit_compare(const hashkit_st *first, const hashkit_st *second);

HASHKIT_API
  void hashkit_free(hashkit_st *hash);

HASHKIT_API
  hashkit_string_st *hashkit_encrypt(hashkit_st *,
                                     const char* source, size_t source_length);

HASHKIT_API
  hashkit_string_st *hashkit_decrypt(hashkit_st *,
                                     const char* source, size_t source_length);

HASHKIT_API
  bool hashkit_key(hashkit_st *, const char *key, const size_t key_length);

#ifdef __cplusplus
} // extern "C"
#endif
