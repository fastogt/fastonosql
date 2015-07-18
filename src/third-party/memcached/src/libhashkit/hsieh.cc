/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  HashKit library
 *
 *  Copyright (C) 2011-2012 Data Differential, http://datadifferential.com/
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

/* By Paul Hsieh (C) 2004, 2005.  Covered under the Paul Hsieh
 * derivative license.
 * See: http://www.azillionmonkeys.com/qed/weblicense.html for license
 * details.
 * http://www.azillionmonkeys.com/qed/hash.html
*/

#include <libhashkit/common.h>

#undef get16bits
#if (defined(__GNUC__) && defined(__i386__))
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
                      +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

#ifdef HAVE_HSIEH_HASH
uint32_t hashkit_hsieh(const char *key, size_t key_length, void *)
{
  uint32_t hash = 0, tmp;
  int rem;

  if (key_length <= 0 || key == NULL)
    return 0;

  rem = key_length & 3;
  key_length >>= 2;

  /* Main loop */
  for (;key_length > 0; key_length--)
  {
    hash  += get16bits (key);
    tmp    = (get16bits (key+2) << 11) ^ hash;
    hash   = (hash << 16) ^ tmp;
    key  += 2*sizeof (uint16_t);
    hash  += hash >> 11;
  }

  /* Handle end cases */
  switch (rem)
  {
  case 3: hash += get16bits (key);
          hash ^= hash << 16;
          hash ^= (uint32_t)key[sizeof (uint16_t)] << 18;
          hash += hash >> 11;
          break;
  case 2: hash += get16bits (key);
          hash ^= hash << 11;
          hash += hash >> 17;
          break;
  case 1: hash += (unsigned char)(*key);
          hash ^= hash << 10;
          hash += hash >> 1;
  default:
          break;
  }

  /* Force "avalanching" of final 127 bits */
  hash ^= hash << 3;
  hash += hash >> 5;
  hash ^= hash << 4;
  hash += hash >> 17;
  hash ^= hash << 25;
  hash += hash >> 6;

  return hash;
}
#else
uint32_t hashkit_hsieh(const char *, size_t , void *)
{
  return 0;
}
#endif
