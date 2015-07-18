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

/*
  This is a partial implementation for fetching/creating memcached_server_st objects.
*/
#include <libmemcached/common.h>

static inline void _server_init(memcached_server_st *self, Memcached *root,
                                const memcached_string_t& hostname,
                                in_port_t port,
                                uint32_t weight, memcached_connection_t type)
{
  self->options.is_shutting_down= false;
  self->options.is_dead= false;
  self->number_of_hosts= 0;
  self->cursor_active= 0;
  self->port= port;
  self->io_bytes_sent= 0;
  self->request_id= 0;
  self->server_failure_counter= 0;
  self->server_failure_counter_query_id= 0;
  self->server_timeout_counter= 0;
  self->server_timeout_counter_query_id= 0;
  self->weight= weight ? weight : 1; // 1 is the default weight value
  self->io_wait_count.read= 0;
  self->io_wait_count.write= 0;
  self->io_wait_count.timeouts= 0;
  self->io_wait_count._bytes_read= 0;
  self->major_version= UINT8_MAX;
  self->micro_version= UINT8_MAX;
  self->minor_version= UINT8_MAX;
  self->type= type;
  self->error_messages= NULL;

  self->state= MEMCACHED_SERVER_STATE_NEW;
  self->next_retry= 0;

  self->root= root;
  if (root)
  {
    self->version= ++root->server_info.version;
  }
  else
  {
    self->version= UINT_MAX;
  }
  self->limit_maxbytes= 0;
  memcpy(self->hostname, hostname.c_str, hostname.size);
  self->hostname[hostname.size]= 0;
}

static memcached_server_st *_server_create(memcached_server_st *self, const Memcached *memc)
{
  if (self == NULL)
  {
   self= libmemcached_xmalloc(memc, struct memcached_server_st);

    if (self == NULL)
    {
      return NULL; /*  MEMCACHED_MEMORY_ALLOCATION_FAILURE */
    }

    self->options.is_allocated= true;
  }
  else
  {
    self->options.is_allocated= false;
  }

  self->options.is_initialized= true;

  return self;
}

memcached_server_st *__server_create_with(Memcached *memc,
                                          memcached_server_st* allocated_instance,
                                          const memcached_string_t& hostname,
                                          const in_port_t port,
                                          uint32_t weight, 
                                          const memcached_connection_t type)
{
  if (memcached_is_valid_servername(hostname) == false)
  {
    memcached_set_error(*memc, MEMCACHED_INVALID_ARGUMENTS, MEMCACHED_AT, memcached_literal_param("Invalid hostname provided"));
    return NULL;
  }

  allocated_instance= _server_create(allocated_instance, memc);

  if (allocated_instance == NULL)
  {
    return NULL;
  }

  _server_init(allocated_instance, const_cast<Memcached *>(memc), hostname, port, weight, type);

  return allocated_instance;
}

void __server_free(memcached_server_st *self)
{
  memcached_error_free(*self);

  if (memcached_is_allocated(self))
  {
    libmemcached_free(self->root, self);
  }
  else
  {
    self->options.is_initialized= false;
  }
}

void memcached_server_free(memcached_server_st *self)
{
  if (self == NULL)
  {
    return;
  }

  if (memcached_server_list_count(self))
  {
    memcached_server_list_free(self);
    return;
  }

  __server_free(self);
}

void memcached_server_error_reset(memcached_server_st *self)
{
  WATCHPOINT_ASSERT(self);
  if (self == NULL)
  {
    return;
  }

  memcached_error_free(*self);
}

uint32_t memcached_servers_set_count(memcached_server_st *servers, uint32_t count)
{
  WATCHPOINT_ASSERT(servers);
  if (servers == NULL)
  {
    return 0;
  }

  return servers->number_of_hosts= count;
}

uint32_t memcached_server_count(const memcached_st *self)
{
  WATCHPOINT_ASSERT(self);
  if (self == NULL)
    return 0;

  return self->number_of_hosts;
}

const char *memcached_server_name(const memcached_instance_st * self)
{
  WATCHPOINT_ASSERT(self);
  if (self)
  {
    return self->_hostname;
  }

  return NULL;
}

in_port_t memcached_server_port(const memcached_instance_st * self)
{
  WATCHPOINT_ASSERT(self);
  if (self == NULL)
  {
    return 0;
  }

  return self->port();
}

in_port_t memcached_server_srcport(const memcached_instance_st * self)
{
  WATCHPOINT_ASSERT(self);
  if (self == NULL || self->fd == INVALID_SOCKET || (self->type != MEMCACHED_CONNECTION_TCP && self->type != MEMCACHED_CONNECTION_UDP))
  {
    return 0;
  }

  struct sockaddr_in sin;
  socklen_t addrlen= sizeof(sin);
  if (getsockname(self->fd, (struct sockaddr*)&sin, &addrlen) != -1)
  {
    return ntohs(sin.sin_port);
  }

  return -1;
}

uint32_t memcached_server_response_count(const memcached_instance_st * self)
{
  WATCHPOINT_ASSERT(self);
  if (self == NULL)
  {
    return 0;
  }

  return self->cursor_active_;
}

const char *memcached_server_type(const memcached_instance_st * ptr)
{
  if (ptr)
  {
    switch (ptr->type)
    {
    case MEMCACHED_CONNECTION_TCP:
      return "TCP";

    case MEMCACHED_CONNECTION_UDP:
      return "UDP";

    case MEMCACHED_CONNECTION_UNIX_SOCKET:
      return "SOCKET";
    }
  }

  return "UNKNOWN";
}

uint8_t memcached_server_major_version(const memcached_instance_st * instance)
{
  if (instance)
  {
    return instance->major_version;
  }

  return UINT8_MAX;
}

uint8_t memcached_server_minor_version(const memcached_instance_st * instance)
{
  if (instance)
  {
    return instance->minor_version;
  }

  return UINT8_MAX;
}

uint8_t memcached_server_micro_version(const memcached_instance_st * instance)
{
  if (instance)
  {
    return instance->micro_version;
  }

  return UINT8_MAX;
}
