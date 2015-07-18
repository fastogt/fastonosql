/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Libmemcached library
 *
 *  Copyright (C) 2011-2012 Data Differential, http://datadifferential.com/
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

#include "libmemcached/common.h"
#include <cassert>

#if defined(LIBMEMCACHED_WITH_SASL_SUPPORT) && LIBMEMCACHED_WITH_SASL_SUPPORT

#if defined(HAVE_LIBSASL) && HAVE_LIBSASL
#include <sasl/sasl.h>
#endif

#include <pthread.h>

void memcached_set_sasl_callbacks(memcached_st *shell,
                                  const sasl_callback_t *callbacks)
{
  Memcached* self= memcached2Memcached(shell);
  if (self)
  {
    self->sasl.callbacks= const_cast<sasl_callback_t *>(callbacks);
    self->sasl.is_allocated= false;
  }
}

sasl_callback_t *memcached_get_sasl_callbacks(memcached_st *shell)
{
  Memcached* self= memcached2Memcached(shell);
  if (self)
  {
    return self->sasl.callbacks;
  }

  return NULL;
}

/**
 * Resolve the names for both ends of a connection
 * @param fd socket to check
 * @param laddr local address (out)
 * @param raddr remote address (out)
 * @return true on success false otherwise (errno contains more info)
 */
static memcached_return_t resolve_names(memcached_instance_st& server, char *laddr, size_t laddr_length, char *raddr, size_t raddr_length)
{
  char host[MEMCACHED_NI_MAXHOST];
  char port[MEMCACHED_NI_MAXSERV];
  struct sockaddr_storage saddr;
  socklen_t salen= sizeof(saddr);

  if (getsockname(server.fd, (struct sockaddr *)&saddr, &salen) < 0)
  {
    return memcached_set_error(server, MEMCACHED_HOST_LOOKUP_FAILURE, MEMCACHED_AT);
  }

  if (getnameinfo((struct sockaddr *)&saddr, salen, host, sizeof(host), port, sizeof(port), NI_NUMERICHOST | NI_NUMERICSERV) < 0)
  {
    return memcached_set_error(server, MEMCACHED_HOST_LOOKUP_FAILURE, MEMCACHED_AT);
  }

  (void)snprintf(laddr, laddr_length, "%s;%s", host, port);
  salen= sizeof(saddr);

  if (getpeername(server.fd, (struct sockaddr *)&saddr, &salen) < 0)
  {
    return memcached_set_error(server, MEMCACHED_HOST_LOOKUP_FAILURE, MEMCACHED_AT);
  }

  if (getnameinfo((struct sockaddr *)&saddr, salen, host, sizeof(host),
                   port, sizeof(port), NI_NUMERICHOST | NI_NUMERICSERV) < 0)
  {
    return memcached_set_error(server, MEMCACHED_HOST_LOOKUP_FAILURE, MEMCACHED_AT);
  }

  (void)snprintf(raddr, raddr_length, "%s;%s", host, port);

  return MEMCACHED_SUCCESS;
}

extern "C" {

static void sasl_shutdown_function()
{
  sasl_done();
}

static volatile int sasl_startup_state= SASL_OK;
pthread_mutex_t sasl_startup_state_LOCK= PTHREAD_MUTEX_INITIALIZER;
static pthread_once_t sasl_startup_once= PTHREAD_ONCE_INIT;
static void sasl_startup_function(void)
{
  sasl_startup_state= sasl_client_init(NULL);

  if (sasl_startup_state == SASL_OK)
  {
    (void)atexit(sasl_shutdown_function);
  }
}

} // extern "C"

