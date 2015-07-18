/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  HashKit
 *
 *  Copyright (C) 2011 Data Differential, http://datadifferential.com/
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

const char * libhashkit_string_hash(hashkit_hash_algorithm_t type)
{
  switch(type)
  {
  case HASHKIT_HASH_DEFAULT: return "DEFAULT";
  case HASHKIT_HASH_MD5: return "MD5";
  case HASHKIT_HASH_CRC: return "CRC";
  case HASHKIT_HASH_FNV1_64: return "FNV1_64";
  case HASHKIT_HASH_FNV1A_64: return "FNV1A_64";
  case HASHKIT_HASH_FNV1_32: return "FNV1_32";
  case HASHKIT_HASH_FNV1A_32: return "FNV1A_32";
  case HASHKIT_HASH_HSIEH: return "HSIEH";
  case HASHKIT_HASH_MURMUR: return "MURMUR";
  case HASHKIT_HASH_MURMUR3: return "MURMUR3";
  case HASHKIT_HASH_JENKINS: return "JENKINS";
  case HASHKIT_HASH_CUSTOM: return "CUSTOM";
  default:
  case HASHKIT_HASH_MAX: return "INVALID";
  }
}
