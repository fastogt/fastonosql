/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Libmemcached library
 *
 *  Copyright (C) 2011-2013 Data Differential, http://datadifferential.com/
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

static const char *memcached_stat_keys[] = {
  "pid",
  "uptime",
  "time",
  "version",
  "pointer_size",
  "rusage_user",
  "rusage_system",
  "curr_items",
  "total_items",
  "bytes",
  "curr_connections",
  "total_connections",
  "connection_structures",
  "cmd_get",
  "cmd_set",
  "get_hits",
  "get_misses",
  "evictions",
  "bytes_read",
  "bytes_written",
  "limit_maxbytes",
  "threads",
  NULL
};

struct local_context
{
  memcached_stat_fn func;
  void *context;
  const char *args;
  const size_t args_length;

  local_context(memcached_stat_fn func_arg,
                void *context_arg,
                const char *args_arg,
                const size_t args_length_arg) :
    func(func_arg),
    context(context_arg),
    args(args_arg),
    args_length(args_length_arg)
  { }
};


static memcached_return_t set_data(memcached_stat_st *memc_stat, const char *key, const char *value)
{

  if (strlen(key) < 1)
  {
    WATCHPOINT_STRING(key);
    return MEMCACHED_UNKNOWN_STAT_KEY;
  }
  else if (strcmp("pid", key) == 0)
  {
    errno= 0;
    int64_t temp= strtoll(value, (char **)NULL, 10);
    if (errno != 0)
    {
      return MEMCACHED_FAILURE;
    }

    if (temp <= INT32_MAX and ( sizeof(pid_t) == sizeof(int32_t) ))
    {
      memc_stat->pid= pid_t(temp);
    }
    else if (temp > -1)
    {
      memc_stat->pid= pid_t(temp);
    }
    else
    {
      // If we got a value less then -1 then something went wrong in the
      // protocol
    }
  }
  else if (not strcmp("uptime", key))
  {
    errno= 0;
    memc_stat->uptime= strtoul(value, (char **)NULL, 10);
    if (errno != 0)
    {
      return MEMCACHED_FAILURE;
    }
  }
  else if (not strcmp("time", key))
  {
    errno= 0;
    memc_stat->time= strtoul(value, (char **)NULL, 10);
    if (errno != 0)
    {
      return MEMCACHED_FAILURE;
    }
  }
  else if (not strcmp("version", key))
  {
    memcpy(memc_stat->version, value, strlen(value));
    memc_stat->version[strlen(value)]= 0;
  }
  else if (not strcmp("pointer_size", key))
  {
    errno= 0;
    memc_stat->pointer_size= strtoul(value, (char **)NULL, 10);
    if (errno != 0)
    {
      return MEMCACHED_FAILURE;
    }
  }
  else if (not strcmp("rusage_user", key))
  {
    char *walk_ptr;
    for (walk_ptr= (char*)value; (!ispunct(*walk_ptr)); walk_ptr++) {};
    *walk_ptr= 0;
    walk_ptr++;

    errno= 0;
    memc_stat->rusage_user_seconds= strtoul(value, (char **)NULL, 10);
    if (errno != 0)
    {
      return MEMCACHED_FAILURE;
    }

    errno= 0;
    memc_stat->rusage_user_microseconds= strtoul(walk_ptr, (char **)NULL, 10);
    if (errno != 0)
    {
      return MEMCACHED_FAILURE;
    }
  }
  else if (not strcmp("rusage_system", key))
  {
    char *walk_ptr;
    for (walk_ptr= (char*)value; (!ispunct(*walk_ptr)); walk_ptr++) {};
    *walk_ptr= 0;
    walk_ptr++;

    errno= 0;
    memc_stat->rusage_system_seconds= strtoul(value, (char **)NULL, 10);
    if (errno != 0)
    {
      return MEMCACHED_FAILURE;
    }

    errno= 0;
    memc_stat->rusage_system_microseconds= strtoul(walk_ptr, (char **)NULL, 10);
    if (errno != 0)
    {
      return MEMCACHED_FAILURE;
    }
  }
  else if (not strcmp("curr_items", key))
  {
    errno= 0;
    memc_stat->curr_items= strtoul(value, (char **)NULL, 10);
    if (errno != 0)
    {
      return MEMCACHED_FAILURE;
    }
  }
  else if (not strcmp("total_items", key))
  {
    errno= 0;
    memc_stat->total_items= strtoul(value, (char **)NULL, 10);
    if (errno != 0)
    {
      return MEMCACHED_FAILURE;
    }
  }
  else if (not strcmp("bytes_read", key))
  {
    errno= 0;
    memc_stat->bytes_read= strtoull(value, (char **)NULL, 10);
    if (errno != 0)
    {
      return MEMCACHED_FAILURE;
    }
  }
  else if (not strcmp("bytes_written", key))
  {
    errno= 0;
    memc_stat->bytes_written= strtoull(value, (char **)NULL, 10);
    if (errno != 0)
    {
      return MEMCACHED_FAILURE;
    }
  }
  else if (not strcmp("bytes", key))
  {
    errno= 0;
    memc_stat->bytes= strtoull(value, (char **)NULL, 10);
    if (errno != 0)
    {
      return MEMCACHED_FAILURE;
    }
  }
  else if (not strcmp("curr_connections", key))
  {
    errno= 0;
    memc_stat->curr_connections= strtoull(value, (char **)NULL, 10);
    if (errno != 0)
    {
      return MEMCACHED_FAILURE;
    }
  }
  else if (not strcmp("total_connections", key))
  {
    errno= 0;
    memc_stat->total_connections= strtoull(value, (char **)NULL, 10);
    if (errno != 0)
    {
      return MEMCACHED_FAILURE;
    }
  }
  else if (not strcmp("connection_structures", key))
  {
    errno= 0;
    memc_stat->connection_structures= strtoul(value, (char **)NULL, 10);
    if (errno != 0)
    {
      return MEMCACHED_FAILURE;
    }
  }
  else if (not strcmp("cmd_get", key))
  {
    errno= 0;
    memc_stat->cmd_get= strtoull(value, (char **)NULL, 10);
    if (errno != 0)
    {
      return MEMCACHED_FAILURE;
    }
  }
  else if (not strcmp("cmd_set", key))
  {
    errno= 0;
    memc_stat->cmd_set= strtoull(value, (char **)NULL, 10);
    if (errno != 0)
    {
      return MEMCACHED_FAILURE;
    }
  }
  else if (not strcmp("get_hits", key))
  {
    errno= 0;
    memc_stat->get_hits= strtoull(value, (char **)NULL, 10);
    if (errno != 0)
    {
      return MEMCACHED_FAILURE;
    }
  }
  else if (not strcmp("get_misses", key))
  {
    errno= 0;
    memc_stat->get_misses= strtoull(value, (char **)NULL, 10);
    if (errno != 0)
    {
      return MEMCACHED_FAILURE;
    }
  }
  else if (not strcmp("evictions", key))
  {
    errno= 0;
    memc_stat->evictions= strtoull(value, (char **)NULL, 10);
    if (errno != 0)
    {
      return MEMCACHED_FAILURE;
    }
  }
  else if (not strcmp("limit_maxbytes", key))
  {
    errno= 0;
    memc_stat->limit_maxbytes= strtoull(value, (char **)NULL, 10);
    if (errno != 0)
    {
      return MEMCACHED_FAILURE;
    }
  }
  else if (not strcmp("threads", key))
  {
    errno= 0;
    memc_stat->threads= strtoul(value, (char **)NULL, 10);
    if (errno != 0)
    {
      return MEMCACHED_FAILURE;
    }
  }
  else if ((strcmp("delete_misses", key) == 0 or /* New stats in the 1.3 beta */
            strcmp("delete_hits", key) == 0 or /* Just swallow them for now.. */
            strcmp("incr_misses", key) == 0 or
            strcmp("incr_hits", key) == 0 or
            strcmp("decr_misses", key) == 0 or
            strcmp("decr_hits", key) == 0 or
            strcmp("cas_misses", key) == 0 or
            strcmp("cas_hits", key) == 0 or
            strcmp("cas_badval", key) == 0 or
            strcmp("cmd_flush", key) == 0 or
            strcmp("accepting_conns", key) == 0 or
            strcmp("listen_disabled_num", key) == 0 or
            strcmp("conn_yields", key) == 0 or
            strcmp("auth_cmds", key) == 0 or
            strcmp("auth_errors", key) == 0 or
            strcmp("reclaimed", key) == 0) == 0)
  {
    WATCHPOINT_STRING(key);
    /* return MEMCACHED_UNKNOWN_STAT_KEY; */
    return MEMCACHED_SUCCESS;
  }

  return MEMCACHED_SUCCESS;
}

