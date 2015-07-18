/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  LibMemcached
 *
 *  Copyright (C) 2011 Data Differential, http://datadifferential.com/
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

#include "libmemcached/assert.hpp"

#include <cerrno>
#include <cstdarg>
#include <cstdio>

#define MAX_ERROR_LENGTH 2048
struct memcached_error_t
{
  Memcached *root;
  uint64_t query_id;
  struct memcached_error_t *next;
  memcached_return_t rc;
  int local_errno;
  size_t size;
  char message[MAX_ERROR_LENGTH];
};

static void _set(memcached_instance_st& server, Memcached& memc)
{
  if (server.error_messages and server.error_messages->query_id != server.root->query_id)
  {
    memcached_error_free(server);
  }

  if (memc.error_messages)
  {
    if (memc.error_messages->rc == MEMCACHED_TIMEOUT)
    {
      server.io_wait_count.timeouts++;
    }

    memcached_error_t *error= libmemcached_xmalloc(&memc, memcached_error_t);
    if (error)
    {
      memcpy(error, memc.error_messages, sizeof(memcached_error_t));
      error->next= server.error_messages;
      server.error_messages= error;
    }
  }
}

#if 0
static int error_log_fd= -1;
#endif

static void _set(Memcached& memc, memcached_string_t *str, memcached_return_t &rc, const char *at, int local_errno= 0)
{
  if (memc.error_messages && memc.error_messages->query_id != memc.query_id)
  {
    memcached_error_free(memc);
  }

  if (memcached_fatal(rc) or rc == MEMCACHED_CLIENT_ERROR)
  {
    // For memory allocation we use our error since it is a bit more specific
    if (local_errno == ENOMEM and rc == MEMCACHED_ERRNO)
    {
      rc= MEMCACHED_MEMORY_ALLOCATION_FAILURE;
    }

    if (rc == MEMCACHED_MEMORY_ALLOCATION_FAILURE)
    {
      local_errno= ENOMEM;
    }

    if (rc == MEMCACHED_ERRNO and not local_errno)
    {
      local_errno= errno;
      rc= MEMCACHED_ERRNO;
    }

    if (rc == MEMCACHED_ERRNO and local_errno == ENOTCONN)
    {
      rc= MEMCACHED_CONNECTION_FAILURE;
    }

    if (rc == MEMCACHED_ERRNO and local_errno == ECONNRESET)
    {
      rc= MEMCACHED_CONNECTION_FAILURE;
    }

    if (local_errno == EINVAL)
    {
      rc= MEMCACHED_INVALID_ARGUMENTS;
    }

    if (local_errno == ECONNREFUSED)
    {
      rc= MEMCACHED_CONNECTION_FAILURE;
    }

    if (rc == MEMCACHED_TIMEOUT)
    {
    }

    memcached_error_t *error= libmemcached_xmalloc(&memc, memcached_error_t);
    if (error == NULL) // Bad business if this happens
    {
      assert_msg(error, "libmemcached_xmalloc() failed to allocate a memcached_error_t");
      return;
    }

    error->root= &memc;
    error->query_id= memc.query_id;
    error->rc= rc;
    error->local_errno= local_errno;

    // MEMCACHED_CLIENT_ERROR is a special case because it is an error coming from the server
    if (rc == MEMCACHED_CLIENT_ERROR)
    {
      assert(str);
      assert(str->size);
      if (str and str->size)
      {
        assert(error->local_errno == 0);
        error->local_errno= 0;

        error->size= (int)snprintf(error->message, MAX_ERROR_LENGTH, "(%p) %.*s", 
                                   error->root,
                                   int(str->size), str->c_str);
      }
    }
    else if (local_errno)
    {
      const char *errmsg_ptr;
      char errmsg[MAX_ERROR_LENGTH];
      errmsg[0]= 0;
      errmsg_ptr= errmsg;

#if defined(STRERROR_R_CHAR_P) && STRERROR_R_CHAR_P
      errmsg_ptr= strerror_r(local_errno, errmsg, sizeof(errmsg));
#elif defined(HAVE_STRERROR_R) && HAVE_STRERROR_R
      strerror_r(local_errno, errmsg, sizeof(errmsg));
      errmsg_ptr= errmsg;
#elif defined(HAVE_STRERROR) && HAVE_STRERROR
      snprintf(errmsg, sizeof(errmsg), "%s", strerror(local_errno));
      errmsg_ptr= errmsg;
#endif

      if (str and str->size and local_errno)
      {
        error->size= (int)snprintf(error->message, MAX_ERROR_LENGTH, "(%p) %s(%s), %.*s -> %s", 
                                   error->root,
                                   memcached_strerror(&memc, rc), 
                                   errmsg_ptr,
                                   memcached_string_printf(*str), at);
      }
      else
      {
        error->size= (int)snprintf(error->message, MAX_ERROR_LENGTH, "(%p) %s(%s) -> %s", 
                                   error->root,
                                   memcached_strerror(&memc, rc), 
                                   errmsg_ptr,
                                   at);
      }
    }
    else if (rc == MEMCACHED_PARSE_ERROR and str and str->size)
    {
      error->size= (int)snprintf(error->message, MAX_ERROR_LENGTH, "(%p) %.*s -> %s", 
                                 error->root,
                                 int(str->size), str->c_str, at);
    }
    else if (str and str->size)
    {
      error->size= (int)snprintf(error->message, MAX_ERROR_LENGTH, "(%p) %s, %.*s -> %s", 
                                 error->root,
                                 memcached_strerror(&memc, rc), 
                                 int(str->size), str->c_str, at);
    }
    else
    {
      error->size= (int)snprintf(error->message, MAX_ERROR_LENGTH, "(%p) %s -> %s", 
                                 error->root,
                                 memcached_strerror(&memc, rc), at);
    }

    error->next= memc.error_messages;
    memc.error_messages= error;
  }

#if 0
  if (error_log_fd == -1)
  {
//    unlink("/tmp/libmemcachd.log");
    if ((error_log_fd= open("/tmp/libmemcachd.log", O_CREAT | O_WRONLY | O_APPEND, 0644)) < 0)
    {
      perror("open");
      error_log_fd= -1;
    }
  }
  ::write(error_log_fd, error->message, error->size);
  ::write(error_log_fd, "\n", 1);
#endif
}

