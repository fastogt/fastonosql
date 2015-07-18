/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 *
 *  Data Differential Utility library
 *
 *  Copyright (C) 2012 Data Differential, http://datadifferential.com/
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

#include <pthread.h>
#include <semaphore.h>

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#ifdef	__cplusplus
extern "C" {
#endif

typedef void (signal_callback_fn)();

#ifdef	__cplusplus
}
#endif

namespace datadifferential {
namespace util {

enum shutdown_t {
  SHUTDOWN_RUNNING,
  SHUTDOWN_GRACEFUL,
  SHUTDOWN_FORCED
};

class SignalThread {
  bool _exit_on_signal;
  sigset_t set;
  sem_t lock;
  uint64_t magic_memory;
  volatile shutdown_t __shutdown;
  pthread_mutex_t shutdown_mutex;

public:

  SignalThread(bool exit_on_signal_arg= false);

  void test();
  void post();
  bool setup();

  bool exit_on_signal()
  {
    return _exit_on_signal;
  }

  int wait(int& sig)
  {
    return sigwait(&set, &sig);
  }

  ~SignalThread();

  void set_shutdown(shutdown_t arg);
  bool is_shutdown();
  shutdown_t get_shutdown();

  void sighup();
  void sighup(signal_callback_fn* arg);

private:
  pthread_t thread;
  signal_callback_fn* _sighup;
};

} /* namespace util */
} /* namespace datadifferential */