char *memcached_stat_get_value(const memcached_st* shell, memcached_stat_st *memc_stat,
                               const char *key, memcached_return_t *error)
{
  memcached_return_t not_used;
  if (error == NULL)
  {
    error= &not_used;
  }

  if (memc_stat == NULL)
  {
    *error= MEMCACHED_INVALID_ARGUMENTS;
    return NULL;
  }

  char buffer[SMALL_STRING_LEN];
  int length;

  *error= MEMCACHED_SUCCESS;

  if (memcmp("pid", key, sizeof("pid") -1) == 0)
  {
    length= snprintf(buffer, SMALL_STRING_LEN,"%lld", (signed long long)memc_stat->pid);
  }
  else if (not memcmp("uptime", key, sizeof("uptime") -1))
  {
    length= snprintf(buffer, SMALL_STRING_LEN,"%lu", memc_stat->uptime);
  }
  else if (not memcmp("time", key, sizeof("time") -1))
  {
    length= snprintf(buffer, SMALL_STRING_LEN,"%llu", (unsigned long long)memc_stat->time);
  }
  else if (not memcmp("version", key, sizeof("version") -1))
  {
    length= snprintf(buffer, SMALL_STRING_LEN,"%s", memc_stat->version);
  }
  else if (not memcmp("pointer_size", key, sizeof("pointer_size") -1))
  {
    length= snprintf(buffer, SMALL_STRING_LEN,"%lu", memc_stat->pointer_size);
  }
  else if (not memcmp("rusage_user", key, sizeof("rusage_user") -1))
  {
    length= snprintf(buffer, SMALL_STRING_LEN,"%lu.%lu", memc_stat->rusage_user_seconds, memc_stat->rusage_user_microseconds);
  }
  else if (not memcmp("rusage_system", key, sizeof("rusage_system") -1))
  {
    length= snprintf(buffer, SMALL_STRING_LEN,"%lu.%lu", memc_stat->rusage_system_seconds, memc_stat->rusage_system_microseconds);
  }
  else if (not memcmp("curr_items", key, sizeof("curr_items") -1))
  {
    length= snprintf(buffer, SMALL_STRING_LEN,"%lu", memc_stat->curr_items);
  }
  else if (not memcmp("total_items", key, sizeof("total_items") -1))
  {
    length= snprintf(buffer, SMALL_STRING_LEN,"%lu", memc_stat->total_items);
  }
  else if (not memcmp("curr_connections", key, sizeof("curr_connections") -1))
  {
    length= snprintf(buffer, SMALL_STRING_LEN,"%lu", memc_stat->curr_connections);
  }
  else if (not memcmp("total_connections", key, sizeof("total_connections") -1))
  {
    length= snprintf(buffer, SMALL_STRING_LEN,"%lu", memc_stat->total_connections);
  }
  else if (not memcmp("connection_structures", key, sizeof("connection_structures") -1))
  {
    length= snprintf(buffer, SMALL_STRING_LEN,"%lu", memc_stat->connection_structures);
  }
  else if (not memcmp("cmd_get", key, sizeof("cmd_get") -1))
  {
    length= snprintf(buffer, SMALL_STRING_LEN,"%llu", (unsigned long long)memc_stat->cmd_get);
  }
  else if (not memcmp("cmd_set", key, sizeof("cmd_set") -1))
  {
    length= snprintf(buffer, SMALL_STRING_LEN,"%llu", (unsigned long long)memc_stat->cmd_set);
  }
  else if (not memcmp("get_hits", key, sizeof("get_hits") -1))
  {
    length= snprintf(buffer, SMALL_STRING_LEN,"%llu", (unsigned long long)memc_stat->get_hits);
  }
  else if (not memcmp("get_misses", key, sizeof("get_misses") -1))
  {
    length= snprintf(buffer, SMALL_STRING_LEN,"%llu", (unsigned long long)memc_stat->get_misses);
  }
  else if (not memcmp("evictions", key, sizeof("evictions") -1))
  {
    length= snprintf(buffer, SMALL_STRING_LEN,"%llu", (unsigned long long)memc_stat->evictions);
  }
  else if (not memcmp("bytes_read", key, sizeof("bytes_read") -1))
  {
    length= snprintf(buffer, SMALL_STRING_LEN,"%llu", (unsigned long long)memc_stat->bytes_read);
  }
  else if (not memcmp("bytes_written", key, sizeof("bytes_written") -1))
  {
    length= snprintf(buffer, SMALL_STRING_LEN,"%llu", (unsigned long long)memc_stat->bytes_written);
  }
  else if (not memcmp("bytes", key, sizeof("bytes") -1))
  {
    length= snprintf(buffer, SMALL_STRING_LEN,"%llu", (unsigned long long)memc_stat->bytes);
  }
  else if (not memcmp("limit_maxbytes", key, sizeof("limit_maxbytes") -1))
  {
    length= snprintf(buffer, SMALL_STRING_LEN,"%llu", (unsigned long long)memc_stat->limit_maxbytes);
  }
  else if (not memcmp("threads", key, sizeof("threads") -1))
  {
    length= snprintf(buffer, SMALL_STRING_LEN,"%lu", memc_stat->threads);
  }
  else
  {
    Memcached* memc= (Memcached*)memcached2Memcached(shell);
    *error= memcached_set_error(*memc, MEMCACHED_INVALID_ARGUMENTS, MEMCACHED_AT, memcached_literal_param("Invalid key provided"));
    return NULL;
  }

  if (length >= SMALL_STRING_LEN || length < 0)
  {
    Memcached* memc= (Memcached*)memcached2Memcached(shell);
    *error= memcached_set_error(*memc, MEMCACHED_FAILURE, MEMCACHED_AT, memcached_literal_param("Internal failure occured with buffer, please report this bug."));
    return NULL;
  }

  // User is responsible for free() memory, so use malloc()
  char *ret= static_cast<char *>(malloc(size_t(length +1)));
  memcpy(ret, buffer, (size_t) length);
  ret[length]= '\0';

  return ret;
}

