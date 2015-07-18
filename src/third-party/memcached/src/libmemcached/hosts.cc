/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Libmemcached library
 *
 *  Copyright (C) 2011 Data Differential, http://datadifferential.com/
 *  Copyright (C) 2006-2010 Brian Aker All rights reserved.
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
#include "libmemcached/assert.hpp"

#include <cmath>
#include <sys/time.h>

/* Protoypes (static) */
static memcached_return_t update_continuum(Memcached *ptr);

static int compare_servers(const void *p1, const void *p2)
{
  const memcached_instance_st * a= (const memcached_instance_st *)p1;
  const memcached_instance_st * b= (const memcached_instance_st *)p2;

  int return_value= strcmp(a->_hostname, b->_hostname);

  if (return_value == 0)
  {
    return_value= int(a->port() - b->port());
  }

  return return_value;
}

static void sort_hosts(Memcached *ptr)
{
  if (memcached_server_count(ptr))
  {
    qsort(memcached_instance_list(ptr), memcached_server_count(ptr), sizeof(memcached_instance_st), compare_servers);
  }
}


memcached_return_t run_distribution(Memcached *ptr)
{
  if (ptr->flags.use_sort_hosts)
  {
    sort_hosts(ptr);
  }

  switch (ptr->distribution)
  {
  case MEMCACHED_DISTRIBUTION_CONSISTENT:
  case MEMCACHED_DISTRIBUTION_CONSISTENT_KETAMA:
  case MEMCACHED_DISTRIBUTION_CONSISTENT_KETAMA_SPY:
  case MEMCACHED_DISTRIBUTION_CONSISTENT_WEIGHTED:
    return update_continuum(ptr);

  case MEMCACHED_DISTRIBUTION_VIRTUAL_BUCKET:
  case MEMCACHED_DISTRIBUTION_MODULA:
    break;

  case MEMCACHED_DISTRIBUTION_RANDOM:
    srandom((uint32_t) time(NULL));
    break;

  case MEMCACHED_DISTRIBUTION_CONSISTENT_MAX:
  default:
    assert_msg(0, "Invalid distribution type passed to run_distribution()");
  }

  return MEMCACHED_SUCCESS;
}

static uint32_t ketama_server_hash(const char *key, size_t key_length, uint32_t alignment)
{
  unsigned char results[16];

  libhashkit_md5_signature((unsigned char*)key, key_length, results);

  return ((uint32_t) (results[3 + alignment * 4] & 0xFF) << 24)
    | ((uint32_t) (results[2 + alignment * 4] & 0xFF) << 16)
    | ((uint32_t) (results[1 + alignment * 4] & 0xFF) << 8)
    | (results[0 + alignment * 4] & 0xFF);
}

static int continuum_item_cmp(const void *t1, const void *t2)
{
  memcached_continuum_item_st *ct1= (memcached_continuum_item_st *)t1;
  memcached_continuum_item_st *ct2= (memcached_continuum_item_st *)t2;

  /* Why 153? Hmmm... */
  WATCHPOINT_ASSERT(ct1->value != 153);
  if (ct1->value == ct2->value)
  {
    return 0;
  }
  else if (ct1->value > ct2->value)
  {
    return 1;
  }
  else
  {
    return -1;
  }
}

