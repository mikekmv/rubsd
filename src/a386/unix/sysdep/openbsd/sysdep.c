/*	$RuOBSD$	*/

/*
 * Copyright (c) 2003 Maxim Tsyplakov <tm@openbsd.ru>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**********************************************************************
unix/os/openbsd.c

Operating system-dependant functions.

INTERNAL INTERFACE

void _a386_call_function (int CHILD, void *function);
	Prepare for CHILD, which must be traced by the caller, to call
	FUNCTION when execution is resumed.

void _a386_jump_to_address (int CHILD, void *FUNCTION);
	Prepare for CHILD, which must be traced by the caller, to jump
	to ADDRESS when execution is resumed.

long _a386_cancel_syscall (int CHILD);
	When CHILD, which must be traced using PTRACE_SYSCALL, resumes
	execution, the system call will not be made.  Returns the number
	of the system call that was cancelled.
	
void _a386_get_syscall_arguments (int CHILD, a386_arg_t *ARG);
	Fetch the system call arguments from the context of the CHILD,
	and store them in the ARG array.

void _a386_terminal_save (int fd, void *state);
	Save the state of the terminal device referred to by FD in
	the memory pointed to by STATE.

void _a386_terminal_restore (int fd, void *state);
	Restore the state of the terminal device referred to by FD from
	the memory pointed to by STATE.
**********************************************************************/

#include <unistd.h>
#include <termios.h>
#include <sys/wait.h>
#include <sys/ioctl.h> /* TCGET/SETS */
#include <linux/ptrace.h>

#include <a386/a386_params.h>
#include <a386/a386_types.h>
#include <a386/a386_debug.h>

#include "sysdep/linux/syscalls.h"
#include "sysdep/linux/arch/arch.h"
#include "sysdep/linux/arch/unistd.h"

#define PTRACE_PEEKUSER PTRACE_PEEKUSR
#define PTRACE_POKEUSER PTRACE_POKEUSR

int unix_errno;

void
_a386_call_function (int child, void *function)
{
  long ip;
  unix_ptrace (PTRACE_PEEKUSER, child, REG_IP, (int)&ip);
  SET_RETURN_ADDRESS (child, ip);
  unix_ptrace (PTRACE_POKEUSER, child, REG_IP, (long)function);
}

void
_a386_jump_to_address (int child, void *address)
{
  unix_ptrace (PTRACE_POKEUSER, child, REG_IP, (long)address);
}

