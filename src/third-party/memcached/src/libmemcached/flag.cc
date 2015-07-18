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

#include <libmemcached/common.h>

bool memcached_flag(const memcached_st& memc, const memcached_flag_t flag)
{
  switch (flag)
  {
  case MEMCACHED_FLAG_AUTO_EJECT_HOSTS:
    return memcached_is_auto_eject_hosts(&memc);

  case MEMCACHED_FLAG_BINARY_PROTOCOL:
    return memcached_is_binary(&memc);

  case MEMCACHED_FLAG_BUFFER_REQUESTS:
    return memcached_is_buffering(&memc);

  case MEMCACHED_FLAG_HASH_WITH_NAMESPACE:
    return memcached_is_hash_with_namespace(&memc);

  case MEMCACHED_FLAG_NO_BLOCK:
    return memcached_is_no_block(&memc);

  case MEMCACHED_FLAG_REPLY:
    return memcached_is_replying(&memc);

  case MEMCACHED_FLAG_RANDOMIZE_REPLICA_READ:
    return memcached_is_randomize_replica_read(&memc);

  case MEMCACHED_FLAG_SUPPORT_CAS:
    return memcached_is_cas(&memc);

  case MEMCACHED_FLAG_TCP_NODELAY:
    return memcached_is_tcp_nodelay(&memc);

  case MEMCACHED_FLAG_USE_SORT_HOSTS:
    return memcached_is_use_sort_hosts(&memc);

  case MEMCACHED_FLAG_USE_UDP:
    return memcached_is_udp(&memc);

  case MEMCACHED_FLAG_VERIFY_KEY:
    return memcached_is_verify_key(&memc);

  case MEMCACHED_FLAG_TCP_KEEPALIVE:
    return memcached_is_use_sort_hosts(&memc);

  case MEMCACHED_FLAG_IS_AES:
    return memcached_is_aes(&memc);

  case MEMCACHED_FLAG_IS_FETCHING_VERSION:
    return memcached_is_fetching_version(&memc);
  }

  abort();
}

void memcached_flag(memcached_st& memc, const memcached_flag_t flag, const bool arg)
{
  switch (flag)
  {
  case MEMCACHED_FLAG_AUTO_EJECT_HOSTS:
    memcached_set_auto_eject_hosts(memc, arg);
    break;

  case MEMCACHED_FLAG_BINARY_PROTOCOL:
    memcached_set_binary(memc, arg);
    break;

  case MEMCACHED_FLAG_BUFFER_REQUESTS:
    memcached_set_buffering(memc, arg);
    break;

  case MEMCACHED_FLAG_HASH_WITH_NAMESPACE:
    memcached_set_hash_with_namespace(memc, arg);
    break;

  case MEMCACHED_FLAG_NO_BLOCK:
    memcached_set_no_block(memc, arg);
    break;

  case MEMCACHED_FLAG_REPLY:
    memcached_set_replying(memc, arg);
    break;

  case MEMCACHED_FLAG_RANDOMIZE_REPLICA_READ:
    memcached_set_randomize_replica_read(memc, arg);
    break;

  case MEMCACHED_FLAG_SUPPORT_CAS:
    memcached_set_cas(memc, arg);
    break;

  case MEMCACHED_FLAG_TCP_NODELAY:
    memcached_set_tcp_nodelay(memc, arg);
    break;

  case MEMCACHED_FLAG_USE_SORT_HOSTS:
    memcached_set_use_sort_hosts(memc, arg);
    break;

  case MEMCACHED_FLAG_USE_UDP:
    memcached_set_udp(memc, arg);
    break;

  case MEMCACHED_FLAG_VERIFY_KEY:
    memcached_set_verify_key(memc, arg);
    break;

  case MEMCACHED_FLAG_TCP_KEEPALIVE:
    memcached_set_use_sort_hosts(memc, arg);
    break;

  case MEMCACHED_FLAG_IS_AES:
    memcached_set_aes(memc, arg);
    break;

  case MEMCACHED_FLAG_IS_FETCHING_VERSION:
    memcached_set_fetching_version(memc, arg);
    break;
  }
}