memcached_return_t memcached_sasl_authenticate_connection(memcached_instance_st* server)
{
  if (LIBMEMCACHED_WITH_SASL_SUPPORT == 0)
  {
    return MEMCACHED_NOT_SUPPORTED;
  }

  if (server == NULL)
  {
    return MEMCACHED_INVALID_ARGUMENTS;
  }

  /* SANITY CHECK: SASL can only be used with the binary protocol */
  if (memcached_is_binary(server->root) == false)
  {
    return  memcached_set_error(*server, MEMCACHED_INVALID_ARGUMENTS, MEMCACHED_AT,
                                memcached_literal_param("memcached_sasl_authenticate_connection() is not supported via the ASCII protocol"));
  }

  /* Try to get the supported mech from the server. Servers without SASL
   * support will return UNKNOWN COMMAND, so we can just treat that
   * as authenticated
 */
  protocol_binary_request_no_extras request= { };

  initialize_binary_request(server, request.message.header);

  request.message.header.request.opcode= PROTOCOL_BINARY_CMD_SASL_LIST_MECHS;

  if (memcached_io_write(server, request.bytes, sizeof(request.bytes), true) != sizeof(request.bytes))
  {
    return MEMCACHED_WRITE_FAILURE;
  }
  assert_msg(server->fd != INVALID_SOCKET, "Programmer error, invalid socket");

  memcached_server_response_increment(server);

  char mech[MEMCACHED_MAX_BUFFER];
  memcached_return_t rc= memcached_response(server, mech, sizeof(mech), NULL);
  if (memcached_failed(rc))
  {
    if (rc == MEMCACHED_PROTOCOL_ERROR)
    {
      /* If the server doesn't support SASL it will return PROTOCOL_ERROR.
       * This error may also be returned for other errors, but let's assume
       * that the server don't support SASL and treat it as success and
       * let the client fail with the next operation if the error was
       * caused by another problem....
     */
      rc= MEMCACHED_SUCCESS;
    }

    return rc;
  }
  assert_msg(server->fd != INVALID_SOCKET, "Programmer error, invalid socket");

  /* set ip addresses */
  char laddr[MEMCACHED_NI_MAXHOST + MEMCACHED_NI_MAXSERV];
  char raddr[MEMCACHED_NI_MAXHOST + MEMCACHED_NI_MAXSERV];

  if (memcached_failed(rc= resolve_names(*server, laddr, sizeof(laddr), raddr, sizeof(raddr))))
  {
    return rc;
  }

  int pthread_error;
  if ((pthread_error= pthread_once(&sasl_startup_once, sasl_startup_function)) != 0)
  {
    return memcached_set_errno(*server, pthread_error, MEMCACHED_AT);
  }

  (void)pthread_mutex_lock(&sasl_startup_state_LOCK);
  if (sasl_startup_state != SASL_OK)
  {
    const char *sasl_error_msg= sasl_errstring(sasl_startup_state, NULL, NULL);
    return memcached_set_error(*server, MEMCACHED_AUTH_PROBLEM, MEMCACHED_AT, 
                               memcached_string_make_from_cstr(sasl_error_msg));
  }
  (void)pthread_mutex_unlock(&sasl_startup_state_LOCK);

  sasl_conn_t *conn;
  int ret;
  if ((ret= sasl_client_new("memcached", server->_hostname, laddr, raddr, server->root->sasl.callbacks, 0, &conn) ) != SASL_OK)
  {
    const char *sasl_error_msg= sasl_errstring(ret, NULL, NULL);

    sasl_dispose(&conn);

    return memcached_set_error(*server, MEMCACHED_AUTH_PROBLEM, MEMCACHED_AT, 
                               memcached_string_make_from_cstr(sasl_error_msg));
  }

  const char *data;
  const char *chosenmech;
  unsigned int len;
  ret= sasl_client_start(conn, mech, NULL, &data, &len, &chosenmech);
  if (ret != SASL_OK and ret != SASL_CONTINUE)
  {
    const char *sasl_error_msg= sasl_errstring(ret, NULL, NULL);

    sasl_dispose(&conn);

    return memcached_set_error(*server, MEMCACHED_AUTH_PROBLEM, MEMCACHED_AT, 
                               memcached_string_make_from_cstr(sasl_error_msg));
  }
  uint16_t keylen= (uint16_t)strlen(chosenmech);
  request.message.header.request.opcode= PROTOCOL_BINARY_CMD_SASL_AUTH;
  request.message.header.request.keylen= htons(keylen);
  request.message.header.request.bodylen= htonl(len + keylen);

  do {
    /* send the packet */

    libmemcached_io_vector_st vector[]=
    {
      { request.bytes, sizeof(request.bytes) },
      { chosenmech, keylen },
      { data, len }
    };

    assert_msg(server->fd != INVALID_SOCKET, "Programmer error, invalid socket");
    if (memcached_io_writev(server, vector, 3, true) == false)
    {
      rc= MEMCACHED_WRITE_FAILURE;
      break;
    }
    assert_msg(server->fd != INVALID_SOCKET, "Programmer error, invalid socket");
    memcached_server_response_increment(server);

    /* read the response */
    assert_msg(server->fd != INVALID_SOCKET, "Programmer error, invalid socket");
    rc= memcached_response(server, NULL, 0, NULL);
    if (rc != MEMCACHED_AUTH_CONTINUE)
    {
      break;
    }
    assert_msg(server->fd != INVALID_SOCKET, "Programmer error, invalid socket");

    ret= sasl_client_step(conn, memcached_result_value(&server->root->result),
                          (unsigned int)memcached_result_length(&server->root->result),
                          NULL, &data, &len);

    if (ret != SASL_OK && ret != SASL_CONTINUE)
    {
      rc= MEMCACHED_AUTH_PROBLEM;
      break;
    }

    request.message.header.request.opcode= PROTOCOL_BINARY_CMD_SASL_STEP;
    request.message.header.request.bodylen= htonl(len + keylen);
  } while (true);

  /* Release resources */
  sasl_dispose(&conn);

  return memcached_set_error(*server, rc, MEMCACHED_AT);
}

