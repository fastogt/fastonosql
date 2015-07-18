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
#include <libmemcached/string.hpp>

static memcached_return_t textual_value_fetch(memcached_instance_st* instance,
                                              char *buffer,
                                              memcached_result_st *result)
{
  char *next_ptr;
  ssize_t read_length= 0;
  size_t value_length;

  WATCHPOINT_ASSERT(instance->root);
  char *end_ptr= buffer + MEMCACHED_DEFAULT_COMMAND_SIZE;

  memcached_result_reset(result);

  char *string_ptr= buffer;
  string_ptr+= 6; /* "VALUE " */


  // Just used for cases of AES decrypt currently
  memcached_return_t rc= MEMCACHED_SUCCESS;

  /* We load the key */
  {
    char *key= result->item_key;
    result->key_length= 0;

    for (ptrdiff_t prefix_length= memcached_array_size(instance->root->_namespace); !(iscntrl(*string_ptr) || isspace(*string_ptr)) ; string_ptr++)
    {
      if (prefix_length == 0)
      {
        *key= *string_ptr;
        key++;
        result->key_length++;
      }
      else
        prefix_length--;
    }
    result->item_key[result->key_length]= 0;
  }

  if (end_ptr == string_ptr)
  {
    goto read_error;
  }

  /* Flags fetch move past space */
  string_ptr++;
  if (end_ptr == string_ptr)
  {
    goto read_error;
  }

  for (next_ptr= string_ptr; isdigit(*string_ptr); string_ptr++) {};
  errno= 0;
  result->item_flags= (uint32_t) strtoul(next_ptr, &string_ptr, 10);

  if (errno != 0 or end_ptr == string_ptr)
  {
    goto read_error;
  }

  /* Length fetch move past space*/
  string_ptr++;
  if (end_ptr == string_ptr)
  {
    goto read_error;
  }

  for (next_ptr= string_ptr; isdigit(*string_ptr); string_ptr++) {};
  errno= 0;
  value_length= (size_t)strtoull(next_ptr, &string_ptr, 10);

  if (errno != 0 or end_ptr == string_ptr)
  {
    goto read_error;
  }

  /* Skip spaces */
  if (*string_ptr == '\r')
  {
    /* Skip past the \r\n */
    string_ptr+= 2;
  }
  else
  {
    string_ptr++;
    for (next_ptr= string_ptr; isdigit(*string_ptr); string_ptr++) {};
    errno= 0;
    result->item_cas= strtoull(next_ptr, &string_ptr, 10);
  }

  if (errno != 0 or end_ptr < string_ptr)
  {
    goto read_error;
  }

  /* We add two bytes so that we can walk the \r\n */
  if (memcached_failed(memcached_string_check(&result->value, value_length +2)))
  {
    return memcached_set_error(*instance, MEMCACHED_MEMORY_ALLOCATION_FAILURE, MEMCACHED_AT);
  }

  {
    char *value_ptr= memcached_string_value_mutable(&result->value);
    /*
      We read the \r\n into the string since not doing so is more
      cycles then the waster of memory to do so.

      We are null terminating through, which will most likely make
      some people lazy about using the return length.
    */
    size_t to_read= (value_length) + 2;
    memcached_return_t rrc= memcached_io_read(instance, value_ptr, to_read, read_length);
    if (memcached_failed(rrc) and rrc == MEMCACHED_IN_PROGRESS)
    {
      memcached_quit_server(instance, true);
      return memcached_set_error(*instance, MEMCACHED_IN_PROGRESS, MEMCACHED_AT);
    }
    else if (memcached_failed(rrc))
    {
      return rrc;
    }
  }

  if (read_length != (ssize_t)(value_length + 2))
  {
    goto read_error;
  }

  /* This next bit blows the API, but this is internal....*/
  {
    char *char_ptr;
    char_ptr= memcached_string_value_mutable(&result->value);;
    char_ptr[value_length]= 0;
    char_ptr[value_length +1]= 0;
    memcached_string_set_length(&result->value, value_length);
  }

  if (memcached_is_encrypted(instance->root) and memcached_result_length(result))
  {
    hashkit_string_st *destination;

    if ((destination= hashkit_decrypt(&instance->root->hashkit,
                                      memcached_result_value(result), memcached_result_length(result))) == NULL)
    {
      rc= memcached_set_error(*instance->root, MEMCACHED_FAILURE, 
                              MEMCACHED_AT, memcached_literal_param("hashkit_decrypt() failed"));
    }
    else
    {
      memcached_result_reset_value(result);
      if (memcached_failed(memcached_result_set_value(result, hashkit_string_c_str(destination), hashkit_string_length(destination))))
      {
        rc= memcached_set_error(*instance->root, MEMCACHED_FAILURE, 
                                MEMCACHED_AT, memcached_literal_param("hashkit_decrypt() failed"));
      }
    }

    if (memcached_failed(rc))
    {
      memcached_result_reset(result);
    }
    hashkit_string_free(destination);
  }

  return rc;

read_error:
  memcached_io_reset(instance);

  return MEMCACHED_PARTIAL_READ;
}

