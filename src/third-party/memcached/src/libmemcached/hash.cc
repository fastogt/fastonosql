/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Libmemcached library
 *
 *  Copyright (C) 2011 Data Differential, http://datadifferential.com/
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


#include <libmemcached/common.h>

#include <sys/time.h>

#include <libmemcached/virtual_bucket.h>

uint32_t memcached_generate_hash_value(const char *key, size_t key_length, memcached_hash_t hash_algorithm)
{
  return libhashkit_digest(key, key_length, (hashkit_hash_algorithm_t)hash_algorithm);
}

static inline uint32_t generate_hash(const Memcached *ptr, const char *key, size_t key_length)
{
  return hashkit_digest(&ptr->hashkit, key, key_length);
}

static uint32_t dispatch_host(const Memcached *ptr, uint32_t hash)
{
  switch (ptr->distribution)
  {
  case MEMCACHED_DISTRIBUTION_CONSISTENT:
  case MEMCACHED_DISTRIBUTION_CONSISTENT_WEIGHTED:
  case MEMCACHED_DISTRIBUTION_CONSISTENT_KETAMA:
  case MEMCACHED_DISTRIBUTION_CONSISTENT_KETAMA_SPY:
    {
      uint32_t num= ptr->ketama.continuum_points_counter;
      WATCHPOINT_ASSERT(ptr->ketama.continuum);

      memcached_continuum_item_st *begin, *end, *left, *right, *middle;
      begin= left= ptr->ketama.continuum;
      end= right= ptr->ketama.continuum + num;

      while (left < right)
      {
        middle= left + (right - left) / 2;
        if (middle->value < hash)
          left= middle + 1;
        else
          right= middle;
      }
      if (right == end)
        right= begin;
      return right->index;
    }
  case MEMCACHED_DISTRIBUTION_MODULA:
    return hash % memcached_server_count(ptr);
  case MEMCACHED_DISTRIBUTION_RANDOM:
    return (uint32_t) random() % memcached_server_count(ptr);
  case MEMCACHED_DISTRIBUTION_VIRTUAL_BUCKET:
    {
      return memcached_virtual_bucket_get(ptr, hash);
    }
  default:
  case MEMCACHED_DISTRIBUTION_CONSISTENT_MAX:
    WATCHPOINT_ASSERT(0); /* We have added a distribution without extending the logic */
    return hash % memcached_server_count(ptr);
  }
  /* NOTREACHED */
}

/*
  One version is public and will not modify the distribution hash, the other will.
*/
static inline uint32_t _generate_hash_wrapper(const Memcached *ptr, const char *key, size_t key_length)
{
  WATCHPOINT_ASSERT(memcached_server_count(ptr));

  if (memcached_server_count(ptr) == 1)
    return 0;

  if (ptr->flags.hash_with_namespace)
  {
    size_t temp_length= memcached_array_size(ptr->_namespace) + key_length;
    char temp[MEMCACHED_MAX_KEY];

    if (temp_length > MEMCACHED_MAX_KEY -1)
      return 0;

    strncpy(temp, memcached_array_string(ptr->_namespace), memcached_array_size(ptr->_namespace));
    strncpy(temp + memcached_array_size(ptr->_namespace), key, key_length);

    return generate_hash(ptr, temp, temp_length);
  }
  else
  {
    return generate_hash(ptr, key, key_length);
  }
}

static inline void _regen_for_auto_eject(Memcached *ptr)
{
  if (_is_auto_eject_host(ptr) && ptr->ketama.next_distribution_rebuild)
  {
    struct timeval now;

    if (gettimeofday(&now, NULL) == 0 and
        now.tv_sec > ptr->ketama.next_distribution_rebuild)
    {
      run_distribution(ptr);
    }
  }
}

void memcached_autoeject(memcached_st *ptr)
{
  _regen_for_auto_eject(ptr);
}

uint32_t memcached_generate_hash_with_redistribution(memcached_st *ptr, const char *key, size_t key_length)
{
  uint32_t hash= _generate_hash_wrapper(ptr, key, key_length);

  _regen_for_auto_eject(ptr);

  return dispatch_host(ptr, hash);
}

uint32_t memcached_generate_hash(const memcached_st *shell, const char *key, size_t key_length)
{
  const Memcached* ptr= memcached2Memcached(shell);
  if (ptr)
  {
    return dispatch_host(ptr, _generate_hash_wrapper(ptr, key, key_length));
  }

  return UINT32_MAX;
}

const hashkit_st *memcached_get_hashkit(const memcached_st *shell)
{
  const Memcached* ptr= memcached2Memcached(shell);
  if (ptr)
  {
    return &ptr->hashkit;
  }

  return NULL;
}

memcached_return_t memcached_set_hashkit(memcached_st *shell, hashkit_st *hashk)
{
  Memcached* self= memcached2Memcached(shell);
  if (self)
  {
    hashkit_free(&self->hashkit);
    hashkit_clone(&self->hashkit, hashk);

    return MEMCACHED_SUCCESS;
  }

  return MEMCACHED_INVALID_ARGUMENTS;
}

const char * libmemcached_string_hash(memcached_hash_t type)
{
  return libhashkit_string_hash((hashkit_hash_algorithm_t)type);
}
