/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Libmemcached library
 *
 *  Copyright (C) 2012 Data Differential, http://datadifferential.com/
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

enum memcached_flag_t
{
  MEMCACHED_FLAG_AUTO_EJECT_HOSTS,
  MEMCACHED_FLAG_BINARY_PROTOCOL,
  MEMCACHED_FLAG_BUFFER_REQUESTS,
  MEMCACHED_FLAG_HASH_WITH_NAMESPACE,
  MEMCACHED_FLAG_NO_BLOCK,
  MEMCACHED_FLAG_REPLY,
  MEMCACHED_FLAG_RANDOMIZE_REPLICA_READ,
  MEMCACHED_FLAG_SUPPORT_CAS,
  MEMCACHED_FLAG_TCP_NODELAY,
  MEMCACHED_FLAG_USE_SORT_HOSTS,
  MEMCACHED_FLAG_USE_UDP,
  MEMCACHED_FLAG_VERIFY_KEY,
  MEMCACHED_FLAG_TCP_KEEPALIVE,
  MEMCACHED_FLAG_IS_AES,
  MEMCACHED_FLAG_IS_FETCHING_VERSION
};

bool memcached_flag(const memcached_st&, const memcached_flag_t);
void memcached_flag(memcached_st&, const memcached_flag_t, const bool);
