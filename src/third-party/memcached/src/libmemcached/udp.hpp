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

#define MAX_UDP_DATAGRAM_LENGTH 1400
#define UDP_DATAGRAM_HEADER_LENGTH 8
#define UDP_REQUEST_ID_MSG_SIG_DIGITS 10
#define UDP_REQUEST_ID_THREAD_MASK 0xFFFF << UDP_REQUEST_ID_MSG_SIG_DIGITS
#define get_udp_datagram_request_id(A) ntohs((A)->request_id)
#define get_udp_datagram_seq_num(A) ntohs((A)->sequence_number)
#define get_udp_datagram_num_datagrams(A) ntohs((A)->num_datagrams)
#define get_msg_num_from_request_id(A) ( (A) & (~(UDP_REQUEST_ID_THREAD_MASK)) )
#define get_thread_id_from_request_id(A) ( (A) & (UDP_REQUEST_ID_THREAD_MASK) ) >> UDP_REQUEST_ID_MSG_SIG_DIGITS
#define generate_udp_request_thread_id(A) (A) << UDP_REQUEST_ID_MSG_SIG_DIGITS
#define UDP_REQUEST_ID_MAX_THREAD_ID get_thread_id_from_request_id(0xFFFF)

struct udp_datagram_header_st
{
  uint16_t request_id;
  uint16_t sequence_number;
  uint16_t num_datagrams;
  uint16_t reserved;
};

bool memcached_io_init_udp_header(memcached_instance_st*, const uint16_t thread_id);
void increment_udp_message_id(memcached_instance_st*);
