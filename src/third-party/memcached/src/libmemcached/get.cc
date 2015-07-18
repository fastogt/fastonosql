/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Libmemcached library
 *
 *  Copyright (C) 2011-2013 Data Differential, http://datadifferential.com/
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

/*
  What happens if no servers exist?
*/
char *memcached_get(memcached_st *ptr, const char *key,
                    size_t key_length,
                    size_t *value_length,
                    uint32_t *flags,
                    memcached_return_t *error)
{
  return memcached_get_by_key(ptr, NULL, 0, key, key_length, value_length,
                              flags, error);
}

static memcached_return_t __mget_by_key_real(memcached_st *ptr,
                                             const char *group_key,
                                             size_t group_key_length,
                                             const char * const *keys,
                                             const size_t *key_length,
                                             size_t number_of_keys,
                                             const bool mget_mode);
char *memcached_get_by_key(memcached_st *shell,
                           const char *group_key,
                           size_t group_key_length,
                           const char *key, size_t key_length,
                           size_t *value_length,
                           uint32_t *flags,
                           memcached_return_t *error)
{
  Memcached* ptr= memcached2Memcached(shell);
  memcached_return_t unused;
  if (error == NULL)
  {
    error= &unused;
  }

  uint64_t query_id= 0;
  if (ptr)
  {
    query_id= ptr->query_id;
  }

  /* Request the key */
  *error= __mget_by_key_real(ptr, group_key, group_key_length,
                             (const char * const *)&key, &key_length, 
                             1, false);
  if (ptr)
  {
    assert_msg(ptr->query_id == query_id +1, "Programmer error, the query_id was not incremented.");
  }

  if (memcached_failed(*error))
  {
    if (ptr)
    {
      if (memcached_has_current_error(*ptr)) // Find the most accurate error
      {
        *error= memcached_last_error(ptr);
      }
    }

    if (value_length) 
    {
      *value_length= 0;
    }

    return NULL;
  }

  char *value= memcached_fetch(ptr, NULL, NULL,
                               value_length, flags, error);
  assert_msg(ptr->query_id == query_id +1, "Programmer error, the query_id was not incremented.");

  /* This is for historical reasons */
  if (*error == MEMCACHED_END)
  {
    *error= MEMCACHED_NOTFOUND;
  }
  if (value == NULL)
  {
    if (ptr->get_key_failure and *error == MEMCACHED_NOTFOUND)
    {
      memcached_result_st key_failure_result;
      memcached_result_st* result_ptr= memcached_result_create(ptr, &key_failure_result);
      memcached_return_t rc= ptr->get_key_failure(ptr, key, key_length, result_ptr);

      /* On all failure drop to returning NULL */
      if (rc == MEMCACHED_SUCCESS or rc == MEMCACHED_BUFFERED)
      {
        if (rc == MEMCACHED_BUFFERED)
        {
          uint64_t latch; /* We use latch to track the state of the original socket */
          latch= memcached_behavior_get(ptr, MEMCACHED_BEHAVIOR_BUFFER_REQUESTS);
          if (latch == 0)
          {
            memcached_behavior_set(ptr, MEMCACHED_BEHAVIOR_BUFFER_REQUESTS, 1);
          }

          rc= memcached_set(ptr, key, key_length,
                            (memcached_result_value(result_ptr)),
                            (memcached_result_length(result_ptr)),
                            0,
                            (memcached_result_flags(result_ptr)));

          if (rc == MEMCACHED_BUFFERED and latch == 0)
          {
            memcached_behavior_set(ptr, MEMCACHED_BEHAVIOR_BUFFER_REQUESTS, 0);
          }
        }
        else
        {
          rc= memcached_set(ptr, key, key_length,
                            (memcached_result_value(result_ptr)),
                            (memcached_result_length(result_ptr)),
                            0,
                            (memcached_result_flags(result_ptr)));
        }

        if (rc == MEMCACHED_SUCCESS or rc == MEMCACHED_BUFFERED)
        {
          *error= rc;
          *value_length= memcached_result_length(result_ptr);
          *flags= memcached_result_flags(result_ptr);
          char *result_value=  memcached_string_take_value(&result_ptr->value);
          memcached_result_free(result_ptr);

          return result_value;
        }
      }

      memcached_result_free(result_ptr);
    }
    assert_msg(ptr->query_id == query_id +1, "Programmer error, the query_id was not incremented.");

    return NULL;
  }

  return value;
}