long
_a386_cancel_syscall (int pid)
{
  long n;

  GET_SYSCALL_NUMBER (pid, n);
  if (n == __NR_sigreturn || n == __NR_rt_sigreturn)
    {
      int status;
      unix_ptrace (PTRACE_SYSCALL, pid, 0, 0);
      unix_waitpid (pid, &status, WUNTRACED);
      unix_ptrace (PTRACE_SYSCALL, pid, 0, 0);
      return -1;
    } 

#if 1
  {
    long x;

    x = unix_ptrace (PTRACE_POKEUSER, pid, REG_SYSCALL_NUBMER, __NR_getpid);
    if (x < 0)
      {
	a386_printf ("failed to redirect system call\n");
	a386_printf ("x = %d, errno = %d\n", x, unix_errno);
	unix_exit (1);
      }
  }
#else
  {
    long arg1, arg2, arg3, arg4, arg5;
    static char empty_string = 0;

    unix_ptrace (PTRACE_PEEKUSER, pid, REG_ARG_1, (int)&arg1);
    unix_ptrace (PTRACE_PEEKUSER, pid, REG_ARG_2, (int)&arg2);
    unix_ptrace (PTRACE_PEEKUSER, pid, REG_ARG_3, (int)&arg3);
    unix_ptrace (PTRACE_PEEKUSER, pid, REG_ARG_4, (int)&arg4);
    unix_ptrace (PTRACE_PEEKUSER, pid, REG_ARG_5, (int)&arg5);

    switch (n)
      {
      case __NR_exit:
      case __NR_fork:
      case __NR_pause:
      case __NR_setsid:
      case __NR_vfork:
	/* FIXME */
	a386_printf ("don't know how to cancel syscall %d=%x\n", n, n);
	break;
      case __NR_umask:
	/* FIXME: cancel by setting same value */
	break;
      case __NR_sync:
	a386_puts ("sync\n");
	break;
      case __NR_close:
      case __NR_dup2:
      case __NR_dup:
      case __NR_fchdir:
      case __NR_fchmod:
      case __NR_fchown:
      case __NR_fcntl:
      case __NR_fdatasync:
      case __NR_flock:
      case __NR_fstat:
      case __NR_fstatfs:
      case __NR_fsync:
      case __NR_ftruncate:
      case __NR_ioctl:
      case __NR_lseek:
      case __NR_oldfstat:
      case __NR_read:
      case __NR_readdir:
      case __NR_readv:
      case __NR_sendfile:
      case __NR_write:
      case __NR_writev:
	arg1 = -1; /* set file descriptor to -1 */
	break;
      case __NR_access:
      case __NR_chdir:
      case __NR_chmod:
      case __NR_chown:
      case __NR_chroot:
      case __NR_creat:
      case __NR_execve:
      case __NR_lchown:
      case __NR_link:
      case __NR_lstat:
      case __NR_mkdir:
      case __NR_mknod:
      case __NR_mount:
      case __NR_oldlstat:
      case __NR_oldstat:
      case __NR_open:
      case __NR_readlink:
      case __NR_rename:
      case __NR_rmdir:
      case __NR_stat:
      case __NR_statfs:
      case __NR_swapoff:
      case __NR_swapon:
      case __NR_symlink:
      case __NR_truncate:
      case __NR_umount2:
      case __NR_umount:
      case __NR_unlink:
      case __NR_uselib:
      case __NR_utime:
      case __NR_acct:
	arg1 = (long)&empty_string; /* set file name pointer to empty string */
	break;
      case __NR__sysctl:
      case __NR_clone:
      case __NR_nanosleep:
      case __NR_oldolduname:
      case __NR_olduname:
      case __NR_pipe:
      case __NR_sethostname:
      case __NR_sigpending:
      case __NR_sigsuspend:
      case __NR_stime:
      case __NR_sysinfo:
      case __NR_time:
      case __NR_times:
      case __NR_uname:
	arg1 = 0; /* set struct pointer to NULL */
	break;
      case __NR_ustat:
	arg2 = 0; /* set struct pointer to NULL */
	break;
      case __NR_afs_syscall:
      case __NR_break:
      case __NR_ftime:
      case __NR_gtty:
      case __NR_mpx:
      case __NR_profil:
      case __NR_stty:
      case __NR_ulimit:
      case __NR_prof:
      case __NR_lock:
	/* unimplemented syscalls */
	break;
      case __NR_getegid:
      case __NR_geteuid:
      case __NR_getgid:
      case __NR_getpgid:
      case __NR_getpgrp:
      case __NR_getpid:
      case __NR_getppid:
      case __NR_getresgid:
      case __NR_getresuid:
      case __NR_getuid:
      case __NR_getsid:
	/* no side effects */
	break;
      case __NR_getgroups:
      case __NR_setgroups:
	arg1 = 0; /* array size */
	break;
      case __NR_waitpid:
      case __NR_wait4:
	arg1 = unix_getpid (); /* set pid to self */
	break;
      case __NR_ptrace:
	arg2 = unix_getpid (); /* set pid to self */
	break;
      case __NR_kill:
	arg2 = -1; /* set signal number to -1 */
	break;
      case __NR_signal:
      case __NR_sigaction:
	arg1 = -1; /* set signal number to -1 */
	break;
      case __NR_brk:
	arg1 = 0; /* set break to 0 */
	break;
      case __NR_select:
      case __NR__newselect:
	arg1 = 0; /* set n = 0 */
	break;
      case __NR_poll:
	arg2 = 0; /* set n = 0 */
	break;
	break;
      case __NR_setuid:
	arg1 = unix_getuid (); /* FIXME: from child */
	break;
      case __NR_setreuid:
	arg1 = unix_geteuid (); /* FIXME: from child */
	break;
      case __NR_setgid:
	arg1 = unix_getgid (); /* FIXME: from child */
	break;
      case __NR_setregid:
	arg1 = unix_getegid (); /* FIXME: from child */
	break;
      case __NR_setpgid:
	arg1 = unix_getpgid (pid);
	break;
      case __NR_mlock:
      case __NR_mmap:
      case __NR_mprotect:
      case __NR_msync:
      case __NR_munlock:
      case __NR_munmap:
      case __NR_getcwd:
	arg2 = 0; /* set length = 0 */
	break;
      case __NR_socketcall:
	arg1 = -1; /* set call number to -1 */
	break;
      case __NR_sigreturn:
	/* do absolutely nothing */
	break;
      case __NR_syslog:
	arg3 = 0; /* set length to 0 */
	break;
      case __NR_idle:
	/* nothing needed, only process 0 may call */
	break;
      case __NR_alarm:
      case __NR_nice:
	arg1 = 0;
	break;
      case __NR_setrlimit:
      case __NR_getrlimit:
      case __NR_getrusage:
      case __NR_setdomainname:
	arg1 = 0; /* set struct pointer to NULL */
	break;
      case __NR_gettimeofday:
      case __NR_settimeofday:
	arg1 = arg2 = 0; /* set struct pointers to NULL */
	break;
      case __NR_reboot:
	arg1 = 0; /* set magic = 0 */
	break;
      case __NR_getpriority:
      case __NR_setpriority:
	arg1 = 42; /* set which argument to invalid value (FIXME) */
	break;
      case __NR_setitimer:
      case __NR_getitimer:
      case __NR_sigprocmask:
	arg2 = arg3 = 0; /* set struct pointers to NULL */
	break;
      case __NR_ipc:
      case __NR_sysfs:
      case __NR_quotactl:
      case __NR_personality:
	arg1 = 424242; /* set call number to hopefully(FIXME) invalid value */
	break;
      case __NR_sgetmask:
      case __NR_ssetmask:
      case __NR_ioperm:
      case __NR_iopl:
      case __NR_vhangup:
      case __NR_vm86old:
      case __NR_modify_ldt:
      case __NR_adjtimex:
      case __NR_create_module:
      case __NR_init_module:
      case __NR_delete_module:
      case __NR_get_kernel_syms:
      case __NR_bdflush:
      case __NR_setfsuid:
      case __NR_setfsgid:
      case __NR__llseek:
      case __NR_getdents:
      case __NR_mlockall:
      case __NR_munlockall:
      case __NR_sched_setparam:
      case __NR_sched_getparam:
      case __NR_sched_setscheduler:
      case __NR_sched_getscheduler:
      case __NR_sched_yield:
      case __NR_sched_get_priority_max:
      case __NR_sched_get_priority_min:
      case __NR_sched_rr_get_interval:
      case __NR_mremap:
      case __NR_setresuid:
      case __NR_vm86:
      case __NR_query_module:
      case __NR_nfsservctl:
      case __NR_setresgid:
      case __NR_prctl:
      case __NR_rt_sigreturn:
      case __NR_rt_sigaction:
      case __NR_rt_sigprocmask:
      case __NR_rt_sigpending:
      case __NR_rt_sigtimedwait:
      case __NR_rt_sigqueueinfo:
      case __NR_rt_sigsuspend:
      case __NR_pread:
      case __NR_pwrite:
      case __NR_capget:
      case __NR_capset:
      case __NR_sigaltstack:
      case __NR_getpmsg:
      case __NR_putpmsg:
	/* FIXME */
	a386_printf ("cancellation unimplemented for syscall %d=%x\n", n, n);
	break;
      default:
	a386_printf ("unknown syscall %d=%x\n", n, n);
      }

    unix_ptrace (PTRACE_POKEUSER, pid, REG_ARG_1, arg1);
    unix_ptrace (PTRACE_POKEUSER, pid, REG_ARG_2, arg2);
    unix_ptrace (PTRACE_POKEUSER, pid, REG_ARG_3, arg3);
    unix_ptrace (PTRACE_POKEUSER, pid, REG_ARG_4, arg4);
    unix_ptrace (PTRACE_POKEUSER, pid, REG_ARG_5, arg5);
  }
#endif

  return n;
}

