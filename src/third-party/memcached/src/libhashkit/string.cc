/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Libhashkit library
 *
 *  Copyright (C) 2011-2012 Data Differential, http://datadifferential.com/
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

#include <cassert>
#include <cstring>

#define HASHKIT_BLOCK_SIZE 1024

struct hashkit_string_st {
  char *end;
  size_t current_size;
  char *string;
};

inline static bool _string_check(hashkit_string_st *string, size_t need)
{
  if (need and need > (size_t)(string->current_size - (size_t)(string->end - string->string)))
  {
    size_t current_offset= (size_t) (string->end - string->string);

    /* This is the block multiplier. To keep it larger and surive division errors we must round it up */
    size_t adjust= (need - (size_t)(string->current_size - (size_t)(string->end - string->string))) / HASHKIT_BLOCK_SIZE;
    adjust++;

    size_t new_size= sizeof(char) * (size_t)((adjust * HASHKIT_BLOCK_SIZE) + string->current_size);
    /* Test for overflow */
    if (new_size < need)
    {
      return false;
    }

    char *new_value= (char*)realloc(string->string, new_size);

    if (new_value == NULL)
    {
      return false;
    }

    string->string= new_value;
    string->end= string->string + current_offset;

    string->current_size+= (HASHKIT_BLOCK_SIZE * adjust);
  }

  return true;
}

static inline void _init_string(hashkit_string_st *self)
{
  self->current_size= 0;
  self->end= self->string= NULL;
}

hashkit_string_st *hashkit_string_create(size_t initial_size)
{
  hashkit_string_st* self= (hashkit_string_st*)calloc(1, sizeof(hashkit_string_st));

  if (self)
  {
    if (_string_check(self, initial_size) == false)
    {
      free(self);

      return NULL;
    }
  }

  return self;
}

#if 0
static bool hashkit_string_append_null(hashkit_string_st *string)
{
  if (_string_check(string, 1) == false)
  {
    return false;
  }

  *string->end= 0;

  return true;
}
#endif

bool hashkit_string_append_character(hashkit_string_st *string,
                                     char character)
{
  if (_string_check(string, 1) == false)
  {
    return false;
  }

  *string->end= character;
  string->end++;

  return true;
}

bool hashkit_string_append(hashkit_string_st *string,
                           const char *value, size_t length)
{
  if (_string_check(string, length) == false)
  {
    return false;
  }

  assert(length <= string->current_size);
  assert(string->string);
  assert(string->end >= string->string);

  memcpy(string->end, value, length);
  string->end+= length;

  return true;
}

char *hashkit_string_c_copy(hashkit_string_st *string)
{
  if (hashkit_string_length(string) == 0)
  {
    return NULL;
  }

  char *c_ptr= static_cast<char *>(malloc((hashkit_string_length(string)+1) * sizeof(char)));
  if (c_ptr == NULL)
  {
    return NULL;
  }

  memcpy(c_ptr, hashkit_string_c_str(string), hashkit_string_length(string));
  c_ptr[hashkit_string_length(string)]= 0;

  return c_ptr;
}

void hashkit_string_reset(hashkit_string_st *string)
{
  string->end= string->string;
}

void hashkit_string_free(hashkit_string_st *ptr)
{
  if (ptr == NULL)
  {
    return;
  }

  if (ptr->string)
  {
    free(ptr->string);
  }
  free(ptr);
}

bool hashkit_string_resize(hashkit_string_st& string, const size_t need)
{
  return _string_check(&string, need);
}

size_t hashkit_string_length(const hashkit_string_st *self)
{
  return size_t(self->end -self->string);
}

size_t hashkit_string_max_size(const hashkit_string_st *self)
{
  return self->current_size;
}

char *hashkit_string_take(hashkit_string_st *self)
{
  assert(self);
  if (self == NULL)
  {
    return NULL;
  }
  char *value= self->string;

  _init_string(self);

  return value;
}

char *hashkit_string_c_str_mutable(hashkit_string_st *self)
{
  assert(self);
  if (self == NULL)
  {
    return NULL;
  }
  return self->string;
}

const char *hashkit_string_c_str(const hashkit_string_st* self)
{
  assert(self);
  if (self == NULL)
  {
    return NULL;
  }
  return self->string;
}

void hashkit_string_set_length(hashkit_string_st *self, size_t length)
{
  assert(self);
  if (self and _string_check(self, length))
  {
    self->end= self->string +length;
  }
}
