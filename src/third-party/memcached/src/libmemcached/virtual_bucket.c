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

#include <libmemcached/common.h>

struct bucket_t {
  uint32_t master;
  uint32_t forward;
};

struct memcached_virtual_bucket_t {
  bool has_forward;
  uint32_t size;
  uint32_t replicas;
  struct bucket_t buckets[];
};

memcached_return_t memcached_virtual_bucket_create(memcached_st *self,
                                                   const uint32_t *host_map,
                                                   const uint32_t *forward_map,
                                                   const uint32_t buckets,
                                                   const uint32_t replicas)
{
  if (self == NULL || host_map == NULL || buckets == 0U)
  {
    return MEMCACHED_INVALID_ARGUMENTS;
  }

  memcached_virtual_bucket_free(self);

  struct memcached_virtual_bucket_t *virtual_bucket= (struct memcached_virtual_bucket_t *)malloc(sizeof(struct memcached_virtual_bucket_t) + sizeof(struct bucket_t) *buckets);
  
  if (virtual_bucket == NULL)
  {
    return MEMCACHED_MEMORY_ALLOCATION_FAILURE;
  }


  virtual_bucket->size= buckets;
  virtual_bucket->replicas= replicas;
  self->virtual_bucket= virtual_bucket;

  uint32_t x= 0;
  for (; x < buckets; x++)
  {
    virtual_bucket->buckets[x].master= host_map[x];
    if (forward_map)
    {
      virtual_bucket->buckets[x].forward= forward_map[x];
    }
    else
    {
      virtual_bucket->buckets[x].forward= 0;
    }
  }

  return MEMCACHED_SUCCESS;
}

void memcached_virtual_bucket_free(memcached_st *self)
{
  if (self)
  {
    if (self->virtual_bucket)
    {
      free(self->virtual_bucket);
      self->virtual_bucket= NULL;
    }
  }
}

uint32_t memcached_virtual_bucket_get(const memcached_st *self, uint32_t digest)
{
  if (self)
  {
    if (self->virtual_bucket)
    {
      uint32_t result= (uint32_t) (digest & (self->virtual_bucket->size -1));
      return self->virtual_bucket->buckets[result].master;
    }

    return (uint32_t) (digest & (self->number_of_hosts -1));
  }

  return 0;
}
