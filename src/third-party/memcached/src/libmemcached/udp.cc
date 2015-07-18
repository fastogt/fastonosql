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

#include <libmemcached/common.h>

/*
 * The udp request id consists of two seperate sections
 *   1) The thread id
 *   2) The message number
 * The thread id should only be set when the memcached_st struct is created
 * and should not be changed.
 *
 * The message num is incremented for each new message we send, this function
 * extracts the message number from message_id, increments it and then
 * writes the new value back into the header
 */
void increment_udp_message_id(memcached_instance_st* ptr)
{
  struct udp_datagram_header_st *header= (struct udp_datagram_header_st *)ptr->write_buffer;
  uint16_t cur_req= get_udp_datagram_request_id(header);
  int msg_num= get_msg_num_from_request_id(cur_req);
  int thread_id= get_thread_id_from_request_id(cur_req);

  if (((++msg_num) & UDP_REQUEST_ID_THREAD_MASK) != 0)
    msg_num= 0;

  header->request_id= htons((uint16_t) (thread_id | msg_num));
}

bool memcached_io_init_udp_header(memcached_instance_st* ptr, const uint16_t thread_id)
{
  if (thread_id > UDP_REQUEST_ID_MAX_THREAD_ID)
  {
    return MEMCACHED_FAILURE;
  }

  struct udp_datagram_header_st *header= (struct udp_datagram_header_st *)ptr->write_buffer;
  header->request_id= htons(uint16_t((generate_udp_request_thread_id(thread_id))));
  header->num_datagrams= htons(1);
  header->sequence_number= htons(0);

  return MEMCACHED_SUCCESS;
}