static memcached_return_t update_continuum(Memcached *ptr)
{
  uint32_t continuum_index= 0;
  uint32_t pointer_counter= 0;
  uint32_t pointer_per_server= MEMCACHED_POINTS_PER_SERVER;
  uint32_t pointer_per_hash= 1;
  uint32_t live_servers= 0;
  struct timeval now;

  if (gettimeofday(&now, NULL))
  {
    return memcached_set_errno(*ptr, errno, MEMCACHED_AT);
  }

  memcached_instance_st* list= memcached_instance_list(ptr);

  /* count live servers (those without a retry delay set) */
  bool is_auto_ejecting= _is_auto_eject_host(ptr);
  if (is_auto_ejecting)
  {
    live_servers= 0;
    ptr->ketama.next_distribution_rebuild= 0;
    for (uint32_t host_index= 0; host_index < memcached_server_count(ptr); ++host_index)
    {
      if (list[host_index].next_retry <= now.tv_sec)
      {
        live_servers++;
      }
      else
      {
        if (ptr->ketama.next_distribution_rebuild == 0 or list[host_index].next_retry < ptr->ketama.next_distribution_rebuild)
        {
          ptr->ketama.next_distribution_rebuild= list[host_index].next_retry;
        }
      }
    }
  }
  else
  {
    live_servers= memcached_server_count(ptr);
  }

  uint32_t points_per_server= (uint32_t) (memcached_is_weighted_ketama(ptr) ? MEMCACHED_POINTS_PER_SERVER_KETAMA : MEMCACHED_POINTS_PER_SERVER);

  if (live_servers == 0)
  {
    return MEMCACHED_SUCCESS;
  }

  if (live_servers > ptr->ketama.continuum_count)
  {
    memcached_continuum_item_st *new_ptr;

    new_ptr= libmemcached_xrealloc(ptr, ptr->ketama.continuum, (live_servers + MEMCACHED_CONTINUUM_ADDITION) * points_per_server, memcached_continuum_item_st);

    if (new_ptr == 0)
    {
      return MEMCACHED_MEMORY_ALLOCATION_FAILURE;
    }

    ptr->ketama.continuum= new_ptr;
    ptr->ketama.continuum_count= live_servers + MEMCACHED_CONTINUUM_ADDITION;
  }
  assert_msg(ptr->ketama.continuum, "Programmer Error, empty ketama continuum");

  uint64_t total_weight= 0;
  if (memcached_is_weighted_ketama(ptr))
  {
    for (uint32_t host_index = 0; host_index < memcached_server_count(ptr); ++host_index)
    {
      if (is_auto_ejecting == false or list[host_index].next_retry <= now.tv_sec)
      {
        total_weight += list[host_index].weight;
      }
    }
  }

  for (uint32_t host_index= 0; host_index < memcached_server_count(ptr); ++host_index)
  {
    if (is_auto_ejecting and list[host_index].next_retry > now.tv_sec)
    {
      continue;
    }

    if (memcached_is_weighted_ketama(ptr))
    {
        float pct= (float)list[host_index].weight / (float)total_weight;
        pointer_per_server= (uint32_t) ((::floor((float) (pct * MEMCACHED_POINTS_PER_SERVER_KETAMA / 4 * (float)live_servers + 0.0000000001))) * 4);
        pointer_per_hash= 4;
        if (DEBUG)
        {
          printf("ketama_weighted:%s|%d|%llu|%u\n",
                 list[host_index]._hostname,
                 list[host_index].port(),
                 (unsigned long long)list[host_index].weight,
                 pointer_per_server);
        }
    }


    if (ptr->distribution == MEMCACHED_DISTRIBUTION_CONSISTENT_KETAMA_SPY)
    {
      for (uint32_t pointer_index= 0;
           pointer_index < pointer_per_server / pointer_per_hash;
           pointer_index++)
      {
        char sort_host[1 +MEMCACHED_NI_MAXHOST +1 +MEMCACHED_NI_MAXSERV +1 + MEMCACHED_NI_MAXSERV ]= "";
        int sort_host_length;

        // Spymemcached ketema key format is: hostname/ip:port-index
        // If hostname is not available then: /ip:port-index
        sort_host_length= snprintf(sort_host, sizeof(sort_host),
                                   "/%s:%u-%u",
                                   list[host_index]._hostname,
                                   (uint32_t)list[host_index].port(),
                                   pointer_index);

        if (size_t(sort_host_length) >= sizeof(sort_host) or sort_host_length < 0)
        {
          return memcached_set_error(*ptr, MEMCACHED_MEMORY_ALLOCATION_FAILURE, MEMCACHED_AT, 
                                     memcached_literal_param("snprintf(sizeof(sort_host))"));
        }

        if (DEBUG)
        {
          fprintf(stdout, "update_continuum: key is %s\n", sort_host);
        }

        if (memcached_is_weighted_ketama(ptr))
        {
          for (uint32_t x= 0; x < pointer_per_hash; x++)
          {
            uint32_t value= ketama_server_hash(sort_host, (size_t)sort_host_length, x);
            ptr->ketama.continuum[continuum_index].index= host_index;
            ptr->ketama.continuum[continuum_index++].value= value;
          }
        }
        else
        {
          uint32_t value= hashkit_digest(&ptr->hashkit, sort_host, (size_t)sort_host_length);
          ptr->ketama.continuum[continuum_index].index= host_index;
          ptr->ketama.continuum[continuum_index++].value= value;
        }
      }
    }
    else
    {
      for (uint32_t pointer_index= 1;
           pointer_index <= pointer_per_server / pointer_per_hash;
           pointer_index++)
      {
        char sort_host[MEMCACHED_NI_MAXHOST +1 +MEMCACHED_NI_MAXSERV +1 +MEMCACHED_NI_MAXSERV]= "";
        int sort_host_length;

        if (list[host_index].port() == MEMCACHED_DEFAULT_PORT)
        {
          sort_host_length= snprintf(sort_host, sizeof(sort_host),
                                     "%s-%u",
                                     list[host_index]._hostname,
                                     pointer_index - 1);
        }
        else
        {
          sort_host_length= snprintf(sort_host, sizeof(sort_host),
                                     "%s:%u-%u",
                                     list[host_index]._hostname,
                                     (uint32_t)list[host_index].port(),
                                     pointer_index - 1);
        }

        if (size_t(sort_host_length) >= sizeof(sort_host) or sort_host_length < 0)
        {
          return memcached_set_error(*ptr, MEMCACHED_MEMORY_ALLOCATION_FAILURE, MEMCACHED_AT, 
                                     memcached_literal_param("snprintf(sizeof(sort_host)))"));
        }

        if (memcached_is_weighted_ketama(ptr))
        {
          for (uint32_t x = 0; x < pointer_per_hash; x++)
          {
            uint32_t value= ketama_server_hash(sort_host, (size_t)sort_host_length, x);
            ptr->ketama.continuum[continuum_index].index= host_index;
            ptr->ketama.continuum[continuum_index++].value= value;
          }
        }
        else
        {
          uint32_t value= hashkit_digest(&ptr->hashkit, sort_host, (size_t)sort_host_length);
          ptr->ketama.continuum[continuum_index].index= host_index;
          ptr->ketama.continuum[continuum_index++].value= value;
        }
      }
    }

    pointer_counter+= pointer_per_server;
  }

  assert_msg(ptr, "Programmer Error, no valid ptr");
  assert_msg(ptr->ketama.continuum, "Programmer Error, empty ketama continuum");
  assert_msg(memcached_server_count(ptr) * MEMCACHED_POINTS_PER_SERVER <= MEMCACHED_CONTINUUM_SIZE, "invalid size information being given to qsort()");
  ptr->ketama.continuum_points_counter= pointer_counter;
  qsort(ptr->ketama.continuum, ptr->ketama.continuum_points_counter, sizeof(memcached_continuum_item_st), continuum_item_cmp);

  if (DEBUG)
  {
    for (uint32_t pointer_index= 0; memcached_server_count(ptr) && pointer_index < ((live_servers * MEMCACHED_POINTS_PER_SERVER) - 1); pointer_index++)
    {
      WATCHPOINT_ASSERT(ptr->ketama.continuum[pointer_index].value <= ptr->ketama.continuum[pointer_index + 1].value);
    }
  }

  return MEMCACHED_SUCCESS;
}

