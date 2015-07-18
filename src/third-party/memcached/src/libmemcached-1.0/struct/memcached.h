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

struct memcached_st {
  /**
    @note these are static and should not change without a call to behavior.
  */
  struct {
    bool is_purging:1;
    bool is_processing_input:1;
    bool is_time_for_rebuild:1;
    bool is_parsing:1;
  } state;

  struct {
    // Everything below here is pretty static.
    bool auto_eject_hosts:1;
    bool binary_protocol:1;
    bool buffer_requests:1;
    bool hash_with_namespace:1;
    bool no_block:1; // Don't block
    bool reply:1;
    bool randomize_replica_read:1;
    bool support_cas:1;
    bool tcp_nodelay:1;
    bool use_sort_hosts:1;
    bool use_udp:1;
    bool verify_key:1;
    bool tcp_keepalive:1;
    bool is_aes:1;
    bool is_fetching_version:1;
    bool not_used:1;
  } flags;

  memcached_server_distribution_t distribution;
  hashkit_st hashkit;
  struct {
    unsigned int version;
  } server_info;
  uint32_t number_of_hosts;
  memcached_instance_st *servers;
  memcached_instance_st *last_disconnected_server;
  int32_t snd_timeout;
  int32_t rcv_timeout;
  uint32_t server_failure_limit;
  uint32_t server_timeout_limit;
  uint32_t io_msg_watermark;
  uint32_t io_bytes_watermark;
  uint32_t io_key_prefetch;
  uint32_t tcp_keepidle;
  int32_t poll_timeout;
  int32_t connect_timeout; // How long we will wait on connect() before we will timeout
  int32_t retry_timeout;
  int32_t dead_timeout;
  int send_size;
  int recv_size;
  void *user_data;
  uint64_t query_id;
  uint32_t number_of_replicas;
  memcached_result_st result;

  struct {
    bool weighted_;
    uint32_t continuum_count; // Ketama
    uint32_t continuum_points_counter; // Ketama
    time_t next_distribution_rebuild; // Ketama
    struct memcached_continuum_item_st *continuum; // Ketama
  } ketama;

  struct memcached_virtual_bucket_t *virtual_bucket;

  struct memcached_allocator_t allocators;

  memcached_clone_fn on_clone;
  memcached_cleanup_fn on_cleanup;
  memcached_trigger_key_fn get_key_failure;
  memcached_trigger_delete_key_fn delete_trigger;
  memcached_callback_st *callbacks;
  struct memcached_sasl_st sasl;
  struct memcached_error_t *error_messages;
  struct memcached_array_st *_namespace;
  struct {
    uint32_t initial_pool_size;
    uint32_t max_pool_size;
    int32_t version; // This is used by pool and others to determine if the memcached_st is out of date.
    struct memcached_array_st *filename;
  } configure;
  struct {
    bool is_allocated:1;
  } options;

};
