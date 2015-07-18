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

inline static memcached_return_t _string_check(memcached_string_st *string, size_t need)
{
  if (need && need > (size_t)(string->current_size - (size_t)(string->end - string->string)))
  {
    size_t current_offset= (size_t) (string->end - string->string);

    /* This is the block multiplier. To keep it larger and surive division errors we must round it up */
    size_t adjust= (need - (size_t)(string->current_size - (size_t)(string->end - string->string))) / MEMCACHED_BLOCK_SIZE;
    adjust++;

    size_t new_size= sizeof(char) * (size_t)((adjust * MEMCACHED_BLOCK_SIZE) + string->current_size);
    /* Test for overflow */
    if (new_size < need)
    {
      char error_message[1024];
      int error_message_length= snprintf(error_message, sizeof(error_message),"Needed %ld, got %ld", (long)need, (long)new_size);
      return memcached_set_error(*string->root, MEMCACHED_MEMORY_ALLOCATION_FAILURE, MEMCACHED_AT, error_message, error_message_length);
    }

    char *new_value= libmemcached_xrealloc(string->root, string->string, new_size, char);

    if (new_value == NULL)
    {
      return memcached_set_error(*string->root, MEMCACHED_MEMORY_ALLOCATION_FAILURE, MEMCACHED_AT);
    }

    string->string= new_value;
    string->end= string->string + current_offset;

    string->current_size+= (MEMCACHED_BLOCK_SIZE * adjust);
  }

  return MEMCACHED_SUCCESS;
}

static inline void _init_string(memcached_string_st *self)
{
  self->current_size= 0;
  self->end= self->string= NULL;
}

memcached_string_st *memcached_string_create(Memcached *memc, memcached_string_st *self, size_t initial_size)
{
  WATCHPOINT_ASSERT(memc);

  /* Saving malloc calls :) */
  if (self)
  {
    WATCHPOINT_ASSERT(self->options.is_initialized == false);

    memcached_set_allocated(self, false);
  }
  else
  {
    self= libmemcached_xmalloc(memc, memcached_string_st);

    if (self == NULL)
    {
      return NULL;
    }

    memcached_set_allocated(self, true);
  }
  self->root= memc;

  _init_string(self);

  if (memcached_failed(_string_check(self, initial_size)))
  {
    if (memcached_is_allocated(self))
    {
      libmemcached_free(memc, self);
    }

    return NULL;
  }

  memcached_set_initialized(self, true);

  WATCHPOINT_ASSERT(self->string == self->end);

  return self;
}

static memcached_return_t memcached_string_append_null(memcached_string_st& string)
{
  if (memcached_failed(_string_check(&string, 1)))
  {
    return MEMCACHED_MEMORY_ALLOCATION_FAILURE;
  }

  *string.end= 0;

  return MEMCACHED_SUCCESS;
}

static memcached_return_t memcached_string_append_null(memcached_string_st *string)
{
  if (memcached_failed(_string_check(string, 1)))
  {
    return MEMCACHED_MEMORY_ALLOCATION_FAILURE;
  }

  *string->end= 0;

  return MEMCACHED_SUCCESS;
}

memcached_return_t memcached_string_append_character(memcached_string_st *string,
                                                     char character)
{
  if (memcached_failed(_string_check(string, 1)))
  {
    return MEMCACHED_MEMORY_ALLOCATION_FAILURE;
  }

  *string->end= character;
  string->end++;

  return MEMCACHED_SUCCESS;
}

memcached_return_t memcached_string_append(memcached_string_st *string,
                                           const char *value, size_t length)
{
  if (memcached_failed(_string_check(string, length)))
  {
    return MEMCACHED_MEMORY_ALLOCATION_FAILURE;
  }

  WATCHPOINT_ASSERT(length <= string->current_size);
  WATCHPOINT_ASSERT(string->string);
  WATCHPOINT_ASSERT(string->end >= string->string);

  memcpy(string->end, value, length);
  string->end+= length;

  return MEMCACHED_SUCCESS;
}

char *memcached_string_c_copy(memcached_string_st *string)
{
  if (memcached_string_length(string) == 0)
  {
    return NULL;
  }

  char *c_ptr= static_cast<char *>(libmemcached_malloc(string->root, (memcached_string_length(string)+1) * sizeof(char)));

  if (c_ptr == NULL)
  {
    return NULL;
  }

  memcpy(c_ptr, memcached_string_value(string), memcached_string_length(string));
  c_ptr[memcached_string_length(string)]= 0;

  return c_ptr;
}

bool memcached_string_set(memcached_string_st& string, const char* value, size_t length)
{
  memcached_string_reset(&string);
  if (memcached_success(memcached_string_append(&string, value, length)))
  {
    memcached_string_append_null(string);
    return true;
  }

  return false;
}

void memcached_string_reset(memcached_string_st *string)
{
  string->end= string->string;
}

void memcached_string_free(memcached_string_st& ptr)
{
  memcached_string_free(&ptr);
}

void memcached_string_free(memcached_string_st *ptr)
{
  if (ptr == NULL)
  {
    return;
  }

  if (ptr->string)
  {
    libmemcached_free(ptr->root, ptr->string);
  }

  if (memcached_is_allocated(ptr))
  {
    libmemcached_free(ptr->root, ptr);
  }
  else
  {
    ptr->options.is_initialized= false;
  }
}

memcached_return_t memcached_string_check(memcached_string_st *string, size_t need)
{
  return _string_check(string, need);
}

bool memcached_string_resize(memcached_string_st& string, const size_t need)
{
  return memcached_success(_string_check(&string, need));
}

size_t memcached_string_length(const memcached_string_st *self)
{
  return size_t(self->end -self->string);
}

size_t memcached_string_length(const memcached_string_st& self)
{
  return size_t(self.end -self.string);
}

size_t memcached_string_size(const memcached_string_st *self)
{
  return self->current_size;
}

const char *memcached_string_value(const memcached_string_st *self)
{
  return self->string;
}

const char *memcached_string_value(const memcached_string_st& self)
{
  return self.string;
}

char *memcached_string_take_value(memcached_string_st *self)
{
  char* value= NULL;

  assert_msg(self, "Invalid memcached_string_st");
  if (self)
  {
    if (memcached_string_length(self))
    {
      // If we fail at adding the null, we copy and move on
      if (memcached_failed(memcached_string_append_null(self)))
      {
        return NULL;
      }

      value= self->string;
      _init_string(self);
    }
  }

  return value;
}

char *memcached_string_value_mutable(const memcached_string_st *self)
{
  return self->string;
}

char *memcached_string_c_str(memcached_string_st& self)
{
  return self.string;
}

void memcached_string_set_length(memcached_string_st *self, size_t length)
{
  self->end= self->string +length;
}

void memcached_string_set_length(memcached_string_st& self, const size_t length)
{
  assert(self.current_size >= length);
  size_t set_length= length;
  if (self.current_size > length)
  {
    if (memcached_failed(_string_check(&self, length)))
    {
      set_length= self.current_size;
    }
  }
  self.end= self.string +set_length;
}
