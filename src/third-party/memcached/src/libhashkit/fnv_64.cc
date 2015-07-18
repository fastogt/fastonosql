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

#if __WORDSIZE == 64 && defined(HAVE_FNV64_HASH)

/* FNV hash'es lifted from Dustin Sallings work */
static uint64_t FNV_64_INIT= 0xcbf29ce484222325;
static uint64_t FNV_64_PRIME= 0x100000001b3;

uint32_t hashkit_fnv1_64(const char *key, size_t key_length, void *)
{
  /* Thanks to pierre@demartines.com for the pointer */
  uint64_t hash= FNV_64_INIT;

  for (size_t x= 0; x < key_length; x++)
  {
    hash *= FNV_64_PRIME;
    hash ^= (uint64_t)key[x];
  }

  return (uint32_t)hash;
}

uint32_t hashkit_fnv1a_64(const char *key, size_t key_length, void *)
{
  uint32_t hash= (uint32_t) FNV_64_INIT;

  for (size_t x= 0; x < key_length; x++)
  {
    uint32_t val= (uint32_t)key[x];
    hash ^= val;
    hash *= (uint32_t) FNV_64_PRIME;
  }

  return hash;
}

#else
uint32_t hashkit_fnv1_64(const char *, size_t, void *)
{
  return 0;
}

uint32_t hashkit_fnv1a_64(const char *, size_t, void *)
{
  return 0;
}
#endif
