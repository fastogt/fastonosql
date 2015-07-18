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
#include <libmemcached/options.hpp>
#include <libmemcached/virtual_bucket.h>

#include <ctime>
#include <sys/types.h>

bool memcached_is_consistent_distribution(const Memcached* memc)
{
  switch (memc->distribution)
  {
  case MEMCACHED_DISTRIBUTION_CONSISTENT:
  case MEMCACHED_DISTRIBUTION_CONSISTENT_KETAMA:
  case MEMCACHED_DISTRIBUTION_CONSISTENT_KETAMA_SPY:
  case MEMCACHED_DISTRIBUTION_CONSISTENT_WEIGHTED:
    return true;

  case MEMCACHED_DISTRIBUTION_MODULA:
  case MEMCACHED_DISTRIBUTION_RANDOM:
  case MEMCACHED_DISTRIBUTION_VIRTUAL_BUCKET:
  case MEMCACHED_DISTRIBUTION_CONSISTENT_MAX:
    break;
  }

  return false;
}

/*
  This function is used to modify the behavior of running client.

  We quit all connections so we can reset the sockets.
*/

memcached_return_t memcached_behavior_set(memcached_st *shell,
                                          const memcached_behavior_t flag,
                                          uint64_t data)
{
  Memcached* ptr= memcached2Memcached(shell);
  if (ptr == NULL)
  {
    return MEMCACHED_INVALID_ARGUMENTS;
  }

  switch (flag)
  {
  case MEMCACHED_BEHAVIOR_NUMBER_OF_REPLICAS:
    ptr->number_of_replicas= (uint32_t)data;
    break;

  case MEMCACHED_BEHAVIOR_IO_MSG_WATERMARK:
    ptr->io_msg_watermark= (uint32_t) data;
    break;

  case MEMCACHED_BEHAVIOR_IO_BYTES_WATERMARK:
    ptr->io_bytes_watermark= (uint32_t)data;
    break;

  case MEMCACHED_BEHAVIOR_IO_KEY_PREFETCH:
    ptr->io_key_prefetch = (uint32_t)data;
    break;

  case MEMCACHED_BEHAVIOR_SND_TIMEOUT:
    ptr->snd_timeout= (int32_t)data;
    break;

  case MEMCACHED_BEHAVIOR_RCV_TIMEOUT:
    ptr->rcv_timeout= (int32_t)data;
    break;

  case MEMCACHED_BEHAVIOR_REMOVE_FAILED_SERVERS:
    ptr->flags.auto_eject_hosts= bool(data);

  case MEMCACHED_BEHAVIOR_SERVER_FAILURE_LIMIT:
    if (data == 0)
    {
      return memcached_set_error(*ptr, MEMCACHED_INVALID_ARGUMENTS, MEMCACHED_AT,
                                        memcached_literal_param("MEMCACHED_BEHAVIOR_SERVER_FAILURE_LIMIT requires a value greater then zero."));
    }
    ptr->server_failure_limit= uint32_t(data);
    break;

  case MEMCACHED_BEHAVIOR_SERVER_TIMEOUT_LIMIT:
    ptr->server_timeout_limit= uint32_t(data);
    break;

  case MEMCACHED_BEHAVIOR_BINARY_PROTOCOL:
    send_quit(ptr); // We need t shutdown all of the connections to make sure we do the correct protocol
    if (data)
    {
      ptr->flags.verify_key= false;
    }
    ptr->flags.binary_protocol= bool(data);
    break;

  case MEMCACHED_BEHAVIOR_SUPPORT_CAS:
    ptr->flags.support_cas= bool(data);
    break;

  case MEMCACHED_BEHAVIOR_NO_BLOCK:
    ptr->flags.no_block= bool(data);
    send_quit(ptr);
    break;

  case MEMCACHED_BEHAVIOR_BUFFER_REQUESTS:
    if (memcached_is_udp(ptr))
    {
      return memcached_set_error(*ptr, MEMCACHED_INVALID_ARGUMENTS, MEMCACHED_AT,
                                 memcached_literal_param("MEMCACHED_BEHAVIOR_BUFFER_REQUESTS cannot be set while MEMCACHED_BEHAVIOR_USE_UDP is enabled."));
    }
    ptr->flags.buffer_requests= bool(data);
    send_quit(ptr);
    break;

  case MEMCACHED_BEHAVIOR_USE_UDP:
    send_quit(ptr); // We need t shutdown all of the connections to make sure we do the correct protocol
    ptr->flags.use_udp= bool(data);
    if (bool(data))
    {
      ptr->flags.reply= false;
      ptr->flags.buffer_requests= false;
    }
    else
    {
      ptr->flags.reply= true;
    }
    break;

  case MEMCACHED_BEHAVIOR_TCP_NODELAY:
    ptr->flags.tcp_nodelay= bool(data);
    send_quit(ptr);
    break;

  case MEMCACHED_BEHAVIOR_TCP_KEEPALIVE:
    ptr->flags.tcp_keepalive= bool(data);
    send_quit(ptr);
    break;

  case MEMCACHED_BEHAVIOR_DISTRIBUTION:
    return memcached_behavior_set_distribution(ptr, (memcached_server_distribution_t)data);

  case MEMCACHED_BEHAVIOR_KETAMA:
    {
      if (data) // Turn on
      {
        return memcached_behavior_set_distribution(ptr, MEMCACHED_DISTRIBUTION_CONSISTENT_KETAMA);
      }

      return memcached_behavior_set_distribution(ptr, MEMCACHED_DISTRIBUTION_MODULA);
    }

  case MEMCACHED_BEHAVIOR_KETAMA_WEIGHTED:
    {
      if (bool(data) == false)
      {
        return memcached_behavior_set(ptr, MEMCACHED_BEHAVIOR_KETAMA, true);
      }

      (void)memcached_behavior_set_key_hash(ptr, MEMCACHED_HASH_MD5);
      (void)memcached_behavior_set_distribution_hash(ptr, MEMCACHED_HASH_MD5);
      /**
        @note We try to keep the same distribution going. This should be deprecated and rewritten.
      */
      return memcached_behavior_set_distribution(ptr, MEMCACHED_DISTRIBUTION_CONSISTENT_WEIGHTED);
    }

  case MEMCACHED_BEHAVIOR_HASH:
    return memcached_behavior_set_key_hash(ptr, (memcached_hash_t)(data));

  case MEMCACHED_BEHAVIOR_KETAMA_HASH:
    return memcached_behavior_set_distribution_hash(ptr, (memcached_hash_t)(data));

  case MEMCACHED_BEHAVIOR_CACHE_LOOKUPS:
    return memcached_set_error(*ptr, MEMCACHED_DEPRECATED, MEMCACHED_AT,
                                      memcached_literal_param("MEMCACHED_BEHAVIOR_CACHE_LOOKUPS has been deprecated."));

  case MEMCACHED_BEHAVIOR_VERIFY_KEY:
    if (ptr->flags.binary_protocol)
    {
      return memcached_set_error(*ptr, MEMCACHED_INVALID_ARGUMENTS, MEMCACHED_AT,
                                        memcached_literal_param("MEMCACHED_BEHAVIOR_VERIFY_KEY if the binary protocol has been enabled."));
    }
    ptr->flags.verify_key= bool(data);
    break;

  case MEMCACHED_BEHAVIOR_SORT_HOSTS:
    {
      ptr->flags.use_sort_hosts= bool(data);
      return run_distribution(ptr);
    }

  case MEMCACHED_BEHAVIOR_POLL_TIMEOUT:
    ptr->poll_timeout= (int32_t)data;
    break;

  case MEMCACHED_BEHAVIOR_CONNECT_TIMEOUT:
    ptr->connect_timeout= (int32_t)data;
    break;

  case MEMCACHED_BEHAVIOR_RETRY_TIMEOUT:
    ptr->retry_timeout= int32_t(data);
    break;

  case MEMCACHED_BEHAVIOR_DEAD_TIMEOUT:
    ptr->dead_timeout= int32_t(data);
    break;

  case MEMCACHED_BEHAVIOR_SOCKET_SEND_SIZE:
    ptr->send_size= (int32_t)data;
    send_quit(ptr);
    break;

  case MEMCACHED_BEHAVIOR_SOCKET_RECV_SIZE:
    ptr->recv_size= (int32_t)data;
    send_quit(ptr);
    break;

  case MEMCACHED_BEHAVIOR_TCP_KEEPIDLE:
    ptr->tcp_keepidle= (uint32_t)data;
    send_quit(ptr);
    break;

  case MEMCACHED_BEHAVIOR_USER_DATA:
    return memcached_set_error(*ptr, MEMCACHED_DEPRECATED, MEMCACHED_AT,
                               memcached_literal_param("MEMCACHED_BEHAVIOR_USER_DATA deprecated."));

  case MEMCACHED_BEHAVIOR_HASH_WITH_PREFIX_KEY:
    ptr->flags.hash_with_namespace= bool(data);
    break;

  case MEMCACHED_BEHAVIOR_NOREPLY:
    if (memcached_is_udp(ptr) and bool(data) == false)
    {
      return memcached_set_error(*ptr, MEMCACHED_INVALID_ARGUMENTS, MEMCACHED_AT,
                                 memcached_literal_param("MEMCACHED_BEHAVIOR_NOREPLY cannot be disabled while MEMCACHED_BEHAVIOR_USE_UDP is enabled."));
    }
    // We reverse the logic here to make it easier to understand throughout the
    // code.
    ptr->flags.reply= bool(data) ? false : true;
    break;

  case MEMCACHED_BEHAVIOR_AUTO_EJECT_HOSTS:
    ptr->flags.auto_eject_hosts= bool(data);
    break;

  case MEMCACHED_BEHAVIOR_RANDOMIZE_REPLICA_READ:
      srandom((uint32_t) time(NULL));
      ptr->flags.randomize_replica_read= bool(data);
      break;

  case MEMCACHED_BEHAVIOR_CORK:
      return memcached_set_error(*ptr, MEMCACHED_DEPRECATED, MEMCACHED_AT,
                                 memcached_literal_param("MEMCACHED_BEHAVIOR_CORK is now incorporated into the driver by default."));

  case MEMCACHED_BEHAVIOR_LOAD_FROM_FILE:
      return memcached_set_error(*ptr, MEMCACHED_INVALID_ARGUMENTS, MEMCACHED_AT,
                                 memcached_literal_param("MEMCACHED_BEHAVIOR_LOAD_FROM_FILE can not be set with memcached_behavior_set()"));

  case MEMCACHED_BEHAVIOR_MAX:
  default:
      /* Shouldn't get here */
      assert_msg(0, "Invalid behavior passed to memcached_behavior_set()");
      return memcached_set_error(*ptr, MEMCACHED_INVALID_ARGUMENTS, MEMCACHED_AT,
                                 memcached_literal_param("Invalid behavior passed to memcached_behavior_set()"));
  }

  return MEMCACHED_SUCCESS;
}