memcached_return_t memcached_set_error(Memcached& memc, memcached_return_t rc, const char *at, const char *str, size_t length)
{
  assert_msg(rc != MEMCACHED_ERRNO, "Programmer error, MEMCACHED_ERRNO was set to be returned to client");
  memcached_string_t tmp= { str, length };
  return memcached_set_error(memc, rc, at, tmp);
}

memcached_return_t memcached_set_error(memcached_instance_st& self, memcached_return_t rc, const char *at, const char *str, size_t length)
{
  assert_msg(rc != MEMCACHED_ERRNO, "Programmer error, MEMCACHED_ERRNO was set to be returned to client");
  assert_msg(rc != MEMCACHED_SOME_ERRORS, "Programmer error, MEMCACHED_SOME_ERRORS was about to be set on a Instance");

  memcached_string_t tmp= { str, length };
  return memcached_set_error(self, rc, at, tmp);
}

#ifndef __INTEL_COMPILER
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#endif

memcached_return_t memcached_set_error(Memcached& memc, memcached_return_t rc, const char *at, memcached_string_t& str)
{
  assert_msg(rc != MEMCACHED_ERRNO, "Programmer error, MEMCACHED_ERRNO was set to be returned to client");
  if (memcached_fatal(rc))
  {
    _set(memc, &str, rc, at);
  }

  return rc;
}

memcached_return_t memcached_set_parser_error(Memcached& memc,
                                              const char *at,
                                              const char *format, ...)
{
  va_list args;

  char buffer[BUFSIZ];
  va_start(args, format);
  int length= vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  return memcached_set_error(memc, MEMCACHED_PARSE_ERROR, at, buffer, length);
}

static inline size_t append_host_to_string(memcached_instance_st& self, char* buffer, const size_t buffer_length)
{
  size_t size= 0;
  switch (self.type)
  {
  case MEMCACHED_CONNECTION_TCP:
  case MEMCACHED_CONNECTION_UDP:
    size+= snprintf(buffer, buffer_length, " host: %s:%d",
                    self.hostname(), int(self.port()));
    break;

  case MEMCACHED_CONNECTION_UNIX_SOCKET:
    size+= snprintf(buffer, buffer_length, " socket: %s",
                    self.hostname());
    break;
  }

  return size;
}

memcached_return_t memcached_set_error(memcached_instance_st& self, memcached_return_t rc, const char *at, memcached_string_t& str)
{
  assert_msg(rc != MEMCACHED_ERRNO, "Programmer error, MEMCACHED_ERRNO was set to be returned to client");
  assert_msg(rc != MEMCACHED_SOME_ERRORS, "Programmer error, MEMCACHED_SOME_ERRORS was about to be set on a memcached_instance_st");
  if (memcached_fatal(rc) == false and rc != MEMCACHED_CLIENT_ERROR)
  {
    return rc;
  }

  char hostname_port_message[MAX_ERROR_LENGTH];
  char* hostname_port_message_ptr= hostname_port_message;
  int size= 0;
  if (str.size)
  {
    size= snprintf(hostname_port_message_ptr, sizeof(hostname_port_message), "%.*s, ",
                   memcached_string_printf(str));
    hostname_port_message_ptr+= size;
  }

  size+= append_host_to_string(self, hostname_port_message_ptr, sizeof(hostname_port_message) -size);

  memcached_string_t error_host= { hostname_port_message, size_t(size) };

  assert_msg(self.root, "Programmer error, root was not set on instance");
  if (self.root)
  {
    _set(*self.root, &error_host, rc, at);
    _set(self, (*self.root));
    assert(self.error_messages);
    assert(self.root->error_messages);
    assert(self.error_messages->rc == self.root->error_messages->rc);
  }

  return rc;
}

