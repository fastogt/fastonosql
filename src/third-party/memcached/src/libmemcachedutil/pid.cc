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
 * Summary: connects to a host, and determines what its pid is
 *
 */

#include <libmemcachedutil/common.h>


// Never look at the stat object directly.


pid_t libmemcached_util_getpid(const char *hostname, in_port_t port, memcached_return_t *ret)
{
  pid_t pid= -1;

  memcached_return_t unused;
  if (ret == NULL)
  {
    ret= &unused;
  }

  memcached_st *memc_ptr= memcached_create(NULL);
  if (memc_ptr == NULL)
  {
    *ret= MEMCACHED_MEMORY_ALLOCATION_FAILURE;
    return -1;
  }

  memcached_return_t rc= memcached_server_add(memc_ptr, hostname, port);
  if (memcached_success(rc))
  {
    memcached_stat_st *stat= memcached_stat(memc_ptr, NULL, &rc);
    if (memcached_success(rc) and stat and stat->pid != -1)
    {
      pid= stat->pid;
    }
    else if (memcached_success(rc))
    {
      rc= MEMCACHED_UNKNOWN_STAT_KEY; // Something went wrong if this happens
    }
    else if (rc == MEMCACHED_SOME_ERRORS) // Generic answer, we will now find the specific reason (if one exists)
    {
      const memcached_instance_st * instance= memcached_server_instance_by_position(memc_ptr, 0);

      assert_msg(instance and memcached_server_error(instance), " ");
      if (instance and memcached_server_error(instance))
      {
        rc= memcached_server_error_return(instance);
      }
    }

    memcached_stat_free(memc_ptr, stat);
  }
  memcached_free(memc_ptr);

  *ret= rc;

  return pid;
}

pid_t libmemcached_util_getpid2(const char *hostname, in_port_t port, const char *username, const char *password,  memcached_return_t *ret)
{
  if (username == NULL)
  {
    return libmemcached_util_getpid(hostname, port, ret);
  }

  pid_t pid= -1;

  memcached_return_t unused;
  if (not ret)
    ret= &unused;

  if (LIBMEMCACHED_WITH_SASL_SUPPORT == 0)
  {
    *ret= MEMCACHED_NOT_SUPPORTED;
    return pid;
  }

  memcached_st *memc_ptr= memcached_create(NULL);
  if (not memc_ptr)
  {
    *ret= MEMCACHED_MEMORY_ALLOCATION_FAILURE;
    return -1;
  }

  if (memcached_failed(*ret= memcached_set_sasl_auth_data(memc_ptr, username, password)))
  {
    memcached_free(memc_ptr);
    return false;
  }


  memcached_return_t rc= memcached_server_add(memc_ptr, hostname, port);
  if (memcached_success(rc))
  {
    memcached_stat_st *stat= memcached_stat(memc_ptr, NULL, &rc);
    if (memcached_success(rc) and stat and stat->pid != -1)
    {
      pid= stat->pid;
    }
    else if (memcached_success(rc))
    {
      rc= MEMCACHED_UNKNOWN_STAT_KEY; // Something went wrong if this happens
    }
    else if (rc == MEMCACHED_SOME_ERRORS) // Generic answer, we will now find the specific reason (if one exists)
    {
      const memcached_instance_st * instance=
        memcached_server_instance_by_position(memc_ptr, 0);

#if 0
      assert_msg(instance and instance->error_messages, " ");
#endif
      if (instance and memcached_server_error(instance))
      {
        rc= memcached_server_error_return(instance);
      }
    }

    memcached_stat_free(memc_ptr, stat);
  }
  memcached_free(memc_ptr);

  *ret= rc;

  return pid;
}