bool _is_auto_eject_host(const memcached_st *ptr)
{
  return ptr->flags.auto_eject_hosts;
}

uint64_t memcached_behavior_get(memcached_st *shell,
                                const memcached_behavior_t flag)
{
  Memcached* ptr= memcached2Memcached(shell);
  if (ptr == NULL)
  {
    return MEMCACHED_INVALID_ARGUMENTS;
  }

  switch (flag)
  {
  case MEMCACHED_BEHAVIOR_NUMBER_OF_REPLICAS:
    return ptr->number_of_replicas;

  case MEMCACHED_BEHAVIOR_IO_MSG_WATERMARK:
    return ptr->io_msg_watermark;

  case MEMCACHED_BEHAVIOR_IO_BYTES_WATERMARK:
    return ptr->io_bytes_watermark;

  case MEMCACHED_BEHAVIOR_IO_KEY_PREFETCH:
    return ptr->io_key_prefetch;

  case MEMCACHED_BEHAVIOR_BINARY_PROTOCOL:
    return ptr->flags.binary_protocol;

  case MEMCACHED_BEHAVIOR_SUPPORT_CAS:
    return ptr->flags.support_cas;

  case MEMCACHED_BEHAVIOR_CACHE_LOOKUPS:
    return true;

  case MEMCACHED_BEHAVIOR_NO_BLOCK:
    return ptr->flags.no_block;

  case MEMCACHED_BEHAVIOR_BUFFER_REQUESTS:
    return ptr->flags.buffer_requests;

  case MEMCACHED_BEHAVIOR_USE_UDP:
    return memcached_is_udp(ptr);

  case MEMCACHED_BEHAVIOR_TCP_NODELAY:
    return ptr->flags.tcp_nodelay;

  case MEMCACHED_BEHAVIOR_VERIFY_KEY:
    return ptr->flags.verify_key;

  case MEMCACHED_BEHAVIOR_KETAMA_WEIGHTED:
    if (memcached_is_consistent_distribution(ptr))
    {
      return memcached_is_weighted_ketama(ptr);
    }
    return false;

  case MEMCACHED_BEHAVIOR_DISTRIBUTION:
    return ptr->distribution;

  case MEMCACHED_BEHAVIOR_KETAMA:
    return memcached_is_consistent_distribution(ptr);

  case MEMCACHED_BEHAVIOR_HASH:
    return hashkit_get_function(&ptr->hashkit);

  case MEMCACHED_BEHAVIOR_KETAMA_HASH:
    return hashkit_get_function(&ptr->hashkit);

  case MEMCACHED_BEHAVIOR_REMOVE_FAILED_SERVERS:
  case MEMCACHED_BEHAVIOR_SERVER_FAILURE_LIMIT:
    return ptr->server_failure_limit;

  case MEMCACHED_BEHAVIOR_SERVER_TIMEOUT_LIMIT:
    return ptr->server_timeout_limit;

  case MEMCACHED_BEHAVIOR_SORT_HOSTS:
    return ptr->flags.use_sort_hosts;

  case MEMCACHED_BEHAVIOR_POLL_TIMEOUT:
    return (uint64_t)ptr->poll_timeout;

  case MEMCACHED_BEHAVIOR_CONNECT_TIMEOUT:
    return (uint64_t)ptr->connect_timeout;

  case MEMCACHED_BEHAVIOR_RETRY_TIMEOUT:
    return (uint64_t)ptr->retry_timeout;

  case MEMCACHED_BEHAVIOR_DEAD_TIMEOUT:
    return uint64_t(ptr->dead_timeout);

  case MEMCACHED_BEHAVIOR_SND_TIMEOUT:
    return (uint64_t)ptr->snd_timeout;

  case MEMCACHED_BEHAVIOR_RCV_TIMEOUT:
    return (uint64_t)ptr->rcv_timeout;

  case MEMCACHED_BEHAVIOR_TCP_KEEPIDLE:
    return (uint64_t)ptr->tcp_keepidle;

  case MEMCACHED_BEHAVIOR_SOCKET_SEND_SIZE:
    {
      int sock_size= 0;
      socklen_t sock_length= sizeof(int);

      if (ptr->send_size != -1) // If value is -1 then we are using the default
      {
        return (uint64_t) ptr->send_size;
      }

      memcached_instance_st* instance= memcached_instance_fetch(ptr, 0);

      if (instance) // If we have an instance we test, otherwise we just set and pray
      {
        /* REFACTOR */
        /* We just try the first host, and if it is down we return zero */
        if (memcached_failed(memcached_connect(instance)))
        {
          return 0;
        }

        if (memcached_failed(memcached_io_wait_for_write(instance)))
        {
          return 0;
        }

        if (getsockopt(instance->fd, SOL_SOCKET, SO_SNDBUF, (char*)&sock_size, &sock_length) < 0)
        {
          memcached_set_errno(*ptr, get_socket_errno(), MEMCACHED_AT);
          return 0; /* Zero means error */
        }
      }

      return (uint64_t) sock_size;
    }

  case MEMCACHED_BEHAVIOR_SOCKET_RECV_SIZE:
    {
      int sock_size= 0;
      socklen_t sock_length= sizeof(int);

      if (ptr->recv_size != -1) // If value is -1 then we are using the default
        return (uint64_t) ptr->recv_size;

      memcached_instance_st* instance= memcached_instance_fetch(ptr, 0);

      /**
        @note REFACTOR
      */
      if (instance)
      {
        /* We just try the first host, and if it is down we return zero */
        if (memcached_failed(memcached_connect(instance)))
        {
          return 0;
        }

        if (memcached_failed(memcached_io_wait_for_write(instance)))
        {
          return 0;
        }

        if (getsockopt(instance->fd, SOL_SOCKET, SO_RCVBUF, (char*)&sock_size, &sock_length) < 0)
        {
          memcached_set_errno(*ptr, get_socket_errno(), MEMCACHED_AT);
          return 0; /* Zero means error */
        }
      }

      return (uint64_t) sock_size;
    }

  case MEMCACHED_BEHAVIOR_USER_DATA:
    memcached_set_error(*ptr, MEMCACHED_DEPRECATED, MEMCACHED_AT,
                        memcached_literal_param("MEMCACHED_BEHAVIOR_USER_DATA deprecated."));
    return 0;

  case MEMCACHED_BEHAVIOR_HASH_WITH_PREFIX_KEY:
    return ptr->flags.hash_with_namespace;

  case MEMCACHED_BEHAVIOR_NOREPLY:
    return ptr->flags.reply ? false : true;

  case MEMCACHED_BEHAVIOR_AUTO_EJECT_HOSTS:
    return ptr->flags.auto_eject_hosts;

  case MEMCACHED_BEHAVIOR_RANDOMIZE_REPLICA_READ:
    return ptr->flags.randomize_replica_read;

  case MEMCACHED_BEHAVIOR_CORK:
#ifdef HAVE_MSG_MORE
    return true;
#else
    return false;
#endif

  case MEMCACHED_BEHAVIOR_TCP_KEEPALIVE:
    return ptr->flags.tcp_keepalive;

  case MEMCACHED_BEHAVIOR_LOAD_FROM_FILE:
    return bool(memcached_parse_filename(ptr));

  case MEMCACHED_BEHAVIOR_MAX:
  default:
    assert_msg(0, "Invalid behavior passed to memcached_behavior_get()");
    return 0;
  }

  /* NOTREACHED */
}


