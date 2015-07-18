/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  LibMemcached
 *
 *  Copyright (C) 2011 Data Differential, http://datadifferential.com/
 *  Copyright (C) 2006-2009 Brian Aker
 *  All rights reserved.
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

#define memcached_set_purging(__object, __value) ((__object)->state.is_purging= (__value))

class Purge
{
public:
  Purge(Memcached* arg) :
    _memc(arg)
  {
    memcached_set_purging(_memc, true);
  }

  ~Purge()
  {
    memcached_set_purging(_memc, false);
  }

private:
  Memcached* _memc;
};

class PollTimeout
{
public:
  PollTimeout(Memcached* arg) :
    _timeout(arg->poll_timeout),
    _origin(arg->poll_timeout)
  {
    _origin = 2000;
  }

  ~PollTimeout()
  {
    _origin= _timeout;
  }

private:
  int32_t _timeout;
  int32_t& _origin;
};

bool memcached_purge(memcached_instance_st* ptr)
{
  Memcached *root= (Memcached *)ptr->root;

  if (memcached_is_purging(ptr->root) || /* already purging */
      (memcached_server_response_count(ptr) < ptr->root->io_msg_watermark &&
       ptr->io_bytes_sent < ptr->root->io_bytes_watermark) ||
      (ptr->io_bytes_sent >= ptr->root->io_bytes_watermark &&
       memcached_server_response_count(ptr) < 2))
  {
    return true;
  }

  /*
    memcached_io_write and memcached_response may call memcached_purge
    so we need to be able stop any recursion.. 
  */
  Purge set_purge(root);

  WATCHPOINT_ASSERT(ptr->fd != INVALID_SOCKET);
  /* 
    Force a flush of the buffer to ensure that we don't have the n-1 pending
    requests buffered up.. 
  */
  if (memcached_io_write(ptr) == false)
  {
    memcached_set_error(*ptr, MEMCACHED_WRITE_FAILURE, MEMCACHED_AT);
    return false;
  }
  WATCHPOINT_ASSERT(ptr->fd != INVALID_SOCKET);

  bool is_successful= true;
  uint32_t no_msg= memcached_server_response_count(ptr) - 1;
  if (no_msg > 0)
  {
    memcached_result_st result;

    /*
     * We need to increase the timeout, because we might be waiting for
     * data to be sent from the server (the commands was in the output buffer
     * and just flushed
   */
    PollTimeout poll_timeout(ptr->root);

    memcached_result_st* result_ptr= memcached_result_create(root, &result);
    assert(result_ptr);

    for (uint32_t x= 0; x < no_msg; x++)
    {
      memcached_result_reset(result_ptr);
      memcached_return_t rc= memcached_read_one_response(ptr, result_ptr);
      /*
       * Purge doesn't care for what kind of command results that is received.
       * The only kind of errors I care about if is I'm out of sync with the
       * protocol or have problems reading data from the network..
     */
      if (rc== MEMCACHED_PROTOCOL_ERROR or rc == MEMCACHED_UNKNOWN_READ_FAILURE or rc == MEMCACHED_READ_FAILURE)
      {
        WATCHPOINT_ERROR(rc);
        memcached_io_reset(ptr);
        is_successful= false;
      }

      if (ptr->root->callbacks != NULL)
      {
        memcached_callback_st cb = *ptr->root->callbacks;
        if (memcached_success(rc))
        {
          for (uint32_t y= 0; y < cb.number_of_callback; y++)
          {
            if (memcached_fatal((*cb.callback[y])(ptr->root, result_ptr, cb.context)))
            {
              break;
            }
          }
        }
      }
    }

    memcached_result_free(result_ptr);
  }

  return is_successful;
}