memcached_return_t memcached_mget(memcached_st *ptr,
                                  const char * const *keys,
                                  const size_t *key_length,
                                  size_t number_of_keys)
{
  return memcached_mget_by_key(ptr, NULL, 0, keys, key_length, number_of_keys);
}

static memcached_return_t binary_mget_by_key(memcached_st *ptr,
                                             const uint32_t master_server_key,
                                             const bool is_group_key_set,
                                             const char * const *keys,
                                             const size_t *key_length,
                                             const size_t number_of_keys,
                                             const bool mget_mode);

static memcached_return_t __mget_by_key_real(memcached_st *ptr,
                                             const char *group_key,
                                             const size_t group_key_length,
                                             const char * const *keys,
                                             const size_t *key_length,
                                             size_t number_of_keys,
                                             const bool mget_mode)
{
  bool failures_occured_in_sending= false;
  const char *get_command= "get";
  uint8_t get_command_length= 3;
  unsigned int master_server_key= (unsigned int)-1; /* 0 is a valid server id! */

  memcached_return_t rc;
  if (memcached_failed(rc= initialize_query(ptr, true)))
  {
    return rc;
  }

  if (memcached_is_udp(ptr))
  {
    return memcached_set_error(*ptr, MEMCACHED_NOT_SUPPORTED, MEMCACHED_AT);
  }

  LIBMEMCACHED_MEMCACHED_MGET_START();

  if (number_of_keys == 0)
  {
    return memcached_set_error(*ptr, MEMCACHED_INVALID_ARGUMENTS, MEMCACHED_AT, memcached_literal_param("Numbers of keys provided was zero"));
  }

  if (memcached_failed((rc= memcached_key_test(*ptr, keys, key_length, number_of_keys))))
  {
    assert(memcached_last_error(ptr) == rc);

    return rc;
  }

  bool is_group_key_set= false;
  if (group_key and group_key_length)
  {
    master_server_key= memcached_generate_hash_with_redistribution(ptr, group_key, group_key_length);
    is_group_key_set= true;
  }

  /*
    Here is where we pay for the non-block API. We need to remove any data sitting
    in the queue before we start our get.

    It might be optimum to bounce the connection if count > some number.
  */
  for (uint32_t x= 0; x < memcached_server_count(ptr); x++)
  {
    memcached_instance_st* instance= memcached_instance_fetch(ptr, x);

    if (instance->response_count())
    {
      char buffer[MEMCACHED_DEFAULT_COMMAND_SIZE];

      if (ptr->flags.no_block)
      {
        memcached_io_write(instance);
      }

      while(instance->response_count())
      {
        (void)memcached_response(instance, buffer, MEMCACHED_DEFAULT_COMMAND_SIZE, &ptr->result);
      }
    }
  }

  if (memcached_is_binary(ptr))
  {
    return binary_mget_by_key(ptr, master_server_key, is_group_key_set, keys,
                              key_length, number_of_keys, mget_mode);
  }

  if (ptr->flags.support_cas)
  {
    get_command= "gets";
    get_command_length= 4;
  }

  /*
    If a server fails we warn about errors and start all over with sending keys
    to the server.
  */
  WATCHPOINT_ASSERT(rc == MEMCACHED_SUCCESS);
  size_t hosts_connected= 0;
  for (uint32_t x= 0; x < number_of_keys; x++)
  {
    uint32_t server_key;

    if (is_group_key_set)
    {
      server_key= master_server_key;
    }
    else
    {
      server_key= memcached_generate_hash_with_redistribution(ptr, keys[x], key_length[x]);
    }

    memcached_instance_st* instance= memcached_instance_fetch(ptr, server_key);

    libmemcached_io_vector_st vector[]=
    {
      { get_command, get_command_length },
      { memcached_literal_param(" ") },
      { memcached_array_string(ptr->_namespace), memcached_array_size(ptr->_namespace) },
      { keys[x], key_length[x] }
    };


    if (instance->response_count() == 0)
    {
      rc= memcached_connect(instance);

      if (memcached_failed(rc))
      {
        memcached_set_error(*instance, rc, MEMCACHED_AT);
        continue;
      }
      hosts_connected++;

      if ((memcached_io_writev(instance, vector, 1, false)) == false)
      {
        failures_occured_in_sending= true;
        continue;
      }
      WATCHPOINT_ASSERT(instance->cursor_active_ == 0);
      memcached_instance_response_increment(instance);
      WATCHPOINT_ASSERT(instance->cursor_active_ == 1);
    }

    {
      if ((memcached_io_writev(instance, (vector + 1), 3, false)) == false)
      {
        memcached_instance_response_reset(instance);
        failures_occured_in_sending= true;
        continue;
      }
    }
  }

  if (hosts_connected == 0)
  {
    LIBMEMCACHED_MEMCACHED_MGET_END();

    if (memcached_failed(rc))
    {
      return rc;
    }

    return memcached_set_error(*ptr, MEMCACHED_NO_SERVERS, MEMCACHED_AT);
  }


  /*
    Should we muddle on if some servers are dead?
  */
  bool success_happened= false;
  for (uint32_t x= 0; x < memcached_server_count(ptr); x++)
  {
    memcached_instance_st* instance= memcached_instance_fetch(ptr, x);

    if (instance->response_count())
    {
      /* We need to do something about non-connnected hosts in the future */
      if ((memcached_io_write(instance, "\r\n", 2, true)) == -1)
      {
        failures_occured_in_sending= true;
      }
      else
      {
        success_happened= true;
      }
    }
  }

  LIBMEMCACHED_MEMCACHED_MGET_END();

  if (failures_occured_in_sending and success_happened)
  {
    return MEMCACHED_SOME_ERRORS;
  }

  if (success_happened)
  {
    return MEMCACHED_SUCCESS;
  }

  return MEMCACHED_FAILURE; // Complete failure occurred
}

