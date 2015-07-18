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

static memcached_return_t ascii_exist(Memcached *memc, memcached_instance_st* instance, const char *key, size_t key_length)
{
  libmemcached_io_vector_st vector[]=
  {
    { NULL, 0 },
    { memcached_literal_param("add ") },
    { memcached_array_string(memc->_namespace), memcached_array_size(memc->_namespace) },
    { key, key_length },
    { memcached_literal_param(" 0") },
    { memcached_literal_param(" 2678400") },
    { memcached_literal_param(" 0") },
    { memcached_literal_param("\r\n") },
    { memcached_literal_param("\r\n") }
  };

  /* Send command header */
  memcached_return_t rc;
  if (memcached_fatal(rc= memcached_vdo(instance, vector, 9, true)))
  {
    return rc;
  }

  char buffer[MEMCACHED_DEFAULT_COMMAND_SIZE];
  rc= memcached_response(instance, buffer, MEMCACHED_DEFAULT_COMMAND_SIZE, NULL);

  if (rc == MEMCACHED_NOTSTORED)
  {
    rc= MEMCACHED_SUCCESS;
  }

  if (rc == MEMCACHED_STORED)
  {
    rc= MEMCACHED_NOTFOUND;
  }

  return rc;
}

static memcached_return_t binary_exist(Memcached *memc, memcached_instance_st* instance, const char *key, size_t key_length)
{
  protocol_binary_request_set request= {};
  size_t send_length= sizeof(request.bytes);

  initialize_binary_request(instance, request.message.header);

  request.message.header.request.opcode= PROTOCOL_BINARY_CMD_ADD;
  request.message.header.request.keylen= htons((uint16_t)(key_length + memcached_array_size(memc->_namespace)));
  request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;
  request.message.header.request.extlen= 8;
  request.message.body.flags= 0;
  request.message.body.expiration= htonl(2678400);

  request.message.header.request.bodylen= htonl((uint32_t) (key_length 
                                                            +memcached_array_size(memc->_namespace)
                                                            +request.message.header.request.extlen));

  libmemcached_io_vector_st vector[]=
  {
    { NULL, 0 },
    { request.bytes, send_length },
    { memcached_array_string(memc->_namespace), memcached_array_size(memc->_namespace) },
    { key, key_length }
  };

  /* write the header */
  memcached_return_t rc;
  if (memcached_fatal(rc= memcached_vdo(instance, vector, 4, true)))
  {
    return rc;
  }

  rc= memcached_response(instance, NULL, 0, NULL);

  if (rc == MEMCACHED_SUCCESS)
  {
    rc= MEMCACHED_NOTFOUND;
  }

  if (rc == MEMCACHED_DATA_EXISTS)
  {
    rc= MEMCACHED_SUCCESS;
  }

  return rc;
}

memcached_return_t memcached_exist(memcached_st *memc, const char *key, size_t key_length)
{
  return memcached_exist_by_key(memc, key, key_length, key, key_length);
}

memcached_return_t memcached_exist_by_key(memcached_st *shell,
                                          const char *group_key, size_t group_key_length,
                                          const char *key, size_t key_length)
{
  Memcached* memc= memcached2Memcached(shell);
  memcached_return_t rc;
  if (memcached_failed(rc= initialize_query(memc, true)))
  {
    return rc;
  }

  if (memcached_is_udp(memc))
  {
    return memcached_set_error(*memc, MEMCACHED_NOT_SUPPORTED, MEMCACHED_AT);
  }

  uint32_t server_key= memcached_generate_hash_with_redistribution(memc, group_key, group_key_length);
  memcached_instance_st* instance= memcached_instance_fetch(memc, server_key);

  if (memcached_is_binary(memc))
  {
    rc= binary_exist(memc, instance, key, key_length);
  }
  else
  {
    rc= ascii_exist(memc, instance, key, key_length);
  }

  if (memcached_fatal(rc))
  {
    memcached_io_reset(instance);
  }

  return rc;
}