static int get_username(void *context, int id, const char **result, unsigned int *len)
{
  if (!context || !result || (id != SASL_CB_USER && id != SASL_CB_AUTHNAME))
  {
    return SASL_BADPARAM;
  }

  *result= (char *)context;
  if (len)
  {
    *len= (unsigned int)strlen(*result);
  }

  return SASL_OK;
}

static int get_password(sasl_conn_t *conn, void *context, int id,
                        sasl_secret_t **psecret)
{
  if (!conn || ! psecret || id != SASL_CB_PASS)
  {
    return SASL_BADPARAM;
  }

  *psecret= (sasl_secret_t *)context;

  return SASL_OK;
}

memcached_return_t memcached_set_sasl_auth_data(memcached_st *shell,
                                                const char *username,
                                                const char *password)
{
  Memcached* ptr= memcached2Memcached(shell);
  if (LIBMEMCACHED_WITH_SASL_SUPPORT == 0)
  {
    return MEMCACHED_NOT_SUPPORTED;
  }

  if (ptr == NULL or username == NULL or password == NULL)
  {
    return MEMCACHED_INVALID_ARGUMENTS;
  }

  memcached_return_t ret;
  if (memcached_failed(ret= memcached_behavior_set(ptr, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL, 1)))
  {
    return memcached_set_error(*ptr, ret, MEMCACHED_AT, memcached_literal_param("Unable change to binary protocol which is required for SASL."));
  }

  memcached_destroy_sasl_auth_data(ptr);

  sasl_callback_t *callbacks= libmemcached_xcalloc(ptr, 4, sasl_callback_t);
  size_t password_length= strlen(password);
  size_t username_length= strlen(username);
  char *name= (char *)libmemcached_malloc(ptr, username_length +1);
  sasl_secret_t *secret= (sasl_secret_t*)libmemcached_malloc(ptr, password_length +1 + sizeof(sasl_secret_t));

  if (callbacks == NULL or name == NULL or secret == NULL)
  {
    libmemcached_free(ptr, callbacks);
    libmemcached_free(ptr, name);
    libmemcached_free(ptr, secret);
    return memcached_set_error(*ptr, MEMCACHED_MEMORY_ALLOCATION_FAILURE, MEMCACHED_AT);
  }

  secret->len= password_length;
  memcpy(secret->data, password, password_length);
  secret->data[password_length]= 0;

  callbacks[0].id= SASL_CB_USER;
  callbacks[0].proc= (int (*)())get_username;
  callbacks[0].context= strncpy(name, username, username_length +1);
  callbacks[1].id= SASL_CB_AUTHNAME;
  callbacks[1].proc= (int (*)())get_username;
  callbacks[1].context= name;
  callbacks[2].id= SASL_CB_PASS;
  callbacks[2].proc= (int (*)())get_password;
  callbacks[2].context= secret;
  callbacks[3].id= SASL_CB_LIST_END;

  ptr->sasl.callbacks= callbacks;
  ptr->sasl.is_allocated= true;

  return MEMCACHED_SUCCESS;
}

memcached_return_t memcached_destroy_sasl_auth_data(memcached_st *shell)
{
  if (LIBMEMCACHED_WITH_SASL_SUPPORT == 0)
  {
    return MEMCACHED_NOT_SUPPORTED;
  }

  Memcached* ptr= memcached2Memcached(shell);
  if (ptr == NULL)
  {
    return MEMCACHED_INVALID_ARGUMENTS;
  }

  if (ptr->sasl.callbacks == NULL)
  {
    return MEMCACHED_SUCCESS;
  }

  if (ptr->sasl.is_allocated)
  {
    libmemcached_free(ptr, ptr->sasl.callbacks[0].context);
    libmemcached_free(ptr, ptr->sasl.callbacks[2].context);
    libmemcached_free(ptr, (void*)ptr->sasl.callbacks);
    ptr->sasl.is_allocated= false;
  }

  ptr->sasl.callbacks= NULL;

  return MEMCACHED_SUCCESS;
}