static memcached_return_t textual_read_one_response(memcached_instance_st* instance,
                                                    char *buffer, const size_t buffer_length,
                                                    memcached_result_st *result)
{
  size_t total_read;
  memcached_return_t rc= memcached_io_readline(instance, buffer, buffer_length, total_read);

  if (memcached_failed(rc))
  {
    return rc;
  }
  assert(total_read);

  switch(buffer[0])
  {
  case 'V':
    {
      // VALUE
      if (buffer[1] == 'A' and buffer[2] == 'L' and buffer[3] == 'U' and buffer[4] == 'E') /* VALUE */
      {
        /* We add back in one because we will need to search for END */
        memcached_server_response_increment(instance);
        return textual_value_fetch(instance, buffer, result);
      }
      // VERSION
      else if (buffer[1] == 'E' and buffer[2] == 'R' and buffer[3] == 'S' and buffer[4] == 'I' and buffer[5] == 'O' and buffer[6] == 'N') /* VERSION */
      {
        /* Find the space, and then move one past it to copy version */
        char *response_ptr= index(buffer, ' ');

        char *endptr;
        errno= 0;
        long int version= strtol(response_ptr, &endptr, 10);
        if (errno != 0 or version == LONG_MIN or version == LONG_MAX or version > UINT8_MAX or version == 0)
        {
          instance->major_version= instance->minor_version= instance->micro_version= UINT8_MAX;
          return memcached_set_error(*instance, MEMCACHED_UNKNOWN_READ_FAILURE, MEMCACHED_AT, memcached_literal_param("strtol() failed to parse major version"));
        }
        instance->major_version= uint8_t(version);

        endptr++;
        errno= 0;
        version= strtol(endptr, &endptr, 10);
        if (errno != 0 or version == LONG_MIN or version == LONG_MAX or version > UINT8_MAX)
        {
          instance->major_version= instance->minor_version= instance->micro_version= UINT8_MAX;
          return memcached_set_error(*instance, MEMCACHED_UNKNOWN_READ_FAILURE, MEMCACHED_AT, memcached_literal_param("strtol() failed to parse minor version"));
        }
        instance->minor_version= uint8_t(version);

        endptr++;
        errno= 0;
        version= strtol(endptr, &endptr, 10);
        if (errno != 0 or version == LONG_MIN or version == LONG_MAX or version > UINT8_MAX)
        {
          instance->major_version= instance->minor_version= instance->micro_version= UINT8_MAX;
          return memcached_set_error(*instance, MEMCACHED_UNKNOWN_READ_FAILURE, MEMCACHED_AT, memcached_literal_param("strtol() failed to parse micro version"));
        }
        instance->micro_version= uint8_t(version);

        return MEMCACHED_SUCCESS;
      }
    }
    break;

  case 'O':
    {
      // OK
      if (buffer[1] == 'K')
      {
        return MEMCACHED_SUCCESS;
      }
    }
    break;

  case 'S':
    {
      // STAT
      if (buffer[1] == 'T' and buffer[2] == 'A' and buffer[3] == 'T') /* STORED STATS */
      {
        memcached_server_response_increment(instance);
        return MEMCACHED_STAT;
      }
      // SERVER_ERROR
      else if (buffer[1] == 'E' and buffer[2] == 'R' and buffer[3] == 'V' and buffer[4] == 'E' and buffer[5] == 'R'
               and buffer[6] == '_' 
               and buffer[7] == 'E' and buffer[8] == 'R' and buffer[9] == 'R' and buffer[10] == 'O' and buffer[11] == 'R' )
      {
        if (total_read == memcached_literal_param_size("SERVER_ERROR"))
        {
          return MEMCACHED_SERVER_ERROR;
        }

        if (total_read >= memcached_literal_param_size("SERVER_ERROR object too large for cache") and
            (memcmp(buffer, memcached_literal_param("SERVER_ERROR object too large for cache")) == 0))
        {
          return MEMCACHED_E2BIG;
        }

        if (total_read >= memcached_literal_param_size("SERVER_ERROR out of memory storing object") and
            (memcmp(buffer, memcached_literal_param("SERVER_ERROR out of memory storing object")) == 0))
        {
          return MEMCACHED_SERVER_MEMORY_ALLOCATION_FAILURE;
        }

        // Move past the basic error message and whitespace
        char *startptr= buffer + memcached_literal_param_size("SERVER_ERROR");
        if (startptr[0] == ' ')
        {
          startptr++;
        }

        char *endptr= startptr;
        while (*endptr != '\r' && *endptr != '\n') endptr++;

        return memcached_set_error(*instance, MEMCACHED_SERVER_ERROR, MEMCACHED_AT, startptr, size_t(endptr - startptr));
      }
      // STORED
      else if (buffer[1] == 'T' and buffer[2] == 'O' and buffer[3] == 'R') //  and buffer[4] == 'E' and buffer[5] == 'D')
      {
        return MEMCACHED_STORED;
      }
    }
    break;

  case 'D':
    {
      // DELETED
      if (buffer[1] == 'E' and buffer[2] == 'L' and buffer[3] == 'E' and buffer[4] == 'T' and buffer[5] == 'E' and buffer[6] == 'D')
      {
        return MEMCACHED_DELETED;
      }
    }
    break;

  case 'N':
    {
      // NOT_FOUND
      if (buffer[1] == 'O' and buffer[2] == 'T' 
          and buffer[3] == '_'
          and buffer[4] == 'F' and buffer[5] == 'O' and buffer[6] == 'U' and buffer[7] == 'N' and buffer[8] == 'D')
      {
        return MEMCACHED_NOTFOUND;
      }
      // NOT_STORED
      else if (buffer[1] == 'O' and buffer[2] == 'T' 
               and buffer[3] == '_'
               and buffer[4] == 'S' and buffer[5] == 'T' and buffer[6] == 'O' and buffer[7] == 'R' and buffer[8] == 'E' and buffer[9] == 'D')
      {
        return MEMCACHED_NOTSTORED;
      }
    }
    break;

  case 'E': /* PROTOCOL ERROR or END */
    {
      // END
      if (buffer[1] == 'N' and buffer[2] == 'D')
      {
        return MEMCACHED_END;
      }
#if 0
      // PROTOCOL_ERROR
      else if (buffer[1] == 'R' and buffer[2] == 'O' and buffer[3] == 'T' and buffer[4] == 'O' and buffer[5] == 'C' and buffer[6] == 'O' and buffer[7] == 'L'
               and buffer[8] == '_'
               and buffer[9] == 'E' and buffer[10] == 'R' and buffer[11] == 'R' and buffer[12] == 'O' and buffer[13] == 'R')
      {
        return MEMCACHED_PROTOCOL_ERROR;
      }
#endif
      // ERROR
      else if (buffer[1] == 'R' and buffer[2] == 'R' and buffer[3] == 'O' and buffer[4] == 'R')
      {
        return MEMCACHED_ERROR;
      }
      // EXISTS
      else if (buffer[1] == 'X' and buffer[2] == 'I' and buffer[3] == 'S' and buffer[4] == 'T' and buffer[5] == 'S')
      {
        return MEMCACHED_DATA_EXISTS;
      }
    }
    break;

  case 'T': /* TOUCHED */
    {
      // TOUCHED
      if (buffer[1] == 'O' and buffer[2] == 'U' and buffer[3] == 'C' and buffer[4] == 'H' and buffer[5] == 'E' and buffer[6] == 'D')
      {
        return MEMCACHED_SUCCESS;
      }
    }
    break;

  case 'I': /* ITEM */
    {
      // ITEM
      if (buffer[1] == 'T' and buffer[2] == 'E' and buffer[3] == 'M')
      {
        /* We add back in one because we will need to search for END */
        memcached_server_response_increment(instance);
        return MEMCACHED_ITEM;
      }
    }
    break;

  case 'C': /* CLIENT ERROR */
    {
      // CLIENT_ERROR
      if (buffer[1] == 'L' and buffer[2] == 'I' and buffer[3] == 'E' and buffer[4] == 'N' and buffer[5] == 'T'
          and buffer[6] == '_'
          and buffer[7] == 'E' and buffer[8] == 'R' and buffer[9] == 'R' and buffer[10] == 'O' and buffer[11] == 'R')
      {
        // Move past the basic error message and whitespace
        char *startptr= buffer + memcached_literal_param_size("CLIENT_ERROR");
        if (startptr[0] == ' ')
        {
          startptr++;
        }

        char *endptr= startptr;
        while (*endptr != '\r' && *endptr != '\n') endptr++;

        return memcached_set_error(*instance, MEMCACHED_CLIENT_ERROR, MEMCACHED_AT, startptr, size_t(endptr - startptr));
      }
    }
    break;

  case '0': /* INCR/DECR response */
  case '1': /* INCR/DECR response */
  case '2': /* INCR/DECR response */
  case '3': /* INCR/DECR response */
  case '4': /* INCR/DECR response */
  case '5': /* INCR/DECR response */
  case '6': /* INCR/DECR response */
  case '7': /* INCR/DECR response */
  case '8': /* INCR/DECR response */
  case '9': /* INCR/DECR response */
    {
      errno= 0;
      unsigned long long int auto_return_value= strtoull(buffer, (char **)NULL, 10);

      if (auto_return_value == ULLONG_MAX and errno == ERANGE)
      {
        result->numeric_value= UINT64_MAX;
        return memcached_set_error(*instance, MEMCACHED_UNKNOWN_READ_FAILURE, MEMCACHED_AT,
                                   memcached_literal_param("Numeric response was out of range"));
      }
      else if (errno == EINVAL)
      {
        result->numeric_value= UINT64_MAX;
        return memcached_set_error(*instance, MEMCACHED_UNKNOWN_READ_FAILURE, MEMCACHED_AT,
                                   memcached_literal_param("Numeric response was out of range"));
      }
      else if (errno != 0)
      {
        result->numeric_value= UINT64_MAX;
        return memcached_set_error(*instance, MEMCACHED_UNKNOWN_READ_FAILURE, MEMCACHED_AT,
                                   memcached_literal_param("Numeric response was out of range"));
      }

      result->numeric_value= uint64_t(auto_return_value);

      WATCHPOINT_STRING(buffer);
      return MEMCACHED_SUCCESS;
    }

  default:
    break;
  }

  buffer[total_read]= 0;
#if 0
  if (total_read >= sizeof("STORSTORED") -1)
  {
    fprintf(stderr, "%s:%d '%s', %.*s\n", __FILE__, __LINE__,
            buffer, MEMCACHED_MAX_BUFFER, instance->read_buffer);
    assert(memcmp(buffer,"STORSTORED", sizeof("STORSTORED") -1));
  }
#endif
  return memcached_set_error(*instance, MEMCACHED_UNKNOWN_READ_FAILURE, MEMCACHED_AT,
                             buffer, total_read);
}

