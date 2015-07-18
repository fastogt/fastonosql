/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  LibMemcached
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

/* These are private */ 
#define memcached_is_allocated(__object) ((__object)->options.is_allocated)
#define memcached_is_encrypted(__object) ((__object)->hashkit._key)
#define memcached_is_initialized(__object) ((__object)->options.is_initialized)
#define memcached_is_purging(__object) ((__object)->state.is_purging)
#define memcached_is_processing_input(__object) ((__object)->state.is_processing_input)

#define memcached_is_aes(__object) ((__object)->flags.is_aes)
#define memcached_is_udp(__object) ((__object)->flags.use_udp)
#define memcached_is_verify_key(__object) ((__object)->flags.verify_key)
#define memcached_is_binary(__object) ((__object)->flags.binary_protocol)
#define memcached_is_fetching_version(__object) ((__object)->flags.is_fetching_version)
#define memcached_is_buffering(__object) ((__object)->flags.buffer_requests)
#define memcached_is_replying(__object) ((__object)->flags.reply)
#define memcached_is_cas(__object) ((__object)->flags.reply)
#define memcached_is_randomize_replica_read(__object) ((__object)->flags.randomize_replica_read)
#define memcached_is_no_block(__object) ((__object)->flags.no_block)
#define memcached_is_hash_with_namespace(__object) ((__object)->flags.hash_with_namespace)
#define memcached_is_tcp_nodelay(__object) ((__object)->flags.tcp_nodelay)
#define memcached_is_auto_eject_hosts(__object) ((__object)->flags.auto_eject_hosts)
#define memcached_is_use_sort_hosts(__object) ((__object)->flags.use_sort_hosts)

#define memcached_is_ready(__object) ((__object)->options.ready)

#define memcached_is_weighted_ketama(__object) ((__object)->ketama.weighted_)

#define memcached_set_ready(__object, __flag) ((__object)->options.ready= (__flag))

#define memcached_set_aes(__object, __flag) ((__object).flags.is_aes= __flag)
#define memcached_set_udp(__object, __flag) ((__object).flags.use_udp= __flag)
#define memcached_set_verify_key(__object, __flag) ((__object).flags.verify_key= __flag)
#define memcached_set_binary(__object, __flag) ((__object).flags.binary_protocol= __flag)
#define memcached_set_fetching_version(__object, __flag) ((__object).flags.is_fetching_version= __flag)
#define memcached_set_buffering(__object, __flag) ((__object).flags.buffer_requests= __flag)
#define memcached_set_replying(__object, __flag) ((__object).flags.reply= __flag)
#define memcached_set_cas(__object, __flag) ((__object).flags.reply= __flag)
#define memcached_set_randomize_replica_read(__object, __flag) ((__object).flags.randomize_replica_read= __flag)
#define memcached_set_no_block(__object, __flag) ((__object).flags.no_block= __flag)
#define memcached_set_hash_with_namespace(__object, __flag) ((__object).flags.hash_with_namespace= __flag)
#define memcached_set_tcp_nodelay(__object, __flag) ((__object).flags.tcp_nodelay= __flag)
#define memcached_set_auto_eject_hosts(__object, __flag) ((__object).flags.auto_eject_hosts= __flag)
#define memcached_set_use_sort_hosts(__object, __flag) ((__object).flags.use_sort_hosts= __flag)

#define memcached_has_root(__object) ((__object)->root)

#define memcached_has_error(__object) ((__object)->error_messages)

#define memcached_has_replicas(__object) ((__object)->root->number_of_replicas)

#define memcached_set_processing_input(__object, __value) ((__object)->state.is_processing_input= (__value))
#define memcached_set_initialized(__object, __value) ((__object)->options.is_initialized= (__value))
#define memcached_set_allocated(__object, __value) ((__object)->options.is_allocated= (__value))

#define memcached_set_weighted_ketama(__object, __value) ((__object)->ketama.weighted_= (__value))

#define memcached2Memcached(__obj) (__obj)
