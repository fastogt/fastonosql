/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  libmcachedd client library.
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

#pragma once

#include "util/string.hpp"

#define memcached_literal_param util_literal_param
#define memcached_literal_param_size util_literal_param_size
#define memcached_string_make_from_cstr util_string_make_from_cstr
#define memcached_array_length util_array_length

/**
  Strings are always under our control so we make some assumptions
  about them.

  1) is_initialized is always valid.
  2) A string once intialized will always be, until free where we
     unset this flag.
  3) A string always has a root.
*/

memcached_string_st *memcached_string_create(memcached_st *ptr,
                                             memcached_string_st *string,
                                             size_t initial_size);

memcached_return_t memcached_string_check(memcached_string_st *string, size_t need);

char *memcached_string_c_copy(memcached_string_st *string);

memcached_return_t memcached_string_append_character(memcached_string_st *string,
                                                     char character);

memcached_return_t memcached_string_append(memcached_string_st *string,
                                           const char *value, size_t length);

void memcached_string_reset(memcached_string_st *string);

void memcached_string_free(memcached_string_st *string);
void memcached_string_free(memcached_string_st&);

size_t memcached_string_length(const memcached_string_st *self);
size_t memcached_string_length(const memcached_string_st&);

size_t memcached_string_size(const memcached_string_st *self);

const char *memcached_string_value(const memcached_string_st *self);
const char *memcached_string_value(const memcached_string_st&);

char *memcached_string_take_value(memcached_string_st *self);

char *memcached_string_value_mutable(const memcached_string_st *self);

bool memcached_string_set(memcached_string_st&, const char*, size_t);

void memcached_string_set_length(memcached_string_st *self, size_t length);
void memcached_string_set_length(memcached_string_st&, const size_t length);

bool memcached_string_resize(memcached_string_st&, const size_t);
char *memcached_string_c_str(memcached_string_st&);
