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

/* 
  I debated about putting this in the client library since it does an 
  action I don't really believe belongs in the library.

  Frankly its too damn useful not to be here though.
*/

#include <libmemcached/common.h>

memcached_server_list_st memcached_servers_parse(const char *server_strings)
{
  char *string;
  const char *begin_ptr;
  const char *end_ptr;
  memcached_server_st *servers= NULL;
  memcached_return_t rc;

  WATCHPOINT_ASSERT(server_strings);

  end_ptr= server_strings + strlen(server_strings);

  for (begin_ptr= server_strings, string= (char *)index(server_strings, ','); 
       begin_ptr != end_ptr; 
       string= (char *)index(begin_ptr, ','))
  {
    char buffer[HUGE_STRING_LEN];
    char *ptr, *ptr2;
    uint32_t weight= 0;

    if (string)
    {
      memcpy(buffer, begin_ptr, (size_t) (string - begin_ptr));
      buffer[(unsigned int)(string - begin_ptr)]= 0;
      begin_ptr= string+1;
    }
    else
    {
      size_t length= strlen(begin_ptr);
      memcpy(buffer, begin_ptr, length);
      buffer[length]= 0;
      begin_ptr= end_ptr;
    }

    ptr= index(buffer, ':');

    in_port_t port= 0;
    if (ptr)
    {
      ptr[0]= 0;

      ptr++;

      errno= 0;
      port= (in_port_t) strtoul(ptr, (char **)NULL, 10);
      if (errno != 0)
      {
        memcached_server_free(servers);
        return NULL;
      }

      ptr2= index(ptr, ' ');
      if (! ptr2)
        ptr2= index(ptr, ':');

      if (ptr2)
      {
        ptr2++;
        errno= 0;
        weight= uint32_t(strtoul(ptr2, (char **)NULL, 10));
        if (errno != 0)
        {
          memcached_server_free(servers);
          return NULL;
        }
      }
    }

    servers= memcached_server_list_append_with_weight(servers, buffer, port, weight, &rc);

    if (isspace(*begin_ptr))
    {
      begin_ptr++;
    }
  }

  return servers;
}
