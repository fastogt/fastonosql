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
#include <libmemcached/memcached/protocol_binary.h>

memcached_return_t memcached_delete(memcached_st *shell, const char *key, size_t key_length,
                                    time_t expiration)
{
  return memcached_delete_by_key(shell, key, key_length, key, key_length, expiration);
}

static inline memcached_return_t ascii_delete(memcached_instance_st* instance,
                                              uint32_t ,
                                              const char *key,
                                              const size_t key_length,
                                              const bool reply,
                                              const bool is_buffering)
{
  libmemcached_io_vector_st vector[]=
  {
    { NULL, 0 },
    { memcached_literal_param("delete ") },
    { memcached_array_string(instance->root->_namespace), memcached_array_size(instance->root->_namespace) },
    { key, key_length },
    { " noreply", reply ? 0 : memcached_literal_param_size(" noreply") },
    { memcached_literal_param("\r\n") }
  };

  /* Send command header, only flush if we are NOT buffering */
  return memcached_vdo(instance, vector, 6, is_buffering ? false : true);
}

static inline memcached_return_t binary_delete(memcached_instance_st* instance,
                                               uint32_t server_key,
                                               const char *key,
                                               const size_t key_length,
                                               const bool reply,
                                               const bool is_buffering)
{
  protocol_binary_request_delete request= {};

  bool should_flush= is_buffering ? false : true;

  initialize_binary_request(instance, request.message.header);

  if (reply)
  {
    request.message.header.request.opcode= PROTOCOL_BINARY_CMD_DELETE;
  }
  else
  {
    request.message.header.request.opcode= PROTOCOL_BINARY_CMD_DELETEQ;
  }
  request.message.header.request.keylen= htons(uint16_t(key_length + memcached_array_size(instance->root->_namespace)));
  request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;
  request.message.header.request.bodylen= htonl(uint32_t(key_length + memcached_array_size(instance->root->_namespace)));

  libmemcached_io_vector_st vector[]=
  {
    { NULL, 0 },
    { request.bytes, sizeof(request.bytes) },
    { memcached_array_string(instance->root->_namespace), memcached_array_size(instance->root->_namespace) },
    { key, key_length }
  };

  memcached_return_t rc;
  if (memcached_fatal(rc= memcached_vdo(instance, vector,  4, should_flush)))
  {
    assert(memcached_last_error(instance->root) != MEMCACHED_SUCCESS);
    memcached_io_reset(instance);
  }

  if (memcached_has_replicas(instance))
  {
    request.message.header.request.opcode= PROTOCOL_BINARY_CMD_DELETEQ;

    for (uint32_t x= 0; x < memcached_has_replicas(instance); ++x)
    {
      ++server_key;

      if (server_key == memcached_server_count(instance->root))
      {
        server_key= 0;
      }

      memcached_instance_st* replica= memcached_instance_fetch(instance->root, server_key);

      if (memcached_fatal(memcached_vdo(replica, vector, 4, should_flush)))
      {
        assert(memcached_last_error(instance->root) != MEMCACHED_SUCCESS);
        memcached_io_reset(replica);
      }
      else
      {
        memcached_server_response_decrement(replica);
      }
    }
  }

  return rc;
}

memcached_return_t memcached_delete_by_key(memcached_st *shell,
                                           const char *group_key, size_t group_key_length,
                                           const char *key, size_t key_length,
                                           time_t expiration)
{
  Memcached* memc= memcached2Memcached(shell);
  LIBMEMCACHED_MEMCACHED_DELETE_START();

  memcached_return_t rc;
  if (memcached_fatal(rc= initialize_query(memc, true)))
  {
    return rc;
  }

  if (memcached_fatal(rc= memcached_key_test(*memc, (const char **)&key, &key_length, 1)))
  {
    return memcached_last_error(memc);
  }

  if (expiration)
  {
    return memcached_set_error(*memc, MEMCACHED_INVALID_ARGUMENTS, MEMCACHED_AT, 
                               memcached_literal_param("Memcached server version does not allow expiration of deleted items"));
  }

  uint32_t server_key= memcached_generate_hash_with_redistribution(memc, group_key, group_key_length);
  memcached_instance_st* instance= memcached_instance_fetch(memc, server_key);
  
  bool is_buffering= memcached_is_buffering(instance->root);
  bool is_replying= memcached_is_replying(instance->root);

  // If a delete trigger exists, we need a response, so no buffering/noreply
  if (memc->delete_trigger)
  {
    if (is_buffering)
    {
      return memcached_set_error(*memc, MEMCACHED_INVALID_ARGUMENTS, MEMCACHED_AT, 
                                 memcached_literal_param("Delete triggers cannot be used if buffering is enabled"));
    }

    if (is_replying == false)
    {
      return memcached_set_error(*memc, MEMCACHED_INVALID_ARGUMENTS, MEMCACHED_AT, 
                                 memcached_literal_param("Delete triggers cannot be used if MEMCACHED_BEHAVIOR_NOREPLY is set"));
    }
  }

  if (memcached_is_binary(memc))
  {
    rc= binary_delete(instance, server_key, key, key_length, is_replying, is_buffering);
  }
  else
  {
    rc= ascii_delete(instance, server_key, key, key_length, is_replying, is_buffering);
  }

  if (rc == MEMCACHED_SUCCESS)
  {
    if (is_buffering == true)
    {
      rc= MEMCACHED_BUFFERED;
    }
    else if (is_replying == false)
    {
      rc= MEMCACHED_SUCCESS;
    }
    else
    {
      char buffer[MEMCACHED_DEFAULT_COMMAND_SIZE];
      rc= memcached_response(instance, buffer, MEMCACHED_DEFAULT_COMMAND_SIZE, NULL);
      if (rc == MEMCACHED_DELETED)
      {
        rc= MEMCACHED_SUCCESS;
        if (memc->delete_trigger)
        {
          memc->delete_trigger(memc, key, key_length);
        }
      }
    }
  }

  LIBMEMCACHED_MEMCACHED_DELETE_END();
  return rc;
}
