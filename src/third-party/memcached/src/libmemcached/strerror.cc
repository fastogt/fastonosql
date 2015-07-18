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

const char *memcached_strerror(const memcached_st *, memcached_return_t rc)
{
  switch (rc)
  {
  case MEMCACHED_SUCCESS:
    return "SUCCESS";

  case MEMCACHED_FAILURE:
    return "FAILURE";

  case MEMCACHED_HOST_LOOKUP_FAILURE: // getaddrinfo only
    return "getaddrinfo() or getnameinfo() HOSTNAME LOOKUP FAILURE";

  case MEMCACHED_CONNECTION_FAILURE:
    return "CONNECTION FAILURE";

  case MEMCACHED_CONNECTION_BIND_FAILURE: // DEPRECATED, see MEMCACHED_HOST_LOOKUP_FAILURE
    return "CONNECTION BIND FAILURE";

  case MEMCACHED_READ_FAILURE:
    return "READ FAILURE";

  case MEMCACHED_UNKNOWN_READ_FAILURE:
    return "UNKNOWN READ FAILURE";

  case MEMCACHED_PROTOCOL_ERROR:
    return "PROTOCOL ERROR";

  case MEMCACHED_CLIENT_ERROR:
    return "CLIENT ERROR";

  case MEMCACHED_SERVER_ERROR:
    return "SERVER ERROR";

  case MEMCACHED_WRITE_FAILURE:
    return "WRITE FAILURE";

  case MEMCACHED_ERROR:
    return "ERROR was returned by server";

  case MEMCACHED_DATA_EXISTS:
    return "CONNECTION DATA EXISTS";

  case MEMCACHED_DATA_DOES_NOT_EXIST:
    return "CONNECTION DATA DOES NOT EXIST";

  case MEMCACHED_NOTSTORED:
    return "NOT STORED";

  case MEMCACHED_STORED:
    return "STORED";

  case MEMCACHED_NOTFOUND:
    return "NOT FOUND";

  case MEMCACHED_MEMORY_ALLOCATION_FAILURE:
    return "MEMORY ALLOCATION FAILURE";

  case MEMCACHED_PARTIAL_READ:
    return "PARTIAL READ";

  case MEMCACHED_SOME_ERRORS:
    return "SOME ERRORS WERE REPORTED";

  case MEMCACHED_NO_SERVERS:
    return "NO SERVERS DEFINED";

  case MEMCACHED_END:
    return "SERVER END";

  case MEMCACHED_DELETED:
    return "SERVER DELETE";

  case MEMCACHED_VALUE:
    return "SERVER VALUE";

  case MEMCACHED_STAT:
    return "STAT VALUE";

  case MEMCACHED_ITEM:
    return "ITEM VALUE";

  case MEMCACHED_ERRNO:
    return "SYSTEM ERROR";

  case MEMCACHED_FAIL_UNIX_SOCKET:
    return "COULD NOT OPEN UNIX SOCKET";

  case MEMCACHED_NOT_SUPPORTED:
    return "ACTION NOT SUPPORTED";

  case MEMCACHED_FETCH_NOTFINISHED:
    return "FETCH WAS NOT COMPLETED";

  case MEMCACHED_NO_KEY_PROVIDED:
    return "A KEY LENGTH OF ZERO WAS PROVIDED";

  case MEMCACHED_BUFFERED:
    return "ACTION QUEUED";

  case MEMCACHED_TIMEOUT:
    return "A TIMEOUT OCCURRED";

  case MEMCACHED_BAD_KEY_PROVIDED:
    return "A BAD KEY WAS PROVIDED/CHARACTERS OUT OF RANGE";

  case MEMCACHED_INVALID_HOST_PROTOCOL:
    return "THE HOST TRANSPORT PROTOCOL DOES NOT MATCH THAT OF THE CLIENT";

  case MEMCACHED_SERVER_MARKED_DEAD:
    return "SERVER IS MARKED DEAD";

  case MEMCACHED_UNKNOWN_STAT_KEY:
    return "ENCOUNTERED AN UNKNOWN STAT KEY";

  case MEMCACHED_E2BIG:
    return "ITEM TOO BIG";

  case MEMCACHED_INVALID_ARGUMENTS:
     return "INVALID ARGUMENTS";

  case MEMCACHED_KEY_TOO_BIG:
     return "KEY RETURNED FROM SERVER WAS TOO LARGE";

  case MEMCACHED_AUTH_PROBLEM:
    return "FAILED TO SEND AUTHENTICATION TO SERVER";

  case MEMCACHED_AUTH_FAILURE:
    return "AUTHENTICATION FAILURE";

  case MEMCACHED_AUTH_CONTINUE:
    return "CONTINUE AUTHENTICATION";

  case MEMCACHED_PARSE_ERROR:
    return "ERROR OCCURED WHILE PARSING";

  case MEMCACHED_PARSE_USER_ERROR:
    return "USER INITIATED ERROR OCCURED WHILE PARSING";

  case MEMCACHED_DEPRECATED:
    return "DEPRECATED";

  case MEMCACHED_IN_PROGRESS:
    return "OPERATION IN PROCESS";

  case MEMCACHED_SERVER_TEMPORARILY_DISABLED:
    return "SERVER HAS FAILED AND IS DISABLED UNTIL TIMED RETRY";

  case MEMCACHED_SERVER_MEMORY_ALLOCATION_FAILURE:
    return "SERVER FAILED TO ALLOCATE OBJECT";

  default:
  case MEMCACHED_MAXIMUM_RETURN:
    return "INVALID memcached_return_t";
  }
}
