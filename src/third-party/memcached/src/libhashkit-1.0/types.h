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

typedef enum {
  HASHKIT_SUCCESS,
  HASHKIT_FAILURE,
  HASHKIT_MEMORY_ALLOCATION_FAILURE,
  HASHKIT_INVALID_HASH,
  HASHKIT_INVALID_ARGUMENT,
  HASHKIT_MAXIMUM_RETURN /* Always add new error code before */
} hashkit_return_t;

static inline bool hashkit_success(const hashkit_return_t rc)
{
  return (rc == HASHKIT_SUCCESS);
}

static inline bool hashkit_failed(const hashkit_return_t rc)
{
  return (rc != HASHKIT_SUCCESS);
}

typedef enum {
  HASHKIT_HASH_DEFAULT= 0, // hashkit_one_at_a_time()
  HASHKIT_HASH_MD5,
  HASHKIT_HASH_CRC,
  HASHKIT_HASH_FNV1_64,
  HASHKIT_HASH_FNV1A_64,
  HASHKIT_HASH_FNV1_32,
  HASHKIT_HASH_FNV1A_32,
  HASHKIT_HASH_HSIEH,
  HASHKIT_HASH_MURMUR,
  HASHKIT_HASH_JENKINS,
  HASHKIT_HASH_MURMUR3,
  HASHKIT_HASH_CUSTOM,
  HASHKIT_HASH_MAX
} hashkit_hash_algorithm_t;

/**
 * Hash distributions that are available to use.
 */
typedef enum
{
  HASHKIT_DISTRIBUTION_MODULA,
  HASHKIT_DISTRIBUTION_RANDOM,
  HASHKIT_DISTRIBUTION_KETAMA,
  HASHKIT_DISTRIBUTION_MAX /* Always add new values before this. */
} hashkit_distribution_t;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct hashkit_st hashkit_st;
typedef struct hashkit_string_st hashkit_string_st;

typedef uint32_t (*hashkit_hash_fn)(const char *key, size_t key_length, void *context);

#ifdef __cplusplus
}
#endif
