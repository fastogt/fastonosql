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

static memcached_return_t ascii_touch(memcached_instance_st* instance,
                                      const char *key, size_t key_length,
                                      time_t expiration)
{
  char expiration_buffer[MEMCACHED_MAXIMUM_INTEGER_DISPLAY_LENGTH +1];
  int expiration_buffer_length= snprintf(expiration_buffer, sizeof(expiration_buffer), " %llu", (unsigned long long)expiration);
  if (size_t(expiration_buffer_length) >= sizeof(expiration_buffer) or expiration_buffer_length < 0)
  {
    return memcached_set_error(*instance, MEMCACHED_MEMORY_ALLOCATION_FAILURE, MEMCACHED_AT, 
                               memcached_literal_param("snprintf(MEMCACHED_MAXIMUM_INTEGER_DISPLAY_LENGTH)"));
  }

  libmemcached_io_vector_st vector[]=
  {
    { NULL, 0 },
    { memcached_literal_param("touch ") },
    { memcached_array_string(instance->root->_namespace), memcached_array_size(instance->root->_namespace) },
    { key, key_length },
    { expiration_buffer, size_t(expiration_buffer_length) },
    { memcached_literal_param("\r\n") }
  };

  memcached_return_t rc;
  if (memcached_failed(rc= memcached_vdo(instance, vector, 6, true)))
  {
    memcached_io_reset(instance);
    return memcached_set_error(*instance, MEMCACHED_WRITE_FAILURE, MEMCACHED_AT);
  }

  return rc;
}

static memcached_return_t binary_touch(memcached_instance_st* instance,
                                       const char *key, size_t key_length,
                                       time_t expiration)
{
  protocol_binary_request_touch request= {}; //{.bytes= {0}};

  initialize_binary_request(instance, request.message.header);

  request.message.header.request.opcode= PROTOCOL_BINARY_CMD_TOUCH;
  request.message.header.request.extlen= 4;
  request.message.header.request.keylen= htons((uint16_t)(key_length +memcached_array_size(instance->root->_namespace)));
  request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;
  request.message.header.request.bodylen= htonl((uint32_t)(key_length +memcached_array_size(instance->root->_namespace) +request.message.header.request.extlen));
  request.message.body.expiration= htonl((uint32_t) expiration);

  libmemcached_io_vector_st vector[]=
  {
    { NULL, 0 },
    { request.bytes, sizeof(request.bytes) },
    { memcached_array_string(instance->root->_namespace), memcached_array_size(instance->root->_namespace) },
    { key, key_length }
  };

  memcached_return_t rc;
  if (memcached_failed(rc= memcached_vdo(instance, vector, 4, true)))
  {
    memcached_io_reset(instance);
    return memcached_set_error(*instance, MEMCACHED_WRITE_FAILURE, MEMCACHED_AT);
  }

  return rc;
}

memcached_return_t memcached_touch(memcached_st *ptr,
                                   const char *key, size_t key_length,
                                   time_t expiration)
{
  return memcached_touch_by_key(ptr, key, key_length, key, key_length, expiration);
}

memcached_return_t memcached_touch_by_key(memcached_st *shell,
                                          const char *group_key, size_t group_key_length,
                                          const char *key, size_t key_length,
                                          time_t expiration)
{
  Memcached* ptr= memcached2Memcached(shell);
  LIBMEMCACHED_MEMCACHED_TOUCH_START();

  memcached_return_t rc;
  if (memcached_failed(rc= initialize_query(ptr, true)))
  {
    return rc;
  }

  if (memcached_failed(rc= memcached_key_test(*ptr, (const char **)&key, &key_length, 1)))
  {
    return memcached_set_error(*ptr, rc, MEMCACHED_AT);
  }

  uint32_t server_key= memcached_generate_hash_with_redistribution(ptr, group_key, group_key_length);
  memcached_instance_st* instance= memcached_instance_fetch(ptr, server_key);

  if (ptr->flags.binary_protocol)
  {
    rc= binary_touch(instance, key, key_length, expiration);
  }
  else
  {
    rc= ascii_touch(instance, key, key_length, expiration);
  }

  if (memcached_failed(rc))
  {
    return memcached_set_error(*instance, rc, MEMCACHED_AT, memcached_literal_param("Error occcured while writing touch command to server"));
  }

  char buffer[MEMCACHED_DEFAULT_COMMAND_SIZE];
  rc= memcached_response(instance, buffer, sizeof(buffer), NULL);

  if (rc == MEMCACHED_SUCCESS or rc == MEMCACHED_NOTFOUND)
  {
    return rc;
  }

  return memcached_set_error(*instance, rc, MEMCACHED_AT, memcached_literal_param("Error occcured while reading response"));
}
