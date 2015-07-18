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

void _libmemcached_free(const memcached_st*, void *mem, void*)
{
  if (mem)
  {
    std::free(mem);
  }
}

void *_libmemcached_malloc(const memcached_st *, size_t size, void *)
{
  return std::malloc(size);
}

void *_libmemcached_realloc(const memcached_st*, void *mem, size_t size, void *)
{
  return std::realloc(mem, size);
}

void *_libmemcached_calloc(const memcached_st *self, size_t nelem, size_t size, void *context)
{
  if (self->allocators.malloc != _libmemcached_malloc)
  {
     void *ret= _libmemcached_malloc(self, nelem * size, context);
     if (ret)
     {
       memset(ret, 0, nelem * size);
     }

     return ret;
  }

  return std::calloc(nelem, size);
}

struct memcached_allocator_t memcached_allocators_return_default(void)
{
  static struct memcached_allocator_t global_default_allocator= { _libmemcached_calloc, _libmemcached_free, _libmemcached_malloc, _libmemcached_realloc, 0 };
  return global_default_allocator;
}

memcached_return_t memcached_set_memory_allocators(memcached_st *shell,
                                                   memcached_malloc_fn mem_malloc,
                                                   memcached_free_fn mem_free,
                                                   memcached_realloc_fn mem_realloc,
                                                   memcached_calloc_fn mem_calloc,
                                                   void *context)
{
  Memcached* self= memcached2Memcached(shell);
  if (self == NULL)
  {
    return MEMCACHED_INVALID_ARGUMENTS;
  }

  /* All should be set, or none should be set */
  if (mem_malloc == NULL and mem_free == NULL and mem_realloc == NULL and mem_calloc == NULL) 
  {
    self->allocators= memcached_allocators_return_default();
  }
  else if (mem_malloc == NULL or mem_free == NULL or mem_realloc == NULL or mem_calloc == NULL)
  {
    return memcached_set_error(*self, MEMCACHED_INVALID_ARGUMENTS, MEMCACHED_AT, memcached_literal_param("NULL parameter provided for one or more allocators"));
  }
  else
  {
    self->allocators.malloc= mem_malloc;
    self->allocators.free= mem_free;
    self->allocators.realloc= mem_realloc;
    self->allocators.calloc= mem_calloc;
    self->allocators.context= context;
  }

  return MEMCACHED_SUCCESS;
}

void *memcached_get_memory_allocators_context(const memcached_st *shell)
{
  const Memcached* self= memcached2Memcached(shell);
  if (self)
  {
    return self->allocators.context;
  }

  return NULL;
}

void memcached_get_memory_allocators(const memcached_st *shell,
                                     memcached_malloc_fn *mem_malloc,
                                     memcached_free_fn *mem_free,
                                     memcached_realloc_fn *mem_realloc,
                                     memcached_calloc_fn *mem_calloc)
{
  const Memcached* self= memcached2Memcached(shell);
  if (self)
  {
    if (mem_malloc)
    {
      *mem_malloc= self->allocators.malloc;
    }

    if (mem_free)
    {
      *mem_free= self->allocators.free;
    }

    if (mem_realloc)
    {
      *mem_realloc= self->allocators.realloc;
    }

    if (mem_calloc)
    {
      *mem_calloc= self->allocators.calloc;
    }
  }
}
