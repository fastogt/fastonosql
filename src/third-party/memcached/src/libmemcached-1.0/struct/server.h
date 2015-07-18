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


#pragma once

#ifdef HAVE_NETDB_H
# include <netdb.h>
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

enum memcached_server_state_t {
  MEMCACHED_SERVER_STATE_NEW, // fd == -1, no address lookup has been done
  MEMCACHED_SERVER_STATE_ADDRINFO, // ADDRRESS information has been gathered
  MEMCACHED_SERVER_STATE_IN_PROGRESS,
  MEMCACHED_SERVER_STATE_CONNECTED,
  MEMCACHED_SERVER_STATE_IN_TIMEOUT,
  MEMCACHED_SERVER_STATE_DISABLED
};

struct memcached_server_st {
  struct {
    bool is_allocated:1;
    bool is_initialized:1;
    bool is_shutting_down:1;
    bool is_dead:1;
  } options;
  uint32_t number_of_hosts;
  uint32_t cursor_active;
  in_port_t port;
  uint32_t io_bytes_sent; /* # bytes sent since last read */
  uint32_t request_id;
  uint32_t server_failure_counter;
  uint64_t server_failure_counter_query_id;
  uint32_t server_timeout_counter;
  uint64_t server_timeout_counter_query_id;
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
  time_t next_retry;
  struct memcached_st *root;
  uint64_t limit_maxbytes;
  struct memcached_error_t *error_messages;
  char hostname[MEMCACHED_NI_MAXHOST];
};
