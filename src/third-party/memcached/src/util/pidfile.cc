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

#include "mem_config.h"

#include "util/pidfile.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

extern "C" {

  char pid_file[1024 * 4]= { 0 };

  static void remove_pidfile(void)
  {
    if (pid_file[0])
    {
      if (unlink(pid_file) == -1)
      {
        std::cerr << "Could not remove pidfile: " << pid_file << "(" << strerror(errno) << ")" << std::endl;
      }

      pid_file[0]= 0;
    }
  }

}

namespace datadifferential {
namespace util {

Pidfile::Pidfile(const std::string &arg) :
  _last_errno(0),
  _filename(arg)
{
}


Pidfile::~Pidfile()
{
  if (not _filename.empty())
  {
    if (access(_filename.c_str(), F_OK) == -1)
    {
      std::stringstream error_stream;
      error_stream << "Could not access the pid file: " << _filename << "(" << strerror(errno) << ")";
      _error_message= error_stream.str();
    }
    else if (unlink(_filename.c_str()) == -1)
    {
      std::stringstream error_stream;
      error_stream << "Could not remove the pid file: " << _filename << "(" << strerror(errno) << ")";
      _error_message= error_stream.str();
    }
  }
  pid_file[0]= 0;
}

bool Pidfile::create()
{
  if (_filename.empty())
  {
    return true;
  }

  if (access(_filename.c_str(), F_OK) == 0)
  {
    if (unlink(_filename.c_str()) == -1)
    {
      std::stringstream error_stream;
      error_stream << "Unable to remove exisiting file:" << _filename << "(" << strerror(errno) << ")";
      _error_message= error_stream.str();

      return false;
    }
  }

  int oflags= O_CREAT|O_WRONLY|O_TRUNC;
#ifdef HAVE_O_CLOEXEC
  oflags= oflags | O_CLOEXEC;
#endif

  int file;
  if ((file = open(_filename.c_str(), oflags, S_IRWXU|S_IRGRP|S_IROTH)) < 0)
  {
    std::stringstream error_stream;
    error_stream << "Could not open pid file for writing: " << _filename << "(" << strerror(errno) << ")";
    _error_message= error_stream.str();
    
    return false;
  }

  char buffer[BUFSIZ];
  unsigned long temp= static_cast<unsigned long>(getpid());
  int length= snprintf(buffer, sizeof(buffer), "%lu\n", temp);
  if (write(file, buffer, length) != length)
  { 
    std::stringstream error_stream;
    error_stream << "Could not write pid to file: " << _filename << "(" << strerror(errno) << ")";
    _error_message= error_stream.str();
    close(file);

    return false;
  }

  if (close(file) < 0)
  {
    _error_message+= "Could not close() file after writing pid to it: "; 
    _error_message+= _filename;
    return false;
  }
  snprintf(pid_file, sizeof(pid_file), "%s", _filename.c_str());
  atexit(remove_pidfile);

  return true;
}

} /* namespace util */
} /* namespace datadifferential */
