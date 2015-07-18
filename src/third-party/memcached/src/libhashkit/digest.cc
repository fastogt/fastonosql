/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 *
 *  HashKit library
 *
 *  Copyright (C) 2010-2012 Data Differential, http://datadifferential.com/
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


#include <libhashkit/common.h>

uint32_t hashkit_digest(const hashkit_st *self, const char *key, size_t key_length)
{
  return self->base_hash.function(key, key_length, self->base_hash.context);
}

uint32_t libhashkit_digest(const char *key, size_t key_length, hashkit_hash_algorithm_t hash_algorithm)
{
  switch (hash_algorithm)
  {
  case HASHKIT_HASH_DEFAULT:
    return libhashkit_one_at_a_time(key, key_length);
  case HASHKIT_HASH_MD5:
    return libhashkit_md5(key, key_length);
  case HASHKIT_HASH_CRC:
    return libhashkit_crc32(key, key_length);
  case HASHKIT_HASH_FNV1_64:
    return libhashkit_fnv1_64(key, key_length);
  case HASHKIT_HASH_FNV1A_64:
    return libhashkit_fnv1a_64(key, key_length);
  case HASHKIT_HASH_FNV1_32:
    return libhashkit_fnv1_32(key, key_length);
  case HASHKIT_HASH_FNV1A_32:
    return libhashkit_fnv1a_32(key, key_length);
  case HASHKIT_HASH_HSIEH:
#ifdef HAVE_HSIEH_HASH
    return libhashkit_hsieh(key, key_length);
#else
    return 1;
#endif
  case HASHKIT_HASH_MURMUR3:
    return libhashkit_murmur3(key, key_length);

  case HASHKIT_HASH_MURMUR:
#ifdef HAVE_MURMUR_HASH
    return libhashkit_murmur(key, key_length);
#else
    return 1;
#endif
  case HASHKIT_HASH_JENKINS:
    return libhashkit_jenkins(key, key_length);
  case HASHKIT_HASH_CUSTOM:
  case HASHKIT_HASH_MAX:
  default:
    if (DEBUG)
    {
      fprintf(stderr, "hashkit_hash_t was extended but libhashkit_generate_value was not updated\n");
      fflush(stderr);
      assert(0);
    }
    break;
  }

  return 1;
}