static memcached_return_t binary_stats_fetch(memcached_stat_st *memc_stat,
                                             const char *args,
                                             const size_t args_length,
                                             memcached_instance_st* instance,
                                             struct local_context *check)
{
  char buffer[MEMCACHED_DEFAULT_COMMAND_SIZE];
  protocol_binary_request_stats request= {}; // = {.bytes= {0}};

  initialize_binary_request(instance, request.message.header);

  request.message.header.request.opcode= PROTOCOL_BINARY_CMD_STAT;
  request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;

  if (args_length)
  {
    request.message.header.request.keylen= htons(uint16_t(args_length));
    request.message.header.request.bodylen= htonl(uint32_t( args_length));

    libmemcached_io_vector_st vector[]=
    {
      { request.bytes, sizeof(request.bytes) },
      { args, args_length }
    };

    if (memcached_vdo(instance, vector, 2, true) != MEMCACHED_SUCCESS)
    {
      memcached_io_reset(instance);
      return MEMCACHED_WRITE_FAILURE;
    }
  }
  else
  {
    libmemcached_io_vector_st vector[]=
    {
      { request.bytes, sizeof(request.bytes) }
    };

    if (memcached_vdo(instance, vector, 1, true) != MEMCACHED_SUCCESS)
    {
      memcached_io_reset(instance);
      return MEMCACHED_WRITE_FAILURE;
    }
  }

