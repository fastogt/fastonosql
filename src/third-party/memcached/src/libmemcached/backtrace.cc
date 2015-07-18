/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Libmemcached client library.
 *
 *  Copyright (C) 2012 Data Differential, http://datadifferential.com/
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

#include "libmemcached/backtrace.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#if defined(HAVE_SHARED_ENABLED) && HAVE_SHARED_ENABLED

#ifdef HAVE_EXECINFO_H
#include <execinfo.h>
#endif

#ifdef HAVE_GCC_ABI_DEMANGLE
# include <cxxabi.h>
# define USE_DEMANGLE 1
#else
# define USE_DEMANGLE 0
#endif

#ifdef HAVE_DLFCN_H
# include <dlfcn.h>
#endif   

const int MAX_DEPTH= 50;

void custom_backtrace(void)
{
#ifdef HAVE_EXECINFO_H
  void *backtrace_buffer[MAX_DEPTH +1];

  int stack_frames= backtrace(backtrace_buffer, MAX_DEPTH);
  if (stack_frames)
  {
    char **symbollist= backtrace_symbols(backtrace_buffer, stack_frames);
    if (symbollist)
    {
      for (int x= 0; x < stack_frames; x++) 
      {
        bool was_demangled= false;

        if (USE_DEMANGLE)
        {
#ifdef HAVE_DLFCN_H
          Dl_info dlinfo;
          if (dladdr(backtrace_buffer[x], &dlinfo))
          {
            char demangled_buffer[1024];
            const char *called_in= "<unresolved>";
            if (dlinfo.dli_sname)
            {
              size_t demangled_size= sizeof(demangled_buffer);
              int status;
              char* demangled;
              if ((demangled= abi::__cxa_demangle(dlinfo.dli_sname, demangled_buffer, &demangled_size, &status)))
              {
                called_in= demangled;
                fprintf(stderr, "---> demangled: %s -> %s\n", demangled_buffer, demangled);
              }
              else
              {
                called_in= dlinfo.dli_sname;
              }

              was_demangled= true;
              fprintf(stderr, "#%d  %p in %s at %s\n",
                      x, backtrace_buffer[x],
                      called_in,
                      dlinfo.dli_fname);
            }
          }
#endif
        }

        if (was_demangled == false)
        {
          fprintf(stderr, "?%d  %p in %s\n", x, backtrace_buffer[x], symbollist[x]);
        }
      }

      ::free(symbollist);
    }
  }
#endif // HAVE_EXECINFO_H
}

#else // HAVE_SHARED_ENABLED

void custom_backtrace(void)
{
  fprintf(stderr, "Backtrace null function called\n");
}
#endif // AX_ENABLE_BACKTRACE