memcached_return_t memcached_clone_sasl(memcached_st *clone, const  memcached_st *source)
{
  if (LIBMEMCACHED_WITH_SASL_SUPPORT == 0)
  {
    return MEMCACHED_NOT_SUPPORTED;
  }

  if (clone == NULL or source == NULL)
  {
    return MEMCACHED_INVALID_ARGUMENTS;
  }

  if (source->sasl.callbacks == NULL)
  {
    return MEMCACHED_SUCCESS;
  }

  /* Hopefully we are using our own callback mechanisms.. */
  if (source->sasl.callbacks[0].id == SASL_CB_USER &&
      source->sasl.callbacks[0].proc ==  (int (*)())get_username &&
      source->sasl.callbacks[1].id == SASL_CB_AUTHNAME &&
      source->sasl.callbacks[1].proc ==  (int (*)())get_username &&
      source->sasl.callbacks[2].id == SASL_CB_PASS &&
      source->sasl.callbacks[2].proc ==  (int (*)())get_password &&
      source->sasl.callbacks[3].id == SASL_CB_LIST_END)
  {
    sasl_secret_t *secret= (sasl_secret_t *)source->sasl.callbacks[2].context;
    return memcached_set_sasl_auth_data(clone,
                                        (const char*)source->sasl.callbacks[0].context,
                                        (const char*)secret->data);
  }

  /*
   * But we're not. It may work if we know what the user tries to pass
   * into the list, but if we don't know the ID we don't know how to handle
   * the context...
 */
  ptrdiff_t total= 0;

  while (source->sasl.callbacks[total].id != SASL_CB_LIST_END)
  {
    switch (source->sasl.callbacks[total].id)
    {
    case SASL_CB_USER:
    case SASL_CB_AUTHNAME:
    case SASL_CB_PASS:
      break;
    default:
      /* I don't know how to deal with this... */
      return MEMCACHED_NOT_SUPPORTED;
    }

    ++total;
  }

  sasl_callback_t *callbacks= libmemcached_xcalloc(clone, total +1, sasl_callback_t);
  if (callbacks == NULL)
  {
    return MEMCACHED_MEMORY_ALLOCATION_FAILURE;
  }
  memcpy(callbacks, source->sasl.callbacks, (total + 1) * sizeof(sasl_callback_t));

  /* Now update the context... */
  for (ptrdiff_t x= 0; x < total; ++x)
  {
    if (callbacks[x].id == SASL_CB_USER || callbacks[x].id == SASL_CB_AUTHNAME)
    {
      callbacks[x].context= (sasl_callback_t*)libmemcached_malloc(clone, strlen((const char*)source->sasl.callbacks[x].context));

      if (callbacks[x].context == NULL)
      {
        /* Failed to allocate memory, clean up previously allocated memory */
        for (ptrdiff_t y= 0; y < x; ++y)
        {
          libmemcached_free(clone, clone->sasl.callbacks[y].context);
        }

        libmemcached_free(clone, callbacks);
        return MEMCACHED_MEMORY_ALLOCATION_FAILURE;
      }
      strncpy((char*)callbacks[x].context, (const char*)source->sasl.callbacks[x].context, sizeof(callbacks[x].context));
    }
    else
    {
      sasl_secret_t *src= (sasl_secret_t *)source->sasl.callbacks[x].context;
      sasl_secret_t *n= (sasl_secret_t*)libmemcached_malloc(clone, src->len + 1 + sizeof(*n));
      if (n == NULL)
      {
        /* Failed to allocate memory, clean up previously allocated memory */
        for (ptrdiff_t y= 0; y < x; ++y)
        {
          libmemcached_free(clone, clone->sasl.callbacks[y].context);
        }

        libmemcached_free(clone, callbacks);
        return MEMCACHED_MEMORY_ALLOCATION_FAILURE;
      }
      memcpy(n, src, src->len + 1 + sizeof(*n));
      callbacks[x].context= n;
    }
  }

  clone->sasl.callbacks= callbacks;
  clone->sasl.is_allocated= true;

  return MEMCACHED_SUCCESS;
}

#else

void memcached_set_sasl_callbacks(memcached_st *, const sasl_callback_t *)
{
}

sasl_callback_t *memcached_get_sasl_callbacks(memcached_st *)
{
  return NULL;
}

memcached_return_t memcached_set_sasl_auth_data(memcached_st *, const char *, const char *)
{
  return MEMCACHED_NOT_SUPPORTED;
}

memcached_return_t memcached_clone_sasl(memcached_st *, const  memcached_st *)
{
  return MEMCACHED_NOT_SUPPORTED;
}

#endif