  memcached_server_response_decrement(instance);
  while (1)
  {
    memcached_return_t rc= memcached_response(instance, buffer, sizeof(buffer), NULL);

    if (rc == MEMCACHED_END)
    {
      break;
    }

    if (rc != MEMCACHED_SUCCESS)
    {
      memcached_io_reset(instance);
      return rc;
    }

    if (check && check->func)
    {
      size_t key_length= strlen(buffer);

      check->func(instance,
                  buffer, key_length,
                  buffer+key_length+1, strlen(buffer+key_length+1),
                  check->context);
    }

    if (memc_stat)
    {
      if ((set_data(memc_stat, buffer, buffer + strlen(buffer) + 1)) == MEMCACHED_UNKNOWN_STAT_KEY)
      {
        WATCHPOINT_ERROR(MEMCACHED_UNKNOWN_STAT_KEY);
        WATCHPOINT_ASSERT(0);
      }
    }
  }

  /* 
   * memcached_response will decrement the counter, so I need to reset it..
   * todo: look at this and try to find a better solution.  
   * */
  instance->cursor_active_= 0;

  return MEMCACHED_SUCCESS;
}

static memcached_return_t ascii_stats_fetch(memcached_stat_st *memc_stat,
                                            const char *args,
                                            const size_t args_length,
                                            memcached_instance_st* instance,
                                            struct local_context *check)
{
  libmemcached_io_vector_st vector[]=
  {
    { memcached_literal_param("stats ") },
    { args, args_length },
    { memcached_literal_param("\r\n") }
  };

