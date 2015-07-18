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

static void auto_response(memcached_instance_st* instance, const bool reply,  memcached_return_t& rc, uint64_t* value)
{
  // If the message was successfully sent, then get the response, otherwise
  // fail.
  if (memcached_success(rc))
  {
    if (reply == false)
    {
      *value= UINT64_MAX;
      return;
    }

    rc= memcached_response(instance, &instance->root->result);
  }

  if (memcached_fatal(rc))
  {
    assert(memcached_last_error(instance->root) != MEMCACHED_SUCCESS);
    *value= UINT64_MAX;
  }
  else if (memcached_failed(rc))
  {
    *value= UINT64_MAX;
  }
  else
  {
    assert(memcached_last_error(instance->root) != MEMCACHED_NOTFOUND);
    *value= instance->root->result.numeric_value;
  }
}

static memcached_return_t text_incr_decr(memcached_instance_st* instance,
                                         const bool is_incr,
                                         const char *key, size_t key_length,
                                         const uint64_t offset,
                                         const bool reply)
{
  char buffer[MEMCACHED_DEFAULT_COMMAND_SIZE];

  int send_length= snprintf(buffer, sizeof(buffer), " %" PRIu64, offset);
  if (size_t(send_length) >= sizeof(buffer) or send_length < 0)
  {
    return memcached_set_error(*instance, MEMCACHED_MEMORY_ALLOCATION_FAILURE, MEMCACHED_AT, 
                               memcached_literal_param("snprintf(MEMCACHED_DEFAULT_COMMAND_SIZE)"));
  }

  libmemcached_io_vector_st vector[]=
  {
    { NULL, 0 },
    { memcached_literal_param("incr ") },
    { memcached_array_string(instance->root->_namespace), memcached_array_size(instance->root->_namespace) },
    { key, key_length },
    { buffer, size_t(send_length) },
    { " noreply", reply ? 0 : memcached_literal_param_size(" noreply") },
    { memcached_literal_param("\r\n") }
  };

  if (is_incr == false)
  {
    vector[1].buffer= "decr ";
  }

  return memcached_vdo(instance, vector, 7, true);
}

static memcached_return_t binary_incr_decr(memcached_instance_st* instance,
                                           protocol_binary_command cmd,
                                           const char *key, const size_t key_length,
                                           const uint64_t offset,
                                           const uint64_t initial,
                                           const uint32_t expiration,
                                           const bool reply)
{
  if (reply == false)
  {
    if(cmd == PROTOCOL_BINARY_CMD_DECREMENT)
    {
      cmd= PROTOCOL_BINARY_CMD_DECREMENTQ;
    }

    if(cmd == PROTOCOL_BINARY_CMD_INCREMENT)
    {
      cmd= PROTOCOL_BINARY_CMD_INCREMENTQ;
    }
  }
  protocol_binary_request_incr request= {}; // = {.bytes= {0}};

  initialize_binary_request(instance, request.message.header);

  request.message.header.request.opcode= cmd;
  request.message.header.request.keylen= htons((uint16_t)(key_length + memcached_array_size(instance->root->_namespace)));
  request.message.header.request.extlen= 20;
  request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;
  request.message.header.request.bodylen= htonl((uint32_t)(key_length + memcached_array_size(instance->root->_namespace) +request.message.header.request.extlen));
  request.message.body.delta= memcached_htonll(offset);
  request.message.body.initial= memcached_htonll(initial);
  request.message.body.expiration= htonl((uint32_t) expiration);

  libmemcached_io_vector_st vector[]=
  {
    { NULL, 0 },
    { request.bytes, sizeof(request.bytes) },
    { memcached_array_string(instance->root->_namespace), memcached_array_size(instance->root->_namespace) },
    { key, key_length }
  };

  return memcached_vdo(instance, vector, 4, true);
}