memcached_return_t memcached_mget_by_key(memcached_st *shell,
                                         const char *group_key,
                                         size_t group_key_length,
                                         const char * const *keys,
                                         const size_t *key_length,
                                         size_t number_of_keys)
{
  Memcached* ptr= memcached2Memcached(shell);
  return __mget_by_key_real(ptr, group_key, group_key_length, keys, key_length, number_of_keys, true);
}

memcached_return_t memcached_mget_execute(memcached_st *ptr,
                                          const char * const *keys,
                                          const size_t *key_length,
                                          size_t number_of_keys,
                                          memcached_execute_fn *callback,
                                          void *context,
                                          unsigned int number_of_callbacks)
{
  return memcached_mget_execute_by_key(ptr, NULL, 0, keys, key_length,
                                       number_of_keys, callback,
                                       context, number_of_callbacks);
}

memcached_return_t memcached_mget_execute_by_key(memcached_st *shell,
                                                 const char *group_key,
                                                 size_t group_key_length,
                                                 const char * const *keys,
                                                 const size_t *key_length,
                                                 size_t number_of_keys,
                                                 memcached_execute_fn *callback,
                                                 void *context,
                                                 unsigned int number_of_callbacks)
{
  Memcached* ptr= memcached2Memcached(shell);
  memcached_return_t rc;
  if (memcached_failed(rc= initialize_query(ptr, false)))
  {
    return rc;
  }

  if (memcached_is_udp(ptr))
  {
    return memcached_set_error(*ptr, MEMCACHED_NOT_SUPPORTED, MEMCACHED_AT);
  }

  if (memcached_is_binary(ptr) == false)
  {
    return memcached_set_error(*ptr, MEMCACHED_NOT_SUPPORTED, MEMCACHED_AT,
                               memcached_literal_param("ASCII protocol is not supported for memcached_mget_execute_by_key()"));
  }

  memcached_callback_st *original_callbacks= ptr->callbacks;
  memcached_callback_st cb= {
    callback,
    context,
    number_of_callbacks
  };

  ptr->callbacks= &cb;
  rc= memcached_mget_by_key(ptr, group_key, group_key_length, keys,
                            key_length, number_of_keys);
  ptr->callbacks= original_callbacks;

  return rc;
}

