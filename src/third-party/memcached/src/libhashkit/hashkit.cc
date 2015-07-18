/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  HashKit library
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


#include <libhashkit/common.h>

static inline void _hashkit_init(hashkit_st *self)
{
  self->base_hash.function= hashkit_one_at_a_time;
  self->base_hash.context= NULL;

  self->distribution_hash.function= hashkit_one_at_a_time;
  self->distribution_hash.context= NULL;

  self->flags.is_base_same_distributed= true;
  self->_key= NULL;
}

static inline hashkit_st *_hashkit_create(hashkit_st *self)
{
  if (self)
  {
    self->options.is_allocated= false;
  }
  else
  {
    self= (hashkit_st*)calloc(1, sizeof(hashkit_st));
    if (self == NULL)
    {
      return NULL;
    }

    self->options.is_allocated= true;
  }

  return self;
}

hashkit_st *hashkit_create(hashkit_st *self)
{
  self= _hashkit_create(self);
  if (self == NULL)
  {
    return NULL;
  }

  _hashkit_init(self);

  return self;
}


void hashkit_free(hashkit_st *self)
{
  if (self and self->_key)
  {
    free(self->_key);
    self->_key= NULL;
  }

  if (hashkit_is_allocated(self))
  {
    free(self);
  }
}

hashkit_st *hashkit_clone(hashkit_st *destination, const hashkit_st *source)
{
  if (source == NULL)
  {
    return hashkit_create(destination);
  }

  /* new_clone will be a pointer to destination */ 
  destination= _hashkit_create(destination);

  // Should only happen on allocation failure.
  if (destination == NULL)
  {
    return NULL;
  }

  destination->base_hash= source->base_hash;
  destination->distribution_hash= source->distribution_hash;
  destination->flags= source->flags;
  destination->_key= aes_clone_key(static_cast<aes_key_t*>(source->_key));

  return destination;
}

bool hashkit_compare(const hashkit_st *first, const hashkit_st *second)
{
  if (not first or not second)
    return false;

  if (first->base_hash.function == second->base_hash.function and
      first->base_hash.context == second->base_hash.context and
      first->distribution_hash.function == second->distribution_hash.function and
      first->distribution_hash.context == second->distribution_hash.context and
      first->flags.is_base_same_distributed == second->flags.is_base_same_distributed)
  {
    return true;
  }

  return false;
}