memcached_return_t memcached_behavior_set_distribution(memcached_st *shell, memcached_server_distribution_t type)
{
  Memcached* ptr= memcached2Memcached(shell);
  if (ptr)
  {
    switch (type)
    {
    case MEMCACHED_DISTRIBUTION_MODULA:
      break;

    case MEMCACHED_DISTRIBUTION_CONSISTENT:
    case MEMCACHED_DISTRIBUTION_CONSISTENT_KETAMA:
      memcached_set_weighted_ketama(ptr, false);
      break;

    case MEMCACHED_DISTRIBUTION_RANDOM:
      break;

    case MEMCACHED_DISTRIBUTION_CONSISTENT_KETAMA_SPY:
      break;

    case MEMCACHED_DISTRIBUTION_CONSISTENT_WEIGHTED:
      memcached_set_weighted_ketama(ptr, true);
      break;

    case MEMCACHED_DISTRIBUTION_VIRTUAL_BUCKET:
      break;

    default:
    case MEMCACHED_DISTRIBUTION_CONSISTENT_MAX:
      return memcached_set_error(*ptr, MEMCACHED_INVALID_ARGUMENTS, MEMCACHED_AT,
                                 memcached_literal_param("Invalid memcached_server_distribution_t"));
    }
    ptr->distribution= type;

    return run_distribution(ptr);
  }

  return MEMCACHED_INVALID_ARGUMENTS;
}