static memcached_return_t binary_read_one_response(memcached_instance_st* instance,
                                                   char *buffer, const size_t buffer_length,
                                                   memcached_result_st *result)
{
  memcached_return_t rc;
  protocol_binary_response_header header;

  assert(memcached_is_binary(instance->root));

  if ((rc= memcached_safe_read(instance, &header.bytes, sizeof(header.bytes))) != MEMCACHED_SUCCESS)
  {
    WATCHPOINT_ERROR(rc);
    return rc;
  }

  if (header.response.magic != PROTOCOL_BINARY_RES)
  {
    return memcached_set_error(*instance, MEMCACHED_UNKNOWN_READ_FAILURE, MEMCACHED_AT);
  }

  /*
   ** Convert the header to host local endian!
 */
  header.response.keylen= ntohs(header.response.keylen);
  header.response.status= ntohs(header.response.status);
  header.response.bodylen= ntohl(header.response.bodylen);
  header.response.cas= memcached_ntohll(header.response.cas);
  uint32_t bodylen= header.response.bodylen;

  if (header.response.status == PROTOCOL_BINARY_RESPONSE_SUCCESS or
      header.response.status == PROTOCOL_BINARY_RESPONSE_AUTH_CONTINUE)
  {
    switch (header.response.opcode)
    {
    case PROTOCOL_BINARY_CMD_GETKQ:
      /*
       * We didn't increment the response counter for the GETKQ packet
       * (only the final NOOP), so we need to increment the counter again.
       */
      memcached_server_response_increment(instance);
      /* FALLTHROUGH */
    case PROTOCOL_BINARY_CMD_GETK:
      {
        uint16_t keylen= header.response.keylen;
        memcached_result_reset(result);
        result->item_cas= header.response.cas;

        if ((rc= memcached_safe_read(instance, &result->item_flags, sizeof (result->item_flags))) != MEMCACHED_SUCCESS)
        {
          WATCHPOINT_ERROR(rc);
          return MEMCACHED_UNKNOWN_READ_FAILURE;
        }

        result->item_flags= ntohl(result->item_flags);
        bodylen -= header.response.extlen;

        result->key_length= keylen;
        if (memcached_failed(rc= memcached_safe_read(instance, result->item_key, keylen)))
        {
          WATCHPOINT_ERROR(rc);
          return MEMCACHED_UNKNOWN_READ_FAILURE;
        }

        // Only bother with doing this if key_length > 0
        if (result->key_length)
        {
          if (memcached_array_size(instance->root->_namespace) and memcached_array_size(instance->root->_namespace) >= result->key_length)
          {
            return memcached_set_error(*instance, MEMCACHED_UNKNOWN_READ_FAILURE, MEMCACHED_AT);
          }

          if (memcached_array_size(instance->root->_namespace))
          {
            result->key_length-= memcached_array_size(instance->root->_namespace);
            memmove(result->item_key, result->item_key +memcached_array_size(instance->root->_namespace), result->key_length);
          }
        }

        bodylen -= keylen;
        if (memcached_failed(memcached_string_check(&result->value, bodylen)))
        {
          return MEMCACHED_MEMORY_ALLOCATION_FAILURE;
        }

        char *vptr= memcached_string_value_mutable(&result->value);
        if (memcached_failed(rc= memcached_safe_read(instance, vptr, bodylen)))
        {
          WATCHPOINT_ERROR(rc);
          return MEMCACHED_UNKNOWN_READ_FAILURE;
        }

        memcached_string_set_length(&result->value, bodylen);
      }
      break;

    case PROTOCOL_BINARY_CMD_INCREMENT:
    case PROTOCOL_BINARY_CMD_DECREMENT:
      {
        if (bodylen != sizeof(uint64_t))
        {
          result->numeric_value= UINT64_MAX;
          return memcached_set_error(*instance, MEMCACHED_UNKNOWN_READ_FAILURE, MEMCACHED_AT);
        }

        uint64_t val;
        if ((rc= memcached_safe_read(instance, &val, sizeof(val))) != MEMCACHED_SUCCESS)
        {
          result->numeric_value= UINT64_MAX;
          return MEMCACHED_UNKNOWN_READ_FAILURE;
        }

        result->numeric_value= memcached_ntohll(val);
      }
      break;

    case PROTOCOL_BINARY_CMD_SASL_LIST_MECHS:
      {
        if (header.response.keylen != 0 || bodylen + 1 > buffer_length)
        {
          return MEMCACHED_UNKNOWN_READ_FAILURE;
        }
        else
        {
          if ((rc= memcached_safe_read(instance, buffer, bodylen)) != MEMCACHED_SUCCESS)
          {
            return MEMCACHED_UNKNOWN_READ_FAILURE;
          }
        }
      }
      break;

    case PROTOCOL_BINARY_CMD_VERSION:
      {
        char version_buffer[32]; // @todo document this number
        memset(version_buffer, 0, sizeof(version_buffer));

        if (memcached_safe_read(instance, version_buffer, bodylen) != MEMCACHED_SUCCESS)
        {
          return MEMCACHED_UNKNOWN_READ_FAILURE;
        }

        char *endptr;
        errno= 0;
        long int version= strtol(version_buffer, &endptr, 10);
        if (errno != 0 or version == LONG_MIN or version == LONG_MAX or version > UINT8_MAX or version == 0)
        {
          instance->major_version= instance->minor_version= instance->micro_version= UINT8_MAX;
          return memcached_set_error(*instance, MEMCACHED_UNKNOWN_READ_FAILURE, MEMCACHED_AT, memcached_literal_param("strtol() failed to parse major version"));
        }
        instance->major_version= uint8_t(version);

        endptr++;
        errno= 0;
        version= strtol(endptr, &endptr, 10);
        if (errno != 0 or version == LONG_MIN or version == LONG_MAX or version > UINT8_MAX)
        {
          instance->major_version= instance->minor_version= instance->micro_version= UINT8_MAX;
          return memcached_set_error(*instance, MEMCACHED_UNKNOWN_READ_FAILURE, MEMCACHED_AT, memcached_literal_param("strtol() failed to parse minor version"));
        }
        instance->minor_version= uint8_t(version);

        endptr++;
        errno= 0;
        version= strtol(endptr, &endptr, 10);
        if (errno != 0 or version == LONG_MIN or version == LONG_MAX or version > UINT8_MAX)
        {
          instance->major_version= instance->minor_version= instance->micro_version= UINT8_MAX;
          return memcached_set_error(*instance, MEMCACHED_UNKNOWN_READ_FAILURE, MEMCACHED_AT, memcached_literal_param("strtol() failed to parse micro version"));
        }
        instance->micro_version= uint8_t(version);
      }
      break;

    case PROTOCOL_BINARY_CMD_TOUCH:
      {
        rc= MEMCACHED_SUCCESS;
        if (bodylen == 4) // The four byte read is a bug?
        {
          char touch_buffer[4]; // @todo document this number
          rc= memcached_safe_read(instance, touch_buffer, sizeof(touch_buffer));
#if 0
          fprintf(stderr, "%s:%d %d %d %d %d %.*s(%d)\n", __FILE__, __LINE__,
                  int(touch_buffer[0]),
                  int(touch_buffer[1]),
                  int(touch_buffer[2]),
                  int(touch_buffer[3]),
                  int(bodylen), touch_buffer, int(bodylen));
#endif
        }
        return memcached_set_error(*instance, rc, MEMCACHED_AT);
      }

    case PROTOCOL_BINARY_CMD_FLUSH:
    case PROTOCOL_BINARY_CMD_QUIT:
    case PROTOCOL_BINARY_CMD_SET:
    case PROTOCOL_BINARY_CMD_ADD:
    case PROTOCOL_BINARY_CMD_REPLACE:
    case PROTOCOL_BINARY_CMD_APPEND:
    case PROTOCOL_BINARY_CMD_PREPEND:
    case PROTOCOL_BINARY_CMD_DELETE:
      {
        WATCHPOINT_ASSERT(bodylen == 0);
        return MEMCACHED_SUCCESS;
      }

    case PROTOCOL_BINARY_CMD_NOOP:
      {
        WATCHPOINT_ASSERT(bodylen == 0);
        return MEMCACHED_END;
      }

    case PROTOCOL_BINARY_CMD_STAT:
      {
        if (bodylen == 0)
        {
          return MEMCACHED_END;
        }
        else if (bodylen + 1 > buffer_length)
        {
          /* not enough space in buffer.. should not happen... */
          return MEMCACHED_UNKNOWN_READ_FAILURE;
        }
        else
        {
          size_t keylen= header.response.keylen;
          memset(buffer, 0, buffer_length);
          if ((rc= memcached_safe_read(instance, buffer, keylen)) != MEMCACHED_SUCCESS ||
              (rc= memcached_safe_read(instance, buffer + keylen + 1, bodylen - keylen)) != MEMCACHED_SUCCESS)
          {
            WATCHPOINT_ERROR(rc);
            return MEMCACHED_UNKNOWN_READ_FAILURE;
          }
        }
      }
      break;

    case PROTOCOL_BINARY_CMD_SASL_AUTH:
    case PROTOCOL_BINARY_CMD_SASL_STEP:
      {
        memcached_result_reset(result);
        result->item_cas= header.response.cas;

        if (memcached_string_check(&result->value,
                                   bodylen) != MEMCACHED_SUCCESS)
          return MEMCACHED_MEMORY_ALLOCATION_FAILURE;

        char *vptr= memcached_string_value_mutable(&result->value);
        if ((rc= memcached_safe_read(instance, vptr, bodylen)) != MEMCACHED_SUCCESS)
        {
          WATCHPOINT_ERROR(rc);
          return MEMCACHED_UNKNOWN_READ_FAILURE;
        }

        memcached_string_set_length(&result->value, bodylen);
      }
      break;
    default:
      {
        /* Command not implemented yet! */
        return memcached_set_error(*instance, MEMCACHED_UNKNOWN_READ_FAILURE, MEMCACHED_AT);
      }
    }
  }
  else if (header.response.bodylen)
  {
    /* What should I do with the error message??? just discard it for now */
    char hole[SMALL_STRING_LEN];
    while (bodylen > 0)
    {
      size_t nr= (bodylen > SMALL_STRING_LEN) ? SMALL_STRING_LEN : bodylen;
      if ((rc= memcached_safe_read(instance, hole, nr)) != MEMCACHED_SUCCESS)
      {
        WATCHPOINT_ERROR(rc);
        return memcached_set_error(*instance, MEMCACHED_UNKNOWN_READ_FAILURE, MEMCACHED_AT);
      }
      bodylen-= (uint32_t) nr;
    }

    /* This might be an error from one of the quiet commands.. if
     * so, just throw it away and get the next one. What about creating
     * a callback to the user with the error information?
   */
    switch (header.response.opcode)
    {
    case PROTOCOL_BINARY_CMD_SETQ:
    case PROTOCOL_BINARY_CMD_ADDQ:
    case PROTOCOL_BINARY_CMD_REPLACEQ:
    case PROTOCOL_BINARY_CMD_APPENDQ:
    case PROTOCOL_BINARY_CMD_PREPENDQ:
      return binary_read_one_response(instance, buffer, buffer_length, result);

    default:
      break;
    }
  }

  rc= MEMCACHED_SUCCESS;
  if (header.response.status != 0)
  {
    switch (header.response.status)
    {
    case PROTOCOL_BINARY_RESPONSE_KEY_ENOENT:
      rc= MEMCACHED_NOTFOUND;
      break;

    case PROTOCOL_BINARY_RESPONSE_KEY_EEXISTS:
      rc= MEMCACHED_DATA_EXISTS;
      break;

    case PROTOCOL_BINARY_RESPONSE_NOT_STORED:
      rc= MEMCACHED_NOTSTORED;
      break;

    case PROTOCOL_BINARY_RESPONSE_E2BIG:
      rc= MEMCACHED_E2BIG;
      break;

    case PROTOCOL_BINARY_RESPONSE_ENOMEM:
      rc= MEMCACHED_MEMORY_ALLOCATION_FAILURE;
      break;

    case PROTOCOL_BINARY_RESPONSE_AUTH_CONTINUE:
      rc= MEMCACHED_AUTH_CONTINUE;
      break;

    case PROTOCOL_BINARY_RESPONSE_AUTH_ERROR:
      rc= MEMCACHED_AUTH_FAILURE;
      break;

    case PROTOCOL_BINARY_RESPONSE_EINVAL:
    case PROTOCOL_BINARY_RESPONSE_UNKNOWN_COMMAND:
    default:
      return memcached_set_error(*instance, MEMCACHED_UNKNOWN_READ_FAILURE, MEMCACHED_AT);
      break;
    }
  }

  return rc;
}

