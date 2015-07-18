/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Libmemcached library
 *
 *  Copyright (C) 2011 Data Differential, http://datadifferential.com/
 *  Copyright (C) 2010 Brian Aker All rights reserved.
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
 * Summary: connects to a host, and makes sure it is alive.
 *
 */

#include <libmemcachedutil/common.h>

bool libmemcached_util_ping(const char *hostname, in_port_t port, memcached_return_t *ret)
{
  memcached_return_t unused;
  if (ret == NULL)
  {
    ret= &unused;
  }

  memcached_st *memc_ptr= memcached_create(NULL);
  if (memc_ptr == NULL)
  {
    *ret= MEMCACHED_MEMORY_ALLOCATION_FAILURE;
    return false;
  }

  if (memcached_success((*ret= memcached_behavior_set(memc_ptr, MEMCACHED_BEHAVIOR_CONNECT_TIMEOUT, 400000))))
  {
    memcached_return_t rc= memcached_server_add(memc_ptr, hostname, port);
    if (memcached_success(rc))
    {
      rc= memcached_version(memc_ptr);
    }

    if (memcached_failed(rc) and rc == MEMCACHED_SOME_ERRORS)
    {
      const memcached_instance_st * instance=
        memcached_server_instance_by_position(memc_ptr, 0);

      assert_msg(instance and memcached_server_error(instance), " ");
      if (instance and memcached_server_error(instance))
      {
        rc= memcached_server_error_return(instance);
      }
    }

    *ret= rc;
  }
  memcached_free(memc_ptr);

  return memcached_success(*ret);
}

bool libmemcached_util_ping2(const char *hostname, in_port_t port, const char *username, const char *password,  memcached_return_t *ret)
{
  if (username == NULL)
  {
    return libmemcached_util_ping(hostname, port, ret);
  }

  memcached_return_t unused;
  if (not ret)
    ret= &unused;

  if (LIBMEMCACHED_WITH_SASL_SUPPORT == 0)
  {
    *ret= MEMCACHED_NOT_SUPPORTED;
    return false;
  }

  memcached_st *memc_ptr= memcached_create(NULL);
  if (not memc_ptr)
  {
    *ret= MEMCACHED_MEMORY_ALLOCATION_FAILURE;
    return false;
  }

  if (memcached_failed(*ret= memcached_set_sasl_auth_data(memc_ptr, username, password)))
  {
    memcached_free(memc_ptr);
    return false;
  }

  memcached_return_t rc= memcached_server_add(memc_ptr, hostname, port);
  if (memcached_success(rc))
  {
    rc= memcached_version(memc_ptr);
  }

  if (memcached_failed(rc) and rc == MEMCACHED_SOME_ERRORS)
  {
    const memcached_instance_st * instance=
      memcached_server_instance_by_position(memc_ptr, 0);

    assert_msg(instance and memcached_server_error(instance), " ");
    if (instance and memcached_server_error(instance))
    {
      rc= memcached_server_error_return(instance);
    }
  }
  memcached_free(memc_ptr);

  *ret= rc;

  return memcached_success(rc);
}