memcached_return_t memcached_set_error(memcached_instance_st& self, memcached_return_t rc, const char *at)
{
  assert_msg(rc != MEMCACHED_SOME_ERRORS, "Programmer error, MEMCACHED_SOME_ERRORS was about to be set on a memcached_instance_st");
  if (memcached_fatal(rc) == false)
  {
    return rc;
  }

  char hostname_port[MEMCACHED_NI_MAXHOST +MEMCACHED_NI_MAXSERV + sizeof("host : ")];
  size_t size= append_host_to_string(self, hostname_port, sizeof(hostname_port));

  memcached_string_t error_host= { hostname_port, size};

  if (self.root)
  {
    _set(*self.root, &error_host, rc, at);
    _set(self, *self.root);
  }

  return rc;
}

memcached_return_t memcached_set_error(Memcached& self, memcached_return_t rc, const char *at)
{
  assert_msg(rc != MEMCACHED_ERRNO, "Programmer error, MEMCACHED_ERRNO was set to be returned to client");
  if (memcached_fatal(rc) == false)
  {
    return rc;
  }

  _set(self, NULL, rc, at);

  return rc;
}

memcached_return_t memcached_set_errno(Memcached& self, int local_errno, const char *at, const char *str, size_t length)
{
  memcached_string_t tmp= { str, length };
  return memcached_set_errno(self, local_errno, at, tmp);
}

memcached_return_t memcached_set_errno(memcached_instance_st& self, int local_errno, const char *at, const char *str, size_t length)
{
  memcached_string_t tmp= { str, length };
  return memcached_set_errno(self, local_errno, at, tmp);
}

memcached_return_t memcached_set_errno(Memcached& self, int local_errno, const char *at)
{
  if (local_errno == 0)
  {
    return MEMCACHED_SUCCESS;
  }

  memcached_return_t rc= MEMCACHED_ERRNO;
  _set(self, NULL, rc, at, local_errno);

  return rc;
}

memcached_return_t memcached_set_errno(Memcached& memc, int local_errno, const char *at, memcached_string_t& str)
{
  if (local_errno == 0)
  {
    return MEMCACHED_SUCCESS;
  }

  memcached_return_t rc= MEMCACHED_ERRNO;
  _set(memc, &str, rc, at, local_errno);

  return rc;
}

memcached_return_t memcached_set_errno(memcached_instance_st& self, int local_errno, const char *at, memcached_string_t& str)
{
  if (local_errno == 0)
  {
    return MEMCACHED_SUCCESS;
  }

  char hostname_port_message[MAX_ERROR_LENGTH];
  char* hostname_port_message_ptr= hostname_port_message;
  size_t size= 0;
  if (str.size)
  {
    size= snprintf(hostname_port_message_ptr, sizeof(hostname_port_message), "%.*s, ", memcached_string_printf(str));
  }
  size+= append_host_to_string(self, hostname_port_message_ptr, sizeof(hostname_port_message) -size);

  memcached_string_t error_host= { hostname_port_message, size };

  memcached_return_t rc= MEMCACHED_ERRNO;
  if (self.root == NULL)
  {
    return rc;
  }

  _set(*self.root, &error_host, rc, at, local_errno);
  _set(self, (*self.root));

#if 0
  if (self.root->error_messages->rc != self.error_messages->rc)
  {
    fprintf(stderr, "%s:%d %s != %s\n", __FILE__, __LINE__,
            memcached_strerror(NULL, self.root->error_messages->rc),
            memcached_strerror(NULL, self.error_messages->rc));
  }
#endif

  return rc;
}

memcached_return_t memcached_set_errno(memcached_instance_st& self, int local_errno, const char *at)
{
  if (local_errno == 0)
  {
    return MEMCACHED_SUCCESS;
  }

  char hostname_port_message[MAX_ERROR_LENGTH];
  size_t size= append_host_to_string(self, hostname_port_message, sizeof(hostname_port_message));

  memcached_string_t error_host= { hostname_port_message, size };

  memcached_return_t rc= MEMCACHED_ERRNO;
  if (self.root == NULL)
  {
    return rc;
  }

  _set(*self.root, &error_host, rc, at, local_errno);
  _set(self, (*self.root));

  return rc;
}

