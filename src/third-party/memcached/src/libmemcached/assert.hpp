/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  libmcachedd client library.
 *
 *  Copyright (C) 2011-2013 Data Differential, http://datadifferential.com/
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

#ifdef __cplusplus
# include <cassert>
#else
# include <assert.h>
#endif // __cplusplus

#ifdef NDEBUG
# define assert_msg(__expr, __mesg) (void)(__expr); (void)(__mesg);
# define assert_vmsg(__expr, __mesg, ...) (void)(__expr); (void)(__mesg);
#else

# ifdef _WIN32
#  include <malloc.h>
# else
#  include <alloca.h>
# endif

#ifdef __cplusplus
# include <cstdarg>
# include <cstdio>
#else
# include <stdarg.h>
# include <stdio.h>
#endif

# include <libmemcached/backtrace.hpp>

# define assert_msg(__expr, __mesg) \
do \
{ \
  if (not (__expr)) \
  { \
    fprintf(stderr, "\n%s:%d Assertion \"%s\" failed for function \"%s\" likely for %s\n", __FILE__, __LINE__, #__expr, __func__, (#__mesg));\
    custom_backtrace(); \
    abort(); \
  } \
} while (0)

# define assert_vmsg(__expr, __mesg, ...) \
do \
{ \
  if (not (__expr)) \
  { \
    size_t ask= snprintf(0, 0, (__mesg), __VA_ARGS__); \
    ask++; \
    char *_error_message= (char*)alloca(sizeof(char) * ask); \
    size_t _error_message_size= snprintf(_error_message, ask, (__mesg), __VA_ARGS__); \
    fprintf(stderr, "\n%s:%d Assertion '%s' failed for function '%s' [ %.*s ]\n", __FILE__, __LINE__, #__expr, __func__, int(_error_message_size), _error_message);\
    custom_backtrace(); \
    abort(); \
  } \
} while (0)

#endif // NDEBUG
