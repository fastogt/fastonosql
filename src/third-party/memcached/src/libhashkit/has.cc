/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  HashKit library
 *
 *  Copyright (C) 2011-2012 Data Differential, http://datadifferential.com/
 *  Copyright (C) 2009 Brian Aker All rights reserved.
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

bool libhashkit_has_algorithm(const hashkit_hash_algorithm_t algo)
{
  switch (algo)
  {
  case HASHKIT_HASH_FNV1_64:
  case HASHKIT_HASH_FNV1A_64:
#if __WORDSIZE == 64 && defined(HAVE_FNV64_HASH)
    return true;
#else
    return false;
#endif

  case HASHKIT_HASH_HSIEH:
#ifdef HAVE_HSIEH_HASH
    return true;
#else
    return false;
#endif

  case HASHKIT_HASH_MURMUR3:
  case HASHKIT_HASH_MURMUR:
#ifdef HAVE_MURMUR_HASH
    return true;
#else
    return false;
#endif

  case HASHKIT_HASH_FNV1_32:
  case HASHKIT_HASH_FNV1A_32:
  case HASHKIT_HASH_DEFAULT:
  case HASHKIT_HASH_MD5:
  case HASHKIT_HASH_CRC:
  case HASHKIT_HASH_JENKINS:
  case HASHKIT_HASH_CUSTOM:
    return true;

  case HASHKIT_HASH_MAX:
    break;
  }

  return false;
}
