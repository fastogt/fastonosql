/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Libmemcached library
 *
 *  Copyright (C) 2011 Data Differential, http://datadifferential.com/
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

static memcached_return_t memcached_flush_binary(Memcached *ptr, 
                                                 time_t expiration,
                                                 const bool reply)
{
  protocol_binary_request_flush request= {};

  request.message.header.request.opcode= PROTOCOL_BINARY_CMD_FLUSH;
  request.message.header.request.extlen= 4;
  request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;
  request.message.header.request.bodylen= htonl(request.message.header.request.extlen);
  request.message.body.expiration= htonl((uint32_t) expiration);

  memcached_return_t rc= MEMCACHED_SUCCESS;

  for (uint32_t x= 0; x < memcached_server_count(ptr); x++)
  {
    memcached_instance_st* instance= memcached_instance_fetch(ptr, x);
    initialize_binary_request(instance, request.message.header);

    if (reply)
    {
      request.message.header.request.opcode= PROTOCOL_BINARY_CMD_FLUSH;
    }
    else
    {
      request.message.header.request.opcode= PROTOCOL_BINARY_CMD_FLUSHQ;
    }

    libmemcached_io_vector_st vector[]=
    {
      { NULL, 0 },
      { request.bytes, sizeof(request.bytes) }
    };

    memcached_return_t rrc;
    if (memcached_failed(rrc= memcached_vdo(instance, vector, 2, true)))
    {
      if (instance->error_messages == NULL or instance->root->error_messages == NULL)
      {
        memcached_set_error(*instance, rrc, MEMCACHED_AT);
      }
      memcached_io_reset(instance);
      rc= MEMCACHED_SOME_ERRORS;
    } 
  }

  for (uint32_t x= 0; x < memcached_server_count(ptr); x++)
  {
    memcached_instance_st* instance= memcached_instance_fetch(ptr, x);

    if (instance->response_count() > 0)
    {
      (void)memcached_response(instance, NULL, 0, NULL);
    }
  }

  return rc;
}

static memcached_return_t memcached_flush_textual(Memcached *ptr, 
                                                  time_t expiration,
                                                  const bool reply)
{
  char buffer[MEMCACHED_MAXIMUM_INTEGER_DISPLAY_LENGTH +1];
  int send_length= 0;
  if (expiration)
  {
    send_length= snprintf(buffer, sizeof(buffer), "%llu", (unsigned long long)expiration);
  }

  if (size_t(send_length) >= sizeof(buffer) or send_length < 0)
  {
    return memcached_set_error(*ptr, MEMCACHED_MEMORY_ALLOCATION_FAILURE, MEMCACHED_AT, 
                               memcached_literal_param("snprintf(MEMCACHED_DEFAULT_COMMAND_SIZE)"));
  }

  memcached_return_t rc= MEMCACHED_SUCCESS;
  for (uint32_t x= 0; x < memcached_server_count(ptr); x++)
  {
    memcached_instance_st* instance= memcached_instance_fetch(ptr, x);

    libmemcached_io_vector_st vector[]=
    {
      { NULL, 0 },
      { memcached_literal_param("flush_all ") },
      { buffer, size_t(send_length) },
      { " noreply", reply ? 0 : memcached_literal_param_size(" noreply") },
      { memcached_literal_param("\r\n") }
    };

    memcached_return_t rrc= memcached_vdo(instance, vector, 5, true);
    if (memcached_success(rrc) and reply == true)
    {
      char response_buffer[MEMCACHED_DEFAULT_COMMAND_SIZE];
      rrc= memcached_response(instance, response_buffer, sizeof(response_buffer), NULL);
    }

    if (memcached_failed(rrc))
    {
      // If an error has already been reported, then don't add to it
      if (instance->error_messages == NULL or instance->root->error_messages == NULL)
      {
        memcached_set_error(*instance, rrc, MEMCACHED_AT);
      }
      rc= MEMCACHED_SOME_ERRORS;
    }
  }

  return rc;
}

memcached_return_t memcached_flush(memcached_st *shell, time_t expiration)
{
  Memcached* ptr= memcached2Memcached(shell);
  memcached_return_t rc;
  if (memcached_failed(rc= initialize_query(ptr, true)))
  {
    return rc;
  }

  bool reply= memcached_is_replying(ptr);

  LIBMEMCACHED_MEMCACHED_FLUSH_START();
  if (memcached_is_binary(ptr))
  {
    rc= memcached_flush_binary(ptr, expiration, reply);
  }
  else
  {
    rc= memcached_flush_textual(ptr, expiration, reply);
  }
  LIBMEMCACHED_MEMCACHED_FLUSH_END();

  return rc;
}