memcached_server_distribution_t memcached_behavior_get_distribution(memcached_st *shell)
{
  Memcached* ptr= memcached2Memcached(shell);
  if (ptr)
  {
    return ptr->distribution;
  }

  return MEMCACHED_DISTRIBUTION_CONSISTENT_MAX;
}

memcached_return_t memcached_behavior_set_key_hash(memcached_st *shell, memcached_hash_t type)
{
  Memcached* ptr= memcached2Memcached(shell);
  if (ptr)
  {
    if (hashkit_success(hashkit_set_function(&ptr->hashkit, (hashkit_hash_algorithm_t)type)))
    {
      return MEMCACHED_SUCCESS;
    }

    return memcached_set_error(*ptr, MEMCACHED_INVALID_ARGUMENTS, MEMCACHED_AT,
                               memcached_literal_param("Invalid memcached_hash_t()"));
  }

  return MEMCACHED_INVALID_ARGUMENTS;
}

memcached_hash_t memcached_behavior_get_key_hash(memcached_st *shell)
{
  Memcached* ptr= memcached2Memcached(shell);
  if (ptr)
  {
    return (memcached_hash_t)hashkit_get_function(&ptr->hashkit);
  }

  return MEMCACHED_HASH_MAX;
}

memcached_return_t memcached_behavior_set_distribution_hash(memcached_st *shell, memcached_hash_t type)
{
  Memcached* ptr= memcached2Memcached(shell);
  if (ptr)
  {
    if (hashkit_success(hashkit_set_distribution_function(&ptr->hashkit, (hashkit_hash_algorithm_t)type)))
    {
      return MEMCACHED_SUCCESS;
    }

    return memcached_set_error(*ptr, MEMCACHED_INVALID_ARGUMENTS, MEMCACHED_AT,
                               memcached_literal_param("Invalid memcached_hash_t()"));
  }

  return MEMCACHED_INVALID_ARGUMENTS;
}

