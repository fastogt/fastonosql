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


#pragma once

#ifdef __cplusplus

struct memcached_st;
struct memcached_stat_st;
struct memcached_analysis_st;
struct memcached_result_st;
struct memcached_array_st;
struct memcached_error_t;

// All of the flavors of memcache_server_st
struct memcached_server_st;
struct memcached_instance_st;
typedef struct memcached_instance_st memcached_instance_st;
typedef struct memcached_server_st *memcached_server_list_st;

struct memcached_callback_st;

// The following two structures are internal, and never exposed to users.
struct memcached_string_st;
struct memcached_string_t;
struct memcached_continuum_item_st;

#else

typedef struct memcached_st memcached_st;
typedef struct memcached_stat_st memcached_stat_st;
typedef struct memcached_analysis_st memcached_analysis_st;
typedef struct memcached_result_st memcached_result_st;
typedef struct memcached_array_st memcached_array_st;
typedef struct memcached_error_t memcached_error_t;

// All of the flavors of memcache_server_st
typedef struct memcached_server_st memcached_server_st;
typedef struct memcached_instance_st memcached_instance_st;
typedef struct memcached_server_st *memcached_server_list_st;

typedef struct memcached_callback_st memcached_callback_st;

// The following two structures are internal, and never exposed to users.
typedef struct memcached_string_st memcached_string_st;
typedef struct memcached_string_t memcached_string_t;

#endif