static memcached_return_t simple_binary_mget(memcached_st *ptr,
                                             const uint32_t master_server_key,
                                             bool is_group_key_set,
                                             const char * const *keys,
                                             const size_t *key_length,
                                             const size_t number_of_keys, const bool mget_mode)
{
  memcached_return_t rc= MEMCACHED_NOTFOUND;

  bool flush= (number_of_keys == 1);

  if (memcached_failed(rc= memcached_key_test(*ptr, keys, key_length, number_of_keys)))
  {
    return rc;
  }

  /*
    If a server fails we warn about errors and start all over with sending keys
    to the server.
  */
  for (uint32_t x= 0; x < number_of_keys; ++x)
  {
    uint32_t server_key;

    if (is_group_key_set)
    {
      server_key= master_server_key;
    }
    else
    {
      server_key= memcached_generate_hash_with_redistribution(ptr, keys[x], key_length[x]);
    }

    memcached_instance_st* instance= memcached_instance_fetch(ptr, server_key);

    if (instance->response_count() == 0)
    {
      rc= memcached_connect(instance);
      if (memcached_failed(rc))
      {
        continue;
      }
    }

    protocol_binary_request_getk request= { }; //= {.bytes= {0}};
    initialize_binary_request(instance, request.message.header);
    if (mget_mode)
    {
      request.message.header.request.opcode= PROTOCOL_BINARY_CMD_GETKQ;
    }
    else
    {
      request.message.header.request.opcode= PROTOCOL_BINARY_CMD_GETK;
    }

#if 0
    {
      memcached_return_t vk= memcached_validate_key_length(key_length[x], ptr->flags.binary_protocol);
      if (memcached_failed(rc= memcached_key_test(*memc, (const char **)&key, &key_length, 1)))
      {
        memcached_set_error(ptr, vk, MEMCACHED_AT, memcached_literal_param("Key was too long."));

        if (x > 0)
        {
          memcached_io_reset(instance);
        }

        return vk;
      }
    }
#endif

    request.message.header.request.keylen= htons((uint16_t)(key_length[x] + memcached_array_size(ptr->_namespace)));
    request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;
    request.message.header.request.bodylen= htonl((uint32_t)( key_length[x] + memcached_array_size(ptr->_namespace)));

    libmemcached_io_vector_st vector[]=
    {
      { request.bytes, sizeof(request.bytes) },
      { memcached_array_string(ptr->_namespace), memcached_array_size(ptr->_namespace) },
      { keys[x], key_length[x] }
    };

    if (memcached_io_writev(instance, vector, 3, flush) == false)
    {
      memcached_server_response_reset(instance);
      rc= MEMCACHED_SOME_ERRORS;
      continue;
    }

    /* We just want one pending response per server */
    memcached_server_response_reset(instance);
    memcached_server_response_increment(instance);
    if ((x > 0 and x == ptr->io_key_prefetch) and memcached_flush_buffers(ptr) != MEMCACHED_SUCCESS)
    {
      rc= MEMCACHED_SOME_ERRORS;
    }
  }

  if (mget_mode)
  {
    /*
      Send a noop command to flush the buffers
    */
    protocol_binary_request_noop request= {}; //= {.bytes= {0}};
    request.message.header.request.opcode= PROTOCOL_BINARY_CMD_NOOP;
    request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;

    for (uint32_t x= 0; x < memcached_server_count(ptr); ++x)
    {
      memcached_instance_st* instance= memcached_instance_fetch(ptr, x);

      if (instance->response_count())
      {
        initialize_binary_request(instance, request.message.header);
        if ((memcached_io_write(instance) == false) or
            (memcached_io_write(instance, request.bytes, sizeof(request.bytes), true) == -1))
        {
          memcached_instance_response_reset(instance);
          memcached_io_reset(instance);
          rc= MEMCACHED_SOME_ERRORS;
        }
      }
    }
  }

  return rc;
}