memcached_hash_t memcached_behavior_get_distribution_hash(memcached_st *shell)
{
  Memcached* ptr= memcached2Memcached(shell);
  if (ptr)
  {
    return (memcached_hash_t)hashkit_get_function(&ptr->hashkit);
  }

  return MEMCACHED_HASH_MAX;
}

const char *libmemcached_string_behavior(const memcached_behavior_t flag)
{
  switch (flag)
  {
  case MEMCACHED_BEHAVIOR_SERVER_TIMEOUT_LIMIT: return "MEMCACHED_BEHAVIOR_SERVER_TIMEOUT_LIMIT";
  case MEMCACHED_BEHAVIOR_NO_BLOCK: return "MEMCACHED_BEHAVIOR_NO_BLOCK";
  case MEMCACHED_BEHAVIOR_TCP_NODELAY: return "MEMCACHED_BEHAVIOR_TCP_NODELAY";
  case MEMCACHED_BEHAVIOR_HASH: return "MEMCACHED_BEHAVIOR_HASH";
  case MEMCACHED_BEHAVIOR_KETAMA: return "MEMCACHED_BEHAVIOR_KETAMA";
  case MEMCACHED_BEHAVIOR_SOCKET_SEND_SIZE: return "MEMCACHED_BEHAVIOR_SOCKET_SEND_SIZE";
  case MEMCACHED_BEHAVIOR_SOCKET_RECV_SIZE: return "MEMCACHED_BEHAVIOR_SOCKET_RECV_SIZE";
  case MEMCACHED_BEHAVIOR_CACHE_LOOKUPS: return "MEMCACHED_BEHAVIOR_CACHE_LOOKUPS";
  case MEMCACHED_BEHAVIOR_SUPPORT_CAS: return "MEMCACHED_BEHAVIOR_SUPPORT_CAS";
  case MEMCACHED_BEHAVIOR_POLL_TIMEOUT: return "MEMCACHED_BEHAVIOR_POLL_TIMEOUT";
  case MEMCACHED_BEHAVIOR_DISTRIBUTION: return "MEMCACHED_BEHAVIOR_DISTRIBUTION";
  case MEMCACHED_BEHAVIOR_BUFFER_REQUESTS: return "MEMCACHED_BEHAVIOR_BUFFER_REQUESTS";
  case MEMCACHED_BEHAVIOR_USER_DATA: return "MEMCACHED_BEHAVIOR_USER_DATA";
  case MEMCACHED_BEHAVIOR_SORT_HOSTS: return "MEMCACHED_BEHAVIOR_SORT_HOSTS";
  case MEMCACHED_BEHAVIOR_VERIFY_KEY: return "MEMCACHED_BEHAVIOR_VERIFY_KEY";
  case MEMCACHED_BEHAVIOR_CONNECT_TIMEOUT: return "MEMCACHED_BEHAVIOR_CONNECT_TIMEOUT";
  case MEMCACHED_BEHAVIOR_RETRY_TIMEOUT: return "MEMCACHED_BEHAVIOR_RETRY_TIMEOUT";
  case MEMCACHED_BEHAVIOR_DEAD_TIMEOUT: return "MEMCACHED_BEHAVIOR_DEAD_TIMEOUT";
  case MEMCACHED_BEHAVIOR_KETAMA_WEIGHTED: return "MEMCACHED_BEHAVIOR_KETAMA_WEIGHTED";
  case MEMCACHED_BEHAVIOR_KETAMA_HASH: return "MEMCACHED_BEHAVIOR_KETAMA_HASH";
  case MEMCACHED_BEHAVIOR_BINARY_PROTOCOL: return "MEMCACHED_BEHAVIOR_BINARY_PROTOCOL";
  case MEMCACHED_BEHAVIOR_SND_TIMEOUT: return "MEMCACHED_BEHAVIOR_SND_TIMEOUT";
  case MEMCACHED_BEHAVIOR_RCV_TIMEOUT: return "MEMCACHED_BEHAVIOR_RCV_TIMEOUT";
  case MEMCACHED_BEHAVIOR_SERVER_FAILURE_LIMIT: return "MEMCACHED_BEHAVIOR_SERVER_FAILURE_LIMIT";
  case MEMCACHED_BEHAVIOR_IO_MSG_WATERMARK: return "MEMCACHED_BEHAVIOR_IO_MSG_WATERMARK";
  case MEMCACHED_BEHAVIOR_IO_BYTES_WATERMARK: return "MEMCACHED_BEHAVIOR_IO_BYTES_WATERMARK";
  case MEMCACHED_BEHAVIOR_IO_KEY_PREFETCH: return "MEMCACHED_BEHAVIOR_IO_KEY_PREFETCH";
  case MEMCACHED_BEHAVIOR_HASH_WITH_PREFIX_KEY: return "MEMCACHED_BEHAVIOR_HASH_WITH_PREFIX_KEY";
  case MEMCACHED_BEHAVIOR_NOREPLY: return "MEMCACHED_BEHAVIOR_NOREPLY";
  case MEMCACHED_BEHAVIOR_USE_UDP: return "MEMCACHED_BEHAVIOR_USE_UDP";
  case MEMCACHED_BEHAVIOR_AUTO_EJECT_HOSTS: return "MEMCACHED_BEHAVIOR_AUTO_EJECT_HOSTS";
  case MEMCACHED_BEHAVIOR_REMOVE_FAILED_SERVERS: return "MEMCACHED_BEHAVIOR_REMOVE_FAILED_SERVERS";
  case MEMCACHED_BEHAVIOR_NUMBER_OF_REPLICAS: return "MEMCACHED_BEHAVIOR_NUMBER_OF_REPLICAS";
  case MEMCACHED_BEHAVIOR_RANDOMIZE_REPLICA_READ: return "MEMCACHED_BEHAVIOR_RANDOMIZE_REPLICA_READ";
  case MEMCACHED_BEHAVIOR_CORK: return "MEMCACHED_BEHAVIOR_CORK";
  case MEMCACHED_BEHAVIOR_TCP_KEEPALIVE: return "MEMCACHED_BEHAVIOR_TCP_KEEPALIVE";
  case MEMCACHED_BEHAVIOR_TCP_KEEPIDLE: return "MEMCACHED_BEHAVIOR_TCP_KEEPIDLE";
  case MEMCACHED_BEHAVIOR_LOAD_FROM_FILE: return "MEMCACHED_BEHAVIOR_LOAD_FROM_FILE";
  default:
  case MEMCACHED_BEHAVIOR_MAX: return "INVALID memcached_behavior_t";
  }
}

