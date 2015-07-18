/*    $Header: /cvsroot/wikipedia/willow/src/bin/willow/daemon.c,v 1.1 2005/05/02 19:15:21 kateturner Exp $    */
/*    $NetBSD: daemon.c,v 1.9 2003/08/07 16:42:46 agc Exp $    */
/*-
 * Copyright (c) 1990, 1993
 *    The Regents of the University of California.  All rights reserved.
 * Copyright (c) 2010
 *    Stewart Smith
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <mem_config.h>

#if defined __SUNPRO_C || defined __DECC || defined __HP_cc
# pragma ident "@(#)$Header: /cvsroot/wikipedia/willow/src/bin/willow/daemon.c,v 1.1 2005/05/02 19:15:21 kateturner Exp $"
# pragma ident "$NetBSD: daemon.c,v 1.9 2003/08/07 16:42:46 agc Exp $"
#endif

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <sys/select.h>

#include <util/daemon.hpp>

#include <iostream>

namespace datadifferential {
namespace util {

pid_t parent_pid;

extern "C"
{

static void sigusr1_handler(int sig)
{
  if (sig == SIGUSR1)
  {
    _exit(EXIT_SUCCESS);
  }
}

}

bool daemon_is_ready(bool close_io)
{
  if (kill(parent_pid, SIGUSR1) == -1)
  {
    perror("kill");
    return false;
  }

  if (close_io == false)
  {
    return true;;
  }

  int fd;
  if ((fd = open("/dev/null", O_RDWR, 0)) < 0)
  {
    perror("open");
    return false;
  }
  else
  {
    if (dup2(fd, STDIN_FILENO) < 0)
    {
      perror("dup2 stdin");
      return false;
    }

    if (dup2(fd, STDOUT_FILENO) < 0)
    {
      perror("dup2 stdout");
      return false;
    }

    if (dup2(fd, STDERR_FILENO) < 0)
    {
      perror("dup2 stderr");
      return false;
    }

    if (fd > STDERR_FILENO)
    {
      if (close(fd) < 0)
      {
        perror("close");
        return false;
      }
    }
  }

  return true;
}

#ifndef __INTEL_COMPILER
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

bool daemonize(bool is_chdir, bool wait_sigusr1)
{
  struct sigaction new_action;

  new_action.sa_handler= sigusr1_handler;
  sigemptyset(&new_action.sa_mask);
  new_action.sa_flags= 0;
  sigaction(SIGUSR1, &new_action, NULL);

  parent_pid= getpid();

  pid_t child= fork();

  switch (child)
  {
  case -1:
    return false;

  case 0:
    break;

  default:
    if (wait_sigusr1)
    {
      /* parent */
      int exit_code= EXIT_FAILURE;
      int status;
      while (waitpid(child, &status, 0) != child)
      { }

      if (WIFEXITED(status))
      {
        exit_code= WEXITSTATUS(status);
      }
      if (WIFSIGNALED(status))
      {
        exit_code= EXIT_FAILURE;
      }
      _exit(exit_code);
    }
    else
    {
      _exit(EXIT_SUCCESS);
    }
  }

  /* child */
  if (setsid() == -1)
  {
    perror("setsid");
    return false;
  }

  if (is_chdir)
  {
    if (chdir("/") < 0)
    {
      perror("chdir");
      return false;
    }
  }

  return true; 
}

} /* namespace util */
} /* namespace datadifferential */