void
_a386_get_syscall_arguments (int pid, a386_arg_t *arg)
{
  long arg1, arg2, arg3, arg4, arg5;

  unix_ptrace (PTRACE_PEEKUSER, pid, REG_ARG_1, (int)&arg1);
  unix_ptrace (PTRACE_PEEKUSER, pid, REG_ARG_2, (int)&arg2);
  unix_ptrace (PTRACE_PEEKUSER, pid, REG_ARG_3, (int)&arg3);
  unix_ptrace (PTRACE_PEEKUSER, pid, REG_ARG_4, (int)&arg4);
  unix_ptrace (PTRACE_PEEKUSER, pid, REG_ARG_5, (int)&arg5);

  unix_ptrace (PTRACE_POKEDATA, pid, &arg[0], arg1);
  unix_ptrace (PTRACE_POKEDATA, pid, &arg[1], arg2);
  unix_ptrace (PTRACE_POKEDATA, pid, &arg[2], arg3);
  unix_ptrace (PTRACE_POKEDATA, pid, &arg[3], arg4);
  unix_ptrace (PTRACE_POKEDATA, pid, &arg[4], arg5);
}

void
_a386_terminal_save (int fd, void *state)
{
  unix_ioctl (0, TCGETS, state);
}

void
_a386_terminal_raw (int fd, void *state)
{
  struct termios termios;

  unix_ioctl (fd, TCGETS, &termios);
  termios.c_iflag = 0;
  termios.c_oflag = 0;
  termios.c_cflag = 0;
  termios.c_lflag = 0;
  unix_ioctl (fd, TCSETS, &termios);
}

void
_a386_terminal_restore (int fd, void *state)
{
  unix_ioctl (0, TCSETS, state);
}