memcached_return_t memcached_increment(memcached_st *memc,
                                       const char *key, size_t key_length,
                                       uint32_t offset,
                                       uint64_t *value)
{
  return memcached_increment_by_key(memc, key, key_length, key, key_length, offset, value);
}

static memcached_return_t increment_decrement_by_key(const protocol_binary_command command,
                                                     Memcached *memc,
                                                     const char *group_key, size_t group_key_length,
                                                     const char *key, size_t key_length,
                                                     uint64_t offset,
                                                     uint64_t *value)
{
  uint64_t local_value;
  if (value == NULL)
  {
    value= &local_value;
  }

  memcached_return_t rc;
  if (memcached_failed(rc= initialize_query(memc, true)))
  {
    return rc;
  }

  if (memcached_is_encrypted(memc))
  {
    return memcached_set_error(*memc, MEMCACHED_NOT_SUPPORTED, MEMCACHED_AT, 
                               memcached_literal_param("Operation not allowed while encyrption is enabled"));
  }

  if (memcached_failed(rc= memcached_key_test(*memc, (const char **)&key, &key_length, 1)))
  {
    return memcached_last_error(memc);
  }

  uint32_t server_key= memcached_generate_hash_with_redistribution(memc, group_key, group_key_length);
  memcached_instance_st* instance= memcached_instance_fetch(memc, server_key);

  bool reply= memcached_is_replying(instance->root);

  if (memcached_is_binary(memc))
  {
    rc= binary_incr_decr(instance, command,
                         key, key_length,
                         uint64_t(offset), 0, MEMCACHED_EXPIRATION_NOT_ADD,
                         reply);
  }
  else
  {
    rc= text_incr_decr(instance,
                       command == PROTOCOL_BINARY_CMD_INCREMENT ? true : false,
                       key, key_length,
                       offset, reply);
  }

  auto_response(instance, reply, rc, value);

  return rc;
}

static memcached_return_t increment_decrement_with_initial_by_key(const protocol_binary_command command,
                                                                  Memcached *memc,
                                                                  const char *group_key,
                                                                  size_t group_key_length,
                                                                  const char *key,
                                                                  size_t key_length,
                                                                  uint64_t offset,
                                                                  uint64_t initial,
                                                                  time_t expiration,
                                                                  uint64_t *value)
{
  uint64_t local_value;
  if (value == NULL)
  {
    value= &local_value;
  }

  memcached_return_t rc;
  if (memcached_failed(rc= initialize_query(memc, true)))
  {
    return rc;
  }

  if (memcached_is_encrypted(memc))
  {
    return memcached_set_error(*memc, MEMCACHED_NOT_SUPPORTED, MEMCACHED_AT, 
                               memcached_literal_param("Operation not allowed while encyrption is enabled"));
  }

  if (memcached_failed(rc= memcached_key_test(*memc, (const char **)&key, &key_length, 1)))
  {
    return memcached_last_error(memc);
  }

  uint32_t server_key= memcached_generate_hash_with_redistribution(memc, group_key, group_key_length);
  memcached_instance_st* instance= memcached_instance_fetch(memc, server_key);

  bool reply= memcached_is_replying(instance->root);

  if (memcached_is_binary(memc))
  {
    rc= binary_incr_decr(instance, command,
                         key, key_length,
                         offset, initial, uint32_t(expiration),
                         reply);
        
  }
  else
  {
    rc=  memcached_set_error(*memc, MEMCACHED_INVALID_ARGUMENTS, MEMCACHED_AT,
                             memcached_literal_param("memcached_increment_with_initial_by_key() is not supported via the ASCII protocol"));
  }

  auto_response(instance, reply, rc, value);

  return rc;
}

memcached_return_t memcached_decrement(memcached_st *memc,
                                       const char *key, size_t key_length,
                                       uint32_t offset,
                                       uint64_t *value)
{
  return memcached_decrement_by_key(memc, key, key_length, key, key_length, offset, value);
}