static memcached_return_t _read_one_response(memcached_instance_st* instance,
                                             char *buffer, const size_t buffer_length,
                                             memcached_result_st *result)
{
  memcached_server_response_decrement(instance);

  if (result == NULL)
  {
    Memcached *root= (Memcached *)instance->root;
    result = &root->result;
  }

  memcached_return_t rc;
  if (memcached_is_binary(instance->root))
  {
    rc= binary_read_one_response(instance, buffer, buffer_length, result);
  }
  else
  {
    rc= textual_read_one_response(instance, buffer, buffer_length, result);
  }

  if (memcached_fatal(rc))
  {
    memcached_io_reset(instance);
  }

  return rc;
}

memcached_return_t memcached_read_one_response(memcached_instance_st* instance,
                                               memcached_result_st *result)
{
  char buffer[SMALL_STRING_LEN];

  if (memcached_is_udp(instance->root))
  {
    return memcached_set_error(*instance, MEMCACHED_NOT_SUPPORTED, MEMCACHED_AT);
  }


  return _read_one_response(instance, buffer, sizeof(buffer), result);
}

memcached_return_t memcached_response(memcached_instance_st* instance,
                                      memcached_result_st *result)
{
  char buffer[1024];

  return memcached_response(instance, buffer, sizeof(buffer), result);
}

