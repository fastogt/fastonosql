/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  LibMemcached
 *
 *  Copyright (C) 2011 Data Differential, http://datadifferential.com/
 *  Copyright (C) 2006-2009 Brian Aker
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

void initialize_binary_request(memcached_instance_st* server, protocol_binary_request_header&);

bool memcached_io_write(memcached_instance_st* ptr);

ssize_t memcached_io_write(memcached_instance_st* ptr,
                           const void *buffer, size_t length, bool with_flush);

bool memcached_io_writev(memcached_instance_st* ptr,
                         libmemcached_io_vector_st vector[],
                         const size_t number_of, const bool with_flush);

memcached_return_t memcached_io_wait_for_write(memcached_instance_st*);
memcached_return_t memcached_io_wait_for_read(memcached_instance_st*);

void memcached_io_reset(memcached_instance_st* ptr);

memcached_return_t memcached_io_read(memcached_instance_st* ptr,
                                     void *buffer, size_t length, ssize_t& nread);

/* Read a line (terminated by '\n') into the buffer */
memcached_return_t memcached_io_readline(memcached_instance_st* ptr,
                                         char *buffer_ptr,
                                         size_t size,
                                         size_t& total);

/* Read n bytes of data from the server and store them in dta */
memcached_return_t memcached_safe_read(memcached_instance_st* ptr,
                                       void *dta,
                                       const size_t size);

memcached_instance_st* memcached_io_get_readable_server(memcached_st *memc, memcached_return_t&);

memcached_return_t memcached_io_slurp(memcached_instance_st* ptr);
