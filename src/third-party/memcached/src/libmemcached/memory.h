/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Libmemcached library
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

#pragma once

#include <mem_config.h>

#include <libmemcached/common.h>

#ifdef __cplusplus
#include <cstddef>
#include <cstdlib>
#else
#include <stddef.h>
#include <stdlib.h>
#endif

static inline void libmemcached_free(const memcached_st *self, void *mem)
{
  if (self)
  {
    self->allocators.free(self, mem, self->allocators.context);
  }
  else if (mem)
  {
#ifdef __cplusplus
    std::free(mem);
#else
    free(mem);
#endif
  }
}

static inline void *libmemcached_malloc(const memcached_st *self, const size_t size)
{
  if (self)
  {
    return self->allocators.malloc(self, size, self->allocators.context);
  }

#ifdef __cplusplus
  return std::malloc(size);
#else
  return malloc(size);
#endif
}
#define libmemcached_xmalloc(__memcachd_st, __type) ((__type *)libmemcached_malloc((__memcachd_st), sizeof(__type)))

static inline void *libmemcached_realloc(const memcached_st *self, void *mem, size_t nmemb,  const size_t size)
{
  if (self)
  {
    return self->allocators.realloc(self, mem, nmemb * size, self->allocators.context);
  }

#ifdef __cplusplus
    return std::realloc(mem, size);
#else
    return realloc(mem, size);
#endif
}
#define libmemcached_xrealloc(__memcachd_st, __mem, __nelem, __type) ((__type *)libmemcached_realloc((__memcachd_st), (__mem), (__nelem), sizeof(__type)))
#define libmemcached_xvalloc(__memcachd_st, __nelem, __type) ((__type *)libmemcached_realloc((__memcachd_st), NULL, (__nelem), sizeof(__type)))

static inline void *libmemcached_calloc(const memcached_st *self, size_t nelem, size_t size)
{
  if (self)
  {
    return self->allocators.calloc(self, nelem, size, self->allocators.context);
  }

#ifdef __cplusplus
    return std::calloc(nelem, size);
#else
    return calloc(nelem, size);
#endif
}
#define libmemcached_xcalloc(__memcachd_st, __nelem, __type) ((__type *)libmemcached_calloc((__memcachd_st), (__nelem), sizeof(__type)))