const char *libmemcached_string_distribution(const memcached_server_distribution_t flag)
{
  switch (flag)
  {
  case MEMCACHED_DISTRIBUTION_MODULA: return "MEMCACHED_DISTRIBUTION_MODULA";
  case MEMCACHED_DISTRIBUTION_CONSISTENT: return "MEMCACHED_DISTRIBUTION_CONSISTENT";
  case MEMCACHED_DISTRIBUTION_CONSISTENT_KETAMA: return "MEMCACHED_DISTRIBUTION_CONSISTENT_KETAMA";
  case MEMCACHED_DISTRIBUTION_RANDOM: return "MEMCACHED_DISTRIBUTION_RANDOM";
  case MEMCACHED_DISTRIBUTION_CONSISTENT_KETAMA_SPY: return "MEMCACHED_DISTRIBUTION_CONSISTENT_KETAMA_SPY";
  case MEMCACHED_DISTRIBUTION_CONSISTENT_WEIGHTED: return "MEMCACHED_DISTRIBUTION_CONSISTENT_WEIGHTED";
  case MEMCACHED_DISTRIBUTION_VIRTUAL_BUCKET: return "MEMCACHED_DISTRIBUTION_VIRTUAL_BUCKET";
  default:
  case MEMCACHED_DISTRIBUTION_CONSISTENT_MAX: return "INVALID memcached_server_distribution_t";
  }
}

memcached_return_t memcached_bucket_set(memcached_st *shell,
                                        const uint32_t *host_map,
                                        const uint32_t *forward_map,
                                        const uint32_t buckets,
                                        const uint32_t replicas)
{
  Memcached* self= memcached2Memcached(shell);
  memcached_return_t rc;

  if (self == NULL)
  {
    return MEMCACHED_INVALID_ARGUMENTS;
  }

  if (host_map == NULL)
  {
    return MEMCACHED_INVALID_ARGUMENTS;
  }

  memcached_server_distribution_t old= memcached_behavior_get_distribution(self);

  if (memcached_failed(rc =memcached_behavior_set_distribution(self, MEMCACHED_DISTRIBUTION_VIRTUAL_BUCKET)))
  {
    return rc;
  }

  if (memcached_failed(rc= memcached_virtual_bucket_create(self, host_map, forward_map, buckets, replicas)))
  {
    memcached_behavior_set_distribution(self, old);
  }

  return rc;
}