memcached_return_t memcached_increment_by_key(memcached_st *shell,
                                              const char *group_key, size_t group_key_length,
                                              const char *key, size_t key_length,
                                              uint64_t offset,
                                              uint64_t *value)
{
  Memcached* memc= memcached2Memcached(shell);
  LIBMEMCACHED_MEMCACHED_INCREMENT_START();
  memcached_return_t rc= increment_decrement_by_key(PROTOCOL_BINARY_CMD_INCREMENT,
                                                    memc,
                                                    group_key, group_key_length,
                                                    key, key_length,
                                                    offset, value);

  LIBMEMCACHED_MEMCACHED_INCREMENT_END();

  return rc;
}

memcached_return_t memcached_decrement_by_key(memcached_st *shell,
                                              const char *group_key, size_t group_key_length,
                                              const char *key, size_t key_length,
                                              uint64_t offset,
                                              uint64_t *value)
{
  Memcached* memc= memcached2Memcached(shell);
  LIBMEMCACHED_MEMCACHED_DECREMENT_START();
  memcached_return_t rc= increment_decrement_by_key(PROTOCOL_BINARY_CMD_DECREMENT,
                                                    memc,
                                                    group_key, group_key_length,
                                                    key, key_length,
                                                    offset, value);
  LIBMEMCACHED_MEMCACHED_DECREMENT_END();

  return rc;
}

memcached_return_t memcached_increment_with_initial(memcached_st *memc,
                                                    const char *key,
                                                    size_t key_length,
                                                    uint64_t offset,
                                                    uint64_t initial,
                                                    time_t expiration,
                                                    uint64_t *value)
{
  return memcached_increment_with_initial_by_key(memc, key, key_length,
                                                 key, key_length,
                                                 offset, initial, expiration, value);
}

memcached_return_t memcached_increment_with_initial_by_key(memcached_st *shell,
                                                           const char *group_key,
                                                           size_t group_key_length,
                                                           const char *key,
                                                           size_t key_length,
                                                           uint64_t offset,
                                                           uint64_t initial,
                                                           time_t expiration,
                                                           uint64_t *value)
{
  LIBMEMCACHED_MEMCACHED_INCREMENT_WITH_INITIAL_START();
  Memcached* memc= memcached2Memcached(shell);
  memcached_return_t rc= increment_decrement_with_initial_by_key(PROTOCOL_BINARY_CMD_INCREMENT, 
                                                                 memc,
                                                                 group_key, group_key_length,
                                                                 key, key_length,
                                                                 offset, initial, expiration, value);
  LIBMEMCACHED_MEMCACHED_INCREMENT_WITH_INITIAL_END();

  return rc;
}

memcached_return_t memcached_decrement_with_initial(memcached_st *memc,
                                                    const char *key,
                                                    size_t key_length,
                                                    uint64_t offset,
                                                    uint64_t initial,
                                                    time_t expiration,
                                                    uint64_t *value)
{
  return memcached_decrement_with_initial_by_key(memc, key, key_length,
                                                 key, key_length,
                                                 offset, initial, expiration, value);
}

memcached_return_t memcached_decrement_with_initial_by_key(memcached_st *shell,
                                                           const char *group_key,
                                                           size_t group_key_length,
                                                           const char *key,
                                                           size_t key_length,
                                                           uint64_t offset,
                                                           uint64_t initial,
                                                           time_t expiration,
                                                           uint64_t *value)
{
  LIBMEMCACHED_MEMCACHED_INCREMENT_WITH_INITIAL_START();
  Memcached* memc= memcached2Memcached(shell);
  memcached_return_t rc= increment_decrement_with_initial_by_key(PROTOCOL_BINARY_CMD_DECREMENT, 
                                                                 memc,
                                                                 group_key, group_key_length,
                                                                 key, key_length,
                                                                 offset, initial, expiration, value);

  LIBMEMCACHED_MEMCACHED_INCREMENT_WITH_INITIAL_END();

  return rc;
}