  memcached_return_t rc= memcached_vdo(instance, vector, 3, true);
  if (memcached_success(rc))
  {
    char buffer[MEMCACHED_DEFAULT_COMMAND_SIZE];
    while ((rc= memcached_response(instance, buffer, sizeof(buffer), NULL)) == MEMCACHED_STAT)
    {
      char *string_ptr= buffer;
      string_ptr+= 5; /* Move past STAT */

      char *end_ptr;
      for (end_ptr= string_ptr; isgraph(*end_ptr); end_ptr++) {};
      char *key= string_ptr;
      key[size_t(end_ptr-string_ptr)]= 0;

      string_ptr= end_ptr + 1;
      for (end_ptr= string_ptr; !(isspace(*end_ptr)); end_ptr++) {};
      char *value= string_ptr;
      value[(size_t)(end_ptr -string_ptr)]= 0;
#if 0
      bool check_bool= bool(check);
      bool check_func_bool= bool(check) ? bool(check->func) : false;
      fprintf(stderr, "%s:%d %s %s %d:%d\n", __FILE__, __LINE__, key, value, check_bool, check_func_bool);
#endif

      if (check and check->func)
      {
        check->func(instance,
                    key, strlen(key),
                    value, strlen(value),
                    check->context);
      }

      if (memc_stat)
      {
        if((set_data(memc_stat, key, value)) == MEMCACHED_UNKNOWN_STAT_KEY)
        {
          WATCHPOINT_ERROR(MEMCACHED_UNKNOWN_STAT_KEY);
          WATCHPOINT_ASSERT(0);
        }
      }
    }
  }

  if (rc == MEMCACHED_ERROR)
  {
    return MEMCACHED_INVALID_ARGUMENTS;
  }

  if (rc == MEMCACHED_END)
  {
    return MEMCACHED_SUCCESS;
  }

  return rc;
}

memcached_stat_st *memcached_stat(memcached_st *shell, char *args, memcached_return_t *error)
{
  Memcached* self= memcached2Memcached(shell);
  memcached_return_t unused;
  if (error == NULL)
  {
    error= &unused;
  }

  if (memcached_failed(*error= initialize_query(self, true)))
  {
    return NULL;
  }

  if (memcached_is_udp(self))
  {
    *error= memcached_set_error(*self, MEMCACHED_NOT_SUPPORTED, MEMCACHED_AT);
    return NULL;
  }

  memcached_return_t rc;
  size_t args_length= 0;
  if (args)
  {
    args_length= strlen(args);
    if (memcached_failed(rc= memcached_key_test(*self, (const char **)&args, &args_length, 1)))
    {
      *error= memcached_set_error(*self, rc, MEMCACHED_AT);
      return NULL;
    }
  }

  WATCHPOINT_ASSERT(error);

  memcached_stat_st *stats= libmemcached_xcalloc(self, memcached_server_count(self), memcached_stat_st);
  if (stats == NULL)
  {
    *error= memcached_set_error(*self, MEMCACHED_MEMORY_ALLOCATION_FAILURE, MEMCACHED_AT);
    return NULL;
  }

  WATCHPOINT_ASSERT(rc == MEMCACHED_SUCCESS);
  rc= MEMCACHED_SUCCESS;
  for (uint32_t x= 0; x < memcached_server_count(self); x++)
  {
    memcached_stat_st* stat_instance= stats +x;

    stat_instance->pid= -1;
    stat_instance->root= self;

    memcached_instance_st* instance= memcached_instance_fetch(self, x);

    memcached_return_t temp_return;
    if (memcached_is_binary(self))
    {
      temp_return= binary_stats_fetch(stat_instance, args, args_length, instance, NULL);
    }
    else
    {
      temp_return= ascii_stats_fetch(stat_instance, args, args_length, instance, NULL);
    }

    // Special case where "args" is invalid
    if (temp_return == MEMCACHED_INVALID_ARGUMENTS)
    {
      rc= MEMCACHED_INVALID_ARGUMENTS;
      break;
    }

    if (memcached_failed(temp_return))
    {
      rc= MEMCACHED_SOME_ERRORS;
    }
  }

  *error= rc;

  return stats;
}