static memcached_return_t server_add(Memcached *memc, 
                                     const memcached_string_t& hostname,
                                     in_port_t port,
                                     uint32_t weight,
                                     memcached_connection_t type)
{
  assert_msg(memc, "Programmer mistake, somehow server_add() was passed a NULL memcached_st");

  if (memc->number_of_hosts)
  {
    assert(memcached_instance_list(memc));
  }

  if (memcached_instance_list(memc))
  {
    assert(memc->number_of_hosts);
  }

  uint32_t host_list_size= memc->number_of_hosts +1;
  memcached_instance_st* new_host_list= libmemcached_xrealloc(memc, memcached_instance_list(memc), host_list_size, memcached_instance_st);

  if (new_host_list == NULL)
  {
    return memcached_set_error(*memc, MEMCACHED_MEMORY_ALLOCATION_FAILURE, MEMCACHED_AT);
  }

  memcached_instance_set(memc, new_host_list, host_list_size);
  assert(memc->number_of_hosts == host_list_size);

  /* TODO: Check return type */
  memcached_instance_st* instance= memcached_instance_fetch(memc, memcached_server_count(memc) -1);

  if (__instance_create_with(memc, instance, hostname, port, weight, type) == NULL)
  {
    return memcached_set_error(*memc, MEMCACHED_MEMORY_ALLOCATION_FAILURE, MEMCACHED_AT);
  }

  if (weight > 1)
  {
    if (memcached_is_consistent_distribution(memc))
    {
      memcached_set_weighted_ketama(memc, true);
    }
  }

  return run_distribution(memc);
}