static memcached_return_t replication_binary_mget(memcached_st *ptr,
                                                  uint32_t* hash,
                                                  bool* dead_servers,
                                                  const char *const *keys,
                                                  const size_t *key_length,
                                                  const size_t number_of_keys)
{
  memcached_return_t rc= MEMCACHED_NOTFOUND;
  uint32_t start= 0;
  uint64_t randomize_read= memcached_behavior_get(ptr, MEMCACHED_BEHAVIOR_RANDOMIZE_REPLICA_READ);

  if (randomize_read)
  {
    start= (uint32_t)random() % (uint32_t)(ptr->number_of_replicas + 1);
  }

  /* Loop for each replica */
  for (uint32_t replica= 0; replica <= ptr->number_of_replicas; ++replica)
  {
    bool success= true;

    for (uint32_t x= 0; x < number_of_keys; ++x)
    {
      if (hash[x] == memcached_server_count(ptr))
      {
        continue; /* Already successfully sent */
      }

      uint32_t server= hash[x] +replica;

      /* In case of randomized reads */
      if (randomize_read and ((server + start) <= (hash[x] + ptr->number_of_replicas)))
      {
        server+= start;
      }

      while (server >= memcached_server_count(ptr))
      {
        server -= memcached_server_count(ptr);
      }

      if (dead_servers[server])
      {
        continue;
      }

      memcached_instance_st* instance= memcached_instance_fetch(ptr, server);

      if (instance->response_count() == 0)
      {
        rc= memcached_connect(instance);

        if (memcached_failed(rc))
        {
          memcached_io_reset(instance);
          dead_servers[server]= true;
          success= false;
          continue;
        }
      }

      protocol_binary_request_getk request= {};
      initialize_binary_request(instance, request.message.header);
      request.message.header.request.opcode= PROTOCOL_BINARY_CMD_GETK;
      request.message.header.request.keylen= htons((uint16_t)(key_length[x] + memcached_array_size(ptr->_namespace)));
      request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;
      request.message.header.request.bodylen= htonl((uint32_t)(key_length[x] + memcached_array_size(ptr->_namespace)));

      /*
       * We need to disable buffering to actually know that the request was
       * successfully sent to the server (so that we should expect a result
       * back). It would be nice to do this in buffered mode, but then it
       * would be complex to handle all error situations if we got to send
       * some of the messages, and then we failed on writing out some others
       * and we used the callback interface from memcached_mget_execute so
       * that we might have processed some of the responses etc. For now,
       * just make sure we work _correctly_
     */
      libmemcached_io_vector_st vector[]=
      {
        { request.bytes, sizeof(request.bytes) },
        { memcached_array_string(ptr->_namespace), memcached_array_size(ptr->_namespace) },
        { keys[x], key_length[x] }
      };

      if (memcached_io_writev(instance, vector, 3, true) == false)
      {
        memcached_io_reset(instance);
        dead_servers[server]= true;
        success= false;
        continue;
      }

      memcached_server_response_increment(instance);
      hash[x]= memcached_server_count(ptr);
    }

    if (success)
    {
      break;
    }
  }

  return rc;
}

static memcached_return_t binary_mget_by_key(memcached_st *ptr,
                                             const uint32_t master_server_key,
                                             bool is_group_key_set,
                                             const char * const *keys,
                                             const size_t *key_length,
                                             const size_t number_of_keys,
                                             const bool mget_mode)
{
  if (ptr->number_of_replicas == 0)
  {
    return simple_binary_mget(ptr, master_server_key, is_group_key_set,
                              keys, key_length, number_of_keys, mget_mode);
  }

  uint32_t* hash= libmemcached_xvalloc(ptr, number_of_keys, uint32_t);
  bool* dead_servers= libmemcached_xcalloc(ptr, memcached_server_count(ptr), bool);

  if (hash == NULL or dead_servers == NULL)
  {
    libmemcached_free(ptr, hash);
    libmemcached_free(ptr, dead_servers);
    return MEMCACHED_MEMORY_ALLOCATION_FAILURE;
  }

  if (is_group_key_set)
  {
    for (size_t x= 0; x < number_of_keys; x++)
    {
      hash[x]= master_server_key;
    }
  }
  else
  {
    for (size_t x= 0; x < number_of_keys; x++)
    {
      hash[x]= memcached_generate_hash_with_redistribution(ptr, keys[x], key_length[x]);
    }
  }

  memcached_return_t rc= replication_binary_mget(ptr, hash, dead_servers, keys,
                                                 key_length, number_of_keys);

  WATCHPOINT_IFERROR(rc);
  libmemcached_free(ptr, hash);
  libmemcached_free(ptr, dead_servers);

  return MEMCACHED_SUCCESS;
}
