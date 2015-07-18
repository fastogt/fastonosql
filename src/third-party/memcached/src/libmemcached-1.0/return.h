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

static inline bool memcached_success(memcached_return_t rc)
{
  return (rc == MEMCACHED_BUFFERED ||
          rc == MEMCACHED_DELETED ||
          rc == MEMCACHED_END || 
          rc == MEMCACHED_ITEM || 
          rc == MEMCACHED_STAT || 
          rc == MEMCACHED_STORED || 
          rc == MEMCACHED_SUCCESS || 
          rc == MEMCACHED_VALUE);
}

static inline bool memcached_failed(memcached_return_t rc)
{
  return (rc != MEMCACHED_SUCCESS && 
          rc != MEMCACHED_END && 
          rc != MEMCACHED_STORED && 
          rc != MEMCACHED_STAT && 
          rc != MEMCACHED_DELETED &&
          rc != MEMCACHED_BUFFERED &&
          rc != MEMCACHED_VALUE);
}

static inline bool memcached_fatal(memcached_return_t rc)
{
  return (
          rc != MEMCACHED_BUFFERED &&
          rc != MEMCACHED_CLIENT_ERROR &&
          rc != MEMCACHED_DATA_EXISTS &&
          rc != MEMCACHED_DELETED &&
          rc != MEMCACHED_E2BIG && 
          rc != MEMCACHED_END && 
          rc != MEMCACHED_ITEM &&
          rc != MEMCACHED_ERROR &&
          rc != MEMCACHED_NOTFOUND && 
          rc != MEMCACHED_NOTSTORED && 
          rc != MEMCACHED_SERVER_MEMORY_ALLOCATION_FAILURE && 
          rc != MEMCACHED_STAT && 
          rc != MEMCACHED_STORED && 
          rc != MEMCACHED_SUCCESS && 
          rc != MEMCACHED_VALUE);
}

#define memcached_continue(__memcached_return_t) ((__memcached_return_t) == MEMCACHED_IN_PROGRESS)