memcached_return_t memcached_server_push(memcached_st *shell, const memcached_server_list_st list)
{
  if (list == NULL)
  {
    return MEMCACHED_SUCCESS;
  }

  Memcached* ptr= memcached2Memcached(shell);
  if (ptr)
  {
    uint32_t original_host_size= memcached_server_count(ptr);
    uint32_t count= memcached_server_list_count(list);
    uint32_t host_list_size= count +original_host_size;

    memcached_instance_st* new_host_list= libmemcached_xrealloc(ptr, memcached_instance_list(ptr), host_list_size, memcached_instance_st);

    if (new_host_list == NULL)
    {
      return MEMCACHED_MEMORY_ALLOCATION_FAILURE;
    }

    memcached_instance_set(ptr, new_host_list, host_list_size);

    ptr->state.is_parsing= true;
    for (uint32_t x= 0; x < count; ++x, ++original_host_size)
    {
      WATCHPOINT_ASSERT(list[x].hostname[0] != 0);

      // We have extended the array, and now we will find it, and use it.
      memcached_instance_st* instance= memcached_instance_fetch(ptr, original_host_size);
      WATCHPOINT_ASSERT(instance);

      memcached_string_t hostname= { memcached_string_make_from_cstr(list[x].hostname) };
      if (__instance_create_with(ptr, instance, 
                                 hostname,
                                 list[x].port, list[x].weight, list[x].type) == NULL)
      {
        ptr->state.is_parsing= false;
        return memcached_set_error(*ptr, MEMCACHED_MEMORY_ALLOCATION_FAILURE, MEMCACHED_AT);
      }

      if (list[x].weight > 1)
      {
        memcached_set_weighted_ketama(ptr, true);
      }
    }
    ptr->state.is_parsing= false;

    return run_distribution(ptr);
  }

  return MEMCACHED_INVALID_ARGUMENTS;
}

memcached_return_t memcached_instance_push(memcached_st *ptr, const struct memcached_instance_st* list, uint32_t number_of_hosts)
{
  if (list == NULL)
  {
    return MEMCACHED_SUCCESS;
  }

  uint32_t original_host_size= memcached_server_count(ptr);
  uint32_t host_list_size= number_of_hosts +original_host_size;
  memcached_instance_st* new_host_list= libmemcached_xrealloc(ptr, memcached_instance_list(ptr), host_list_size, memcached_instance_st);

  if (new_host_list == NULL)
  {
    return MEMCACHED_MEMORY_ALLOCATION_FAILURE;
  }

  memcached_instance_set(ptr, new_host_list, host_list_size);

  // We don't bother with lookups for this operation
  ptr->state.is_parsing= true;

  // We use original_host_size since size will now point to the first new
  // instance allocated.
  for (uint32_t x= 0; x < number_of_hosts; ++x, ++original_host_size)
  {
    WATCHPOINT_ASSERT(list[x]._hostname[0] != 0);

    // We have extended the array, and now we will find it, and use it.
    memcached_instance_st* instance= memcached_instance_fetch(ptr, original_host_size);
    WATCHPOINT_ASSERT(instance);

    memcached_string_t hostname= { memcached_string_make_from_cstr(list[x]._hostname) };
    if (__instance_create_with(ptr, instance, 
                               hostname,
                               list[x].port(), list[x].weight, list[x].type) == NULL)
    {
      ptr->state.is_parsing= false;
      return memcached_set_error(*ptr, MEMCACHED_MEMORY_ALLOCATION_FAILURE, MEMCACHED_AT);
    }

    if (list[x].weight > 1)
    {
      memcached_set_weighted_ketama(ptr, true);
    }
  }
  ptr->state.is_parsing= false;

  return run_distribution(ptr);
}