memcached_return_t memcached_stat_servername(memcached_stat_st *memc_stat, char *args,
                                             const char *hostname, in_port_t port)
{
  memcached_st memc;

  memcached_stat_st unused_memc_stat;
  if (memc_stat == NULL)
  {
    memc_stat= &unused_memc_stat;
  }

  memset(memc_stat, 0, sizeof(memcached_stat_st));

  memcached_st *memc_ptr= memcached_create(&memc);
  if (memc_ptr == NULL)
  {
    return MEMCACHED_MEMORY_ALLOCATION_FAILURE;
  }

  memcached_return_t rc;
  if (memcached_failed(rc= memcached_server_add(&memc, hostname, port)))
  {
    memcached_free(&memc);
    return rc;
  }

  if (memcached_success(rc= initialize_query(memc_ptr, true)))
  {
    size_t args_length= 0;
    if (args)
    {
      args_length= strlen(args);
      rc= memcached_key_test(*memc_ptr, (const char **)&args, &args_length, 1);
    }

    if (memcached_success(rc))
    {
      memcached_instance_st* instance= memcached_instance_fetch(memc_ptr, 0);
      if (memc.flags.binary_protocol)
      {
        rc= binary_stats_fetch(memc_stat, args, args_length, instance, NULL);
      }
      else
      {
        rc= ascii_stats_fetch(memc_stat, args, args_length, instance, NULL);
      }
    }
  }

  memcached_free(&memc);

  return rc;
}

/*
  We make a copy of the keys since at some point in the not so distant future
  we will add support for "found" keys.
*/
char ** memcached_stat_get_keys(memcached_st *shell,
                                memcached_stat_st *,
                                memcached_return_t *error)
{
  Memcached* memc= memcached2Memcached(shell);
  if (memc)
  {
    char **list= static_cast<char **>(libmemcached_malloc(memc, sizeof(memcached_stat_keys)));
    if (list == NULL)
    {
      if (error)
      {
        *error= memcached_set_error(*memc, MEMCACHED_MEMORY_ALLOCATION_FAILURE, MEMCACHED_AT);
      }

      return NULL;
    }

    memcpy(list, memcached_stat_keys, sizeof(memcached_stat_keys));

    if (error)
    {
      *error= MEMCACHED_SUCCESS;
    }

    return list;
  }

  return NULL;
}

void memcached_stat_free(const memcached_st *, memcached_stat_st *memc_stat)
{
  WATCHPOINT_ASSERT(memc_stat); // Be polite, but when debugging catch this as an error
  if (memc_stat)
  {
    libmemcached_free(memc_stat->root, memc_stat);
  }
}

static memcached_return_t call_stat_fn(memcached_st *memc,
                                       memcached_instance_st* instance,
                                       void *context)
{
  if (memc)
  {
    local_context *check= (struct local_context *)context;

    if (memcached_is_binary(memc))
    {
      return binary_stats_fetch(NULL, check->args, check->args_length, instance, check);
    }
    else
    {
      return ascii_stats_fetch(NULL, check->args, check->args_length, instance, check);
    }
  }

  return MEMCACHED_INVALID_ARGUMENTS;
}

memcached_return_t memcached_stat_execute(memcached_st *shell, const char *args,  memcached_stat_fn func, void *context)
{
  Memcached* memc= memcached2Memcached(shell);
  if (memcached_fatal(memcached_version(memc)))
  {
    return memcached_last_error(memc);
  }

 local_context check(func, context, args, args ? strlen(args) : 0);

 return memcached_server_execute(memc, call_stat_fn, (void *)&check);
}
