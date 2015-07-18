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
#include <cerrno>
#include <pthread.h>
#include <memory>

struct memcached_pool_st
{
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  memcached_st *master;
  memcached_st **server_pool;
  int firstfree;
  const uint32_t size;
  uint32_t current_size;
  bool _owns_master;
  struct timespec _timeout;

  memcached_pool_st(memcached_st *master_arg, size_t max_arg) :
    master(master_arg),
    server_pool(NULL),
    firstfree(-1),
    size(uint32_t(max_arg)),
    current_size(0),
    _owns_master(false)
  {
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    _timeout.tv_sec= 5;
    _timeout.tv_nsec= 0;
  }

  const struct timespec& timeout() const
  {
    return _timeout;
  }

  bool release(memcached_st*, memcached_return_t& rc);

  memcached_st *fetch(memcached_return_t& rc);
  memcached_st *fetch(const struct timespec&, memcached_return_t& rc);

  bool init(uint32_t initial);

  ~memcached_pool_st()
  {
    for (int x= 0; x <= firstfree; ++x)
    {
      memcached_free(server_pool[x]);
      server_pool[x]= NULL;
    }

    int error;
    if ((error= pthread_mutex_destroy(&mutex)) != 0)
    {
      assert_vmsg(error != 0, "pthread_mutex_destroy() %s(%d)", strerror(error), error);
    }

    if ((error= pthread_cond_destroy(&cond)) != 0)
    {
      assert_vmsg(error != 0, "pthread_cond_destroy() %s", strerror(error));
    }

    delete [] server_pool;
    if (_owns_master)
    {
      memcached_free(master);
    }
  }

  void increment_version()
  {
    ++master->configure.version;
  }

  bool compare_version(const memcached_st *arg) const
  {
    return (arg->configure.version == version());
  }

  int32_t version() const
  {
    return master->configure.version;
  }
};


/**
 * Grow the connection pool by creating a connection structure and clone the
 * original memcached handle.
 */
static bool grow_pool(memcached_pool_st* pool)
{
  assert(pool);

  memcached_st *obj;
  if (not (obj= memcached_clone(NULL, pool->master)))
  {
    return false;
  }

  pool->server_pool[++pool->firstfree]= obj;
  pool->current_size++;
  obj->configure.version= pool->version();

  return true;
}

bool memcached_pool_st::init(uint32_t initial)
{
  server_pool= new (std::nothrow) memcached_st *[size];
  if (server_pool == NULL)
  {
    return false;
  }

  /*
    Try to create the initial size of the pool. An allocation failure at
    this time is not fatal..
  */
  for (unsigned int x= 0; x < initial; ++x)
  {
    if (grow_pool(this) == false)
    {
      break;
    }
  }

  return true;
}


static inline memcached_pool_st *_pool_create(memcached_st* master, uint32_t initial, uint32_t max)
{
  if (initial == 0 or max == 0 or (initial > max))
  {
    return NULL;
  }

  memcached_pool_st *object= new (std::nothrow) memcached_pool_st(master, max);
  if (object == NULL)
  {
    return NULL;
  }

  /*
    Try to create the initial size of the pool. An allocation failure at
    this time is not fatal..
  */
  if (not object->init(initial))
  {
    delete object;
    return NULL;
  }

  return object;
}

memcached_pool_st *memcached_pool_create(memcached_st* master, uint32_t initial, uint32_t max)
{
  return _pool_create(master, initial, max);
}

memcached_pool_st * memcached_pool(const char *option_string, size_t option_string_length)
{
  memcached_st *memc= memcached(option_string, option_string_length);

  if (memc == NULL)
  {
    return NULL;
  }

  memcached_pool_st *self= memcached_pool_create(memc, memc->configure.initial_pool_size, memc->configure.max_pool_size);
  if (self == NULL)
  {
    memcached_free(memc);
    return NULL;
  }

  self->_owns_master= true;

  return self;
}

memcached_st*  memcached_pool_destroy(memcached_pool_st* pool)
{
  if (pool == NULL)
  {
    return NULL;
  }

  // Legacy that we return the original structure
  memcached_st *ret= NULL;
  if (pool->_owns_master)
  { }
  else
  {
    ret= pool->master;
  }

  delete pool;

  return ret;
}

memcached_st* memcached_pool_st::fetch(memcached_return_t& rc)
{
  static struct timespec relative_time= { 0, 0 };
  return fetch(relative_time, rc);
}

memcached_st* memcached_pool_st::fetch(const struct timespec& relative_time, memcached_return_t& rc)
{
  rc= MEMCACHED_SUCCESS;

  int error;
  if ((error= pthread_mutex_lock(&mutex)) != 0)
  {
    rc= MEMCACHED_IN_PROGRESS;
    return NULL;
  }

  memcached_st *ret= NULL;
  do
  {
    if (firstfree > -1)
    {
      ret= server_pool[firstfree--];
    }
    else if (current_size == size)
    {
      if (relative_time.tv_sec == 0 and relative_time.tv_nsec == 0)
      {
        error= pthread_mutex_unlock(&mutex);
        rc= MEMCACHED_NOTFOUND;

        return NULL;
      }

      struct timespec time_to_wait= {0, 0};
      time_to_wait.tv_sec= time(NULL) +relative_time.tv_sec;
      time_to_wait.tv_nsec= relative_time.tv_nsec;

      int thread_ret;
      if ((thread_ret= pthread_cond_timedwait(&cond, &mutex, &time_to_wait)) != 0)
      {
        int unlock_error;
        if ((unlock_error= pthread_mutex_unlock(&mutex)) != 0)
        {
          assert_vmsg(error != 0, "pthread_mutex_unlock() %s", strerror(error));
        }

        if (thread_ret == ETIMEDOUT)
        {
          rc= MEMCACHED_TIMEOUT;
        }
        else
        {
          errno= thread_ret;
          rc= MEMCACHED_ERRNO;
        }

        return NULL;
      }
    }
    else if (grow_pool(this) == false)
    {
      int unlock_error;
      if ((unlock_error= pthread_mutex_unlock(&mutex)) != 0)
      {
        assert_vmsg(error != 0, "pthread_mutex_unlock() %s", strerror(error));
      }

      return NULL;
    }
  } while (ret == NULL);

  if ((error= pthread_mutex_unlock(&mutex)) != 0)
  {
    assert_vmsg(error != 0, "pthread_mutex_unlock() %s", strerror(error));
  }

  return ret;
}

