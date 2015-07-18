/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Libmemcached library
 *
 *  Copyright (C) 2012-2013 Data Differential, http://datadifferential.com/ 
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


#pragma once

#ifndef WIN32
# ifdef HAVE_NETDB_H
#  include <netdb.h>
# endif
#endif

#ifdef NI_MAXHOST
# define MEMCACHED_NI_MAXHOST NI_MAXHOST
#else
# define MEMCACHED_NI_MAXHOST 1025
#endif

#ifdef NI_MAXSERV
# define MEMCACHED_NI_MAXSERV NI_MAXSERV
#else
# define MEMCACHED_NI_MAXSERV 32
#endif

#include "libmemcached/string.hpp"

// @todo Complete class transformation
struct memcached_instance_st {
  in_port_t port() const
  {
    return port_;
  }

  void port(in_port_t arg)
  {
    port_= arg;
  }

  void mark_server_as_clean()
  {
    server_failure_counter= 0;
    server_timeout_counter= 0;
    next_retry= 0;
  }

  void disable()
  {
  }

  void enable()
  {
  }

  bool valid() const;

  bool is_shutting_down() const;

  void start_close_socket();
  void close_socket();
  void reset_socket();

  uint32_t response_count() const
  {
    return cursor_active_;
  }

  struct {
    bool is_allocated;
    bool is_initialized;
    bool is_shutting_down;
    bool is_dead;
    bool ready;
  } options;

  short _events;
  short _revents;

  short events(void)
  {
    return _events;
  }

  short revents(void)
  {
    return _revents;
  }

  const char* hostname()
  {
    return _hostname;
  }

  void hostname(const memcached_string_t& hostname_)
  {
    if (hostname_.size)
    {
      memcpy(_hostname, hostname_.c_str, hostname_.size);
      _hostname[hostname_.size]= 0;
    }
    else
    {
      memcpy(_hostname, memcached_literal_param("localhost"));
      _hostname[memcached_literal_param_size("localhost")]= 0;
    }
  }

  void events(short);
  void revents(short);

  uint32_t cursor_active_;
  in_port_t port_;
  memcached_socket_t fd;
  uint32_t io_bytes_sent; /* # bytes sent since last read */
  uint32_t request_id;
  uint32_t server_failure_counter;
  uint64_t server_failure_counter_query_id;
  uint32_t server_timeout_counter;
  uint32_t server_timeout_counter_query_id;
  uint32_t weight;
  uint32_t version;
  enum memcached_server_state_t state;
  struct {
    uint32_t read;
    uint32_t write;
    uint32_t timeouts;
    size_t _bytes_read;
  } io_wait_count;
  uint8_t major_version; // Default definition of UINT8_MAX means that it has not been set.
  uint8_t micro_version; // ditto, and note that this is the third, not second version bit
  uint8_t minor_version; // ditto
  memcached_connection_t type;
  char *read_ptr;
  size_t read_buffer_length;
  size_t read_data_length;
  size_t write_buffer_offset;
  struct addrinfo *address_info;
  struct addrinfo *address_info_next;
  time_t next_retry;
  struct memcached_st *root;
  uint64_t limit_maxbytes;
  struct memcached_error_t *error_messages;
  char read_buffer[MEMCACHED_MAX_BUFFER];
  char write_buffer[MEMCACHED_MAX_BUFFER];
  char _hostname[MEMCACHED_NI_MAXHOST];

  void clear_addrinfo()
  {
    if (address_info)
    {
      freeaddrinfo(address_info);
      address_info= NULL;
      address_info_next= NULL;
    }
  }
};

memcached_instance_st* __instance_create_with(memcached_st *memc,
                                              memcached_instance_st* self,
                                              const memcached_string_t& _hostname,
                                              const in_port_t port,
                                              uint32_t weight, 
                                              const memcached_connection_t type);

memcached_return_t memcached_instance_push(memcached_st *ptr, const memcached_instance_st*, uint32_t);

void __instance_free(memcached_instance_st *);