memcached_return_t memcached_response(memcached_instance_st* instance,
                                      char *buffer, size_t buffer_length,
                                      memcached_result_st *result)
{
  if (memcached_is_udp(instance->root))
  {
    return memcached_set_error(*instance, MEMCACHED_NOT_SUPPORTED, MEMCACHED_AT);
  }

  /* We may have old commands in the buffer not sent, first purge */
  if ((instance->root->flags.no_block) and (memcached_is_processing_input(instance->root) == false))
  {
    (void)memcached_io_write(instance);
  }

  /*  Before going into loop wait to see if we have any IO waiting for us */
  if (0)
  {
    memcached_return_t read_rc= memcached_io_wait_for_read(instance);
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, memcached_strerror(NULL, read_rc));
  }

  /*
   * The previous implementation purged all pending requests and just
   * returned the last one. Purge all pending messages to ensure backwards
   * compatibility.
 */
  if (memcached_is_binary(instance->root) == false and memcached_server_response_count(instance) > 1)
  {
    memcached_result_st junked_result;
    memcached_result_st *junked_result_ptr= memcached_result_create(instance->root, &junked_result);

    assert(junked_result_ptr);

    while (memcached_server_response_count(instance) > 1)
    {
      memcached_return_t rc= _read_one_response(instance, buffer, buffer_length, junked_result_ptr);

      // @TODO should we return an error on another but a bad read case?
      if (memcached_fatal(rc))
      {
        memcached_result_free(junked_result_ptr);
        return rc;
      }
    }
    memcached_result_free(junked_result_ptr);
  }

  return _read_one_response(instance, buffer, buffer_length, result);
}
