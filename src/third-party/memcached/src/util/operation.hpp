/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  DataDifferential Utility Library
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


#include <cstring>
#include <iosfwd>
#include <vector>

namespace datadifferential {
namespace util {

class Operation {
  typedef std::vector<char> Packet;

public:
  typedef std::vector<Operation *> vector;

  Operation(const char *command, size_t command_length, bool expect_response= true) :
    _expect_response(expect_response),
    packet(),
    _response()
  {
    packet.resize(command_length);
    memcpy(&packet[0], command, command_length);
  }

  ~Operation()
  { }

  size_t size() const
  {
    return packet.size();
  }

  const char* ptr() const
  {
    return &(packet)[0];
  }

  bool has_response() const
  {
    return _expect_response;
  }

  void push(const char *buffer, size_t buffer_size)
  {
    size_t response_size= _response.size();
    _response.resize(response_size +buffer_size);
    memcpy(&_response[0] +response_size, buffer, buffer_size);
  }

  // Return false on error
  bool response(std::string &);

  bool reconnect() const
  {
    return false;
  }

private:
  bool _expect_response;
  Packet packet;
  Packet _response;
};

} /* namespace util */
} /* namespace datadifferential */