static void _error_print(const memcached_error_t *error)
{
  if (error == NULL)
  {
    return;
  }

  if (error->size == 0)
  {
    fprintf(stderr, "\t%s\n", memcached_strerror(NULL, error->rc) );
  }
  else
  {
    fprintf(stderr, "\t%s %s\n", memcached_strerror(NULL, error->rc), error->message);
  }

  _error_print(error->next);
}

void memcached_error_print(const Memcached *shell)
{
  const Memcached* self= memcached2Memcached(shell);
  if (self == NULL)
  {
    return;
  }

  _error_print(self->error_messages);

  for (uint32_t x= 0; x < memcached_server_count(self); x++)
  {
    memcached_instance_st* instance= memcached_instance_by_position(self, x);

    _error_print(instance->error_messages);
  }
}

static void _error_free(memcached_error_t *error)
{
  if (error)
  {
    _error_free(error->next);

    libmemcached_free(error->root, error);
  }
}

void memcached_error_free(Memcached& self)
{
  _error_free(self.error_messages);
  self.error_messages= NULL;
}

void memcached_error_free(memcached_instance_st& self)
{
  _error_free(self.error_messages);
  self.error_messages= NULL;
}

void memcached_error_free(memcached_server_st& self)
{
  _error_free(self.error_messages);
  self.error_messages= NULL;
}

const char *memcached_error(const memcached_st *memc)
{
  return memcached_last_error_message(memc);
}

const char *memcached_last_error_message(const memcached_st *shell)
{
  const Memcached* memc= memcached2Memcached(shell);
  if (memc)
  {
    if (memc->error_messages)
    {
      if (memc->error_messages->size and memc->error_messages->message[0])
      {
        return memc->error_messages->message;
      }

      return memcached_strerror(memc, memc->error_messages->rc);
    }

    return memcached_strerror(memc, MEMCACHED_SUCCESS);
  }

  return memcached_strerror(memc, MEMCACHED_INVALID_ARGUMENTS);
}

bool memcached_has_current_error(Memcached &memc)
{
  if (memc.error_messages 
      and memc.error_messages->query_id == memc.query_id
      and memcached_failed(memc.error_messages->rc))
  {
    return true;
  }

  return false;
}

bool memcached_has_current_error(memcached_instance_st& server)
{
  return memcached_has_current_error(*(server.root));
}

memcached_return_t memcached_last_error(const memcached_st *shell)
{
  const Memcached* memc= memcached2Memcached(shell);
  if (memc)
  {
    if (memc->error_messages)
    {
      return memc->error_messages->rc;
    }

    return MEMCACHED_SUCCESS;
  }

  return MEMCACHED_INVALID_ARGUMENTS;
}

int memcached_last_error_errno(const memcached_st *shell)
{
  const Memcached* memc= memcached2Memcached(shell);
  if (memc == NULL)
  {
    return 0;
  }

  if (memc->error_messages == NULL)
  {
    return 0;
  }

  return memc->error_messages->local_errno;
}

const char *memcached_server_error(const memcached_instance_st * server)
{
  if (server == NULL)
  {
    return NULL;
  }

  if (server->error_messages == NULL)
  {
    return memcached_strerror(server->root, MEMCACHED_SUCCESS);
  }

  if (server->error_messages->size == 0)
  {
    return memcached_strerror(server->root, server->error_messages->rc);
  }

  return server->error_messages->message;
}


memcached_error_t *memcached_error_copy(const memcached_instance_st& server)
{
  if (server.error_messages == NULL)
  {
    return NULL;
  }

  memcached_error_t *error= libmemcached_xmalloc(server.root, memcached_error_t);
  memcpy(error, server.error_messages, sizeof(memcached_error_t));
  error->next= NULL;

  return error;
}

memcached_return_t memcached_server_error_return(const memcached_instance_st * ptr)
{
  if (ptr == NULL)
  {
    return MEMCACHED_INVALID_ARGUMENTS;
  }

  if (ptr and ptr->error_messages)
  {
    return ptr->error_messages->rc;
  }

  return MEMCACHED_SUCCESS;
}

memcached_return_t memcached_instance_error_return(memcached_instance_st* instance)
{
  if (instance == NULL)
  {
    return MEMCACHED_INVALID_ARGUMENTS;
  }

  if (instance and instance->error_messages)
  {
    return instance->error_messages->rc;
  }

  return MEMCACHED_SUCCESS;
}