bool memcached_pool_st::release(memcached_st *released, memcached_return_t& rc)
{
  rc= MEMCACHED_SUCCESS;
  if (released == NULL)
  {
    rc= MEMCACHED_INVALID_ARGUMENTS;
    return false;
  }

  int error;
  if ((error= pthread_mutex_lock(&mutex)))
  {
    rc= MEMCACHED_IN_PROGRESS;
    return false;
  }

  /* 
    Someone updated the behavior on the object, so we clone a new memcached_st with the new settings. If we fail to clone, we keep the old one around.
  */
  if (compare_version(released) == false)
  {
    memcached_st *memc;
    if ((memc= memcached_clone(NULL, master)))
    {
      memcached_free(released);
      released= memc;
    }
  }

  server_pool[++firstfree]= released;

  if (firstfree == 0 and current_size == size)
  {
    /* we might have people waiting for a connection.. wake them up :-) */
    if ((error= pthread_cond_broadcast(&cond)) != 0)
    {
      assert_vmsg(error != 0, "pthread_cond_broadcast() %s", strerror(error));
    }
  }

  if ((error= pthread_mutex_unlock(&mutex)) != 0)
  {
  }

  return true;
}

memcached_st* memcached_pool_fetch(memcached_pool_st* pool, struct timespec* relative_time, memcached_return_t* rc)
{
  if (pool == NULL)
  {
    return NULL;
  }

  memcached_return_t unused;
  if (rc == NULL)
  {
    rc= &unused;
  }

  if (relative_time == NULL)
  {
    return pool->fetch(*rc);
  }

  return pool->fetch(*relative_time, *rc);
}

memcached_st* memcached_pool_pop(memcached_pool_st* pool,
                                 bool block,
                                 memcached_return_t *rc)
{
  if (pool == NULL)
  {
    return NULL;
  }

  memcached_return_t unused;
  if (rc == NULL)
  {
    rc= &unused;
  }

  memcached_st *memc;
  if (block)
  {
    memc= pool->fetch(pool->timeout(), *rc);
  }
  else
  {
    memc= pool->fetch(*rc);
  }

  return memc;
}

memcached_return_t memcached_pool_release(memcached_pool_st* pool, memcached_st *released)
{
  if (pool == NULL)
  {
    return MEMCACHED_INVALID_ARGUMENTS;
  }

  memcached_return_t rc;

  (void) pool->release(released, rc);

  return rc;
}

memcached_return_t memcached_pool_push(memcached_pool_st* pool, memcached_st *released)
{
  return memcached_pool_release(pool, released);
}


memcached_return_t memcached_pool_behavior_set(memcached_pool_st *pool,
                                               memcached_behavior_t flag,
                                               uint64_t data)
{
  if (pool == NULL)
  {
    return MEMCACHED_INVALID_ARGUMENTS;
  }

  int error;
  if ((error= pthread_mutex_lock(&pool->mutex)))
  {
    return MEMCACHED_IN_PROGRESS;
  }

  /* update the master */
  memcached_return_t rc= memcached_behavior_set(pool->master, flag, data);
  if (memcached_failed(rc))
  {
    if ((error= pthread_mutex_unlock(&pool->mutex)) != 0)
    {
      assert_vmsg(error != 0, "pthread_mutex_unlock() %s", strerror(error));
    }
    return rc;
  }

  pool->increment_version();
  /* update the clones */
  for (int xx= 0; xx <= pool->firstfree; ++xx)
  {
    if (memcached_success(memcached_behavior_set(pool->server_pool[xx], flag, data)))
    {
      pool->server_pool[xx]->configure.version= pool->version();
    }
    else
    {
      memcached_st *memc;
      if ((memc= memcached_clone(NULL, pool->master)))
      {
        memcached_free(pool->server_pool[xx]);
        pool->server_pool[xx]= memc;
        /* I'm not sure what to do in this case.. this would happen
          if we fail to push the server list inside the client..
          I should add a testcase for this, but I believe the following
          would work, except that you would add a hole in the pool list..
          in theory you could end up with an empty pool....
        */
      }
    }
  }

  if ((error= pthread_mutex_unlock(&pool->mutex)) != 0)
  {
    assert_vmsg(error != 0, "pthread_mutex_unlock() %s", strerror(error));
  }

  return rc;
}

memcached_return_t memcached_pool_behavior_get(memcached_pool_st *pool,
                                               memcached_behavior_t flag,
                                               uint64_t *value)
{
  if (pool == NULL)
  {
    return MEMCACHED_INVALID_ARGUMENTS;
  }

  int error;
  if ((error= pthread_mutex_lock(&pool->mutex)))
  {
    return MEMCACHED_IN_PROGRESS;
  }

  *value= memcached_behavior_get(pool->master, flag);

  if ((error= pthread_mutex_unlock(&pool->mutex)) != 0)
  {
    assert_vmsg(error != 0, "pthread_mutex_unlock() %s", strerror(error));
  }

  return MEMCACHED_SUCCESS;
}
