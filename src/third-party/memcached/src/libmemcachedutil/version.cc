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
 */


#include <libmemcachedutil/common.h>
#include <cassert>

struct local_context
{
  uint8_t major_version;
  uint8_t minor_version;
  uint8_t micro_version;

  bool truth;
};

static memcached_return_t check_server_version(const memcached_st *,
                                               const memcached_instance_st * instance,
                                               void *context)
{
  /* Do Nothing */
  struct local_context *check= (struct local_context *)context;

  if (memcached_server_major_version(instance) != UINT8_MAX &&
      memcached_server_major_version(instance) >= check->major_version and
      memcached_server_minor_version(instance) >= check->minor_version and
      memcached_server_micro_version(instance) >= check->micro_version )
  {
    return MEMCACHED_SUCCESS;
  }

  check->truth= false;

  return MEMCACHED_FAILURE;
}

bool libmemcached_util_version_check(memcached_st *memc,
                                     uint8_t major_version,
                                     uint8_t minor_version,
                                     uint8_t micro_version)
{
  if (memcached_failed(memcached_version(memc)))
  {
    return false;
  }

  struct local_context check= { major_version, minor_version, micro_version, true };

  memcached_server_fn callbacks[1];
  callbacks[0]= check_server_version;
  memcached_server_cursor(memc, callbacks, (void *)&check,  1);

  return check.truth;
}