memcached_return_t memcached_server_add_unix_socket(memcached_st *ptr,
                                                    const char *filename)
{
  return memcached_server_add_unix_socket_with_weight(ptr, filename, 0);
}

memcached_return_t memcached_server_add_unix_socket_with_weight(memcached_st *shell,
                                                                const char *filename,
                                                                uint32_t weight)
{
  Memcached* ptr= memcached2Memcached(shell);
  if (ptr)
  {
    memcached_string_t _filename= { memcached_string_make_from_cstr(filename) };
    if (memcached_is_valid_filename(_filename) == false)
    {
      return memcached_set_error(*ptr, MEMCACHED_INVALID_ARGUMENTS, MEMCACHED_AT, memcached_literal_param("Invalid filename for socket provided"));
    }

    return server_add(ptr, _filename, 0, weight, MEMCACHED_CONNECTION_UNIX_SOCKET);
  }

  return MEMCACHED_FAILURE;
}

memcached_return_t memcached_server_add_udp(memcached_st *ptr,
                                            const char *hostname,
                                            in_port_t port)
{
  return memcached_server_add_udp_with_weight(ptr, hostname, port, 0);
}

memcached_return_t memcached_server_add_udp_with_weight(memcached_st *shell,
                                                        const char *,
                                                        in_port_t,
                                                        uint32_t)
{
  Memcached* self= memcached2Memcached(shell);
  if (self)
  {
    return memcached_set_error(*self, MEMCACHED_DEPRECATED, MEMCACHED_AT);
  }

  return MEMCACHED_INVALID_ARGUMENTS;
}

memcached_return_t memcached_server_add(memcached_st *shell,
                                        const char *hostname,
                                        in_port_t port)
{
  return memcached_server_add_with_weight(shell, hostname, port, 0);
}

memcached_return_t memcached_server_add_with_weight(memcached_st *shell,
                                                    const char *hostname,
                                                    in_port_t port,
                                                    uint32_t weight)
{
  Memcached* ptr= memcached2Memcached(shell);
  if (ptr == NULL)
  {
    return MEMCACHED_INVALID_ARGUMENTS;
  }

  if (port == 0)
  {
    port= MEMCACHED_DEFAULT_PORT;
  }

  size_t hostname_length= hostname ? strlen(hostname) : 0;
  if (hostname_length == 0)
  {
    hostname= "localhost";
    hostname_length= memcached_literal_param_size("localhost");
  }

  memcached_string_t _hostname= { hostname, hostname_length };

  if (memcached_is_valid_servername(_hostname) == false)
  {
    return memcached_set_error(*ptr, MEMCACHED_INVALID_ARGUMENTS, MEMCACHED_AT, memcached_literal_param("Invalid hostname provided"));
  }

  return server_add(ptr, _hostname, port, weight, _hostname.c_str[0] == '/' ? MEMCACHED_CONNECTION_UNIX_SOCKET  : MEMCACHED_CONNECTION_TCP);
}

memcached_return_t memcached_server_add_parsed(memcached_st *ptr,
                                               const char *hostname,
                                               size_t hostname_length,
                                               in_port_t port,
                                               uint32_t weight)
{
  char buffer[MEMCACHED_NI_MAXHOST]= { 0 };

  memcpy(buffer, hostname, hostname_length);
  buffer[hostname_length]= 0;

  memcached_string_t _hostname= { buffer, hostname_length };

  return server_add(ptr, _hostname,
                    port,
                    weight,
                    MEMCACHED_CONNECTION_TCP);
}
