/* $RuOBSD$ */

#ifndef A386_UNIX_OS_SYSCALLS_H
#define A386_UNIX_OS_SYSCALLS_H

#include "arch/unistd.h"

#define __NR__unix_mmap		__NR_mmap
#define __NR_unix_close		__NR_close
#define __NR_unix_exit		__NR_exit
#define __NR_unix_fcntl		__NR_fcntl
#define __NR_unix_fork		__NR_fork
#define __NR_unix_getegid	__NR_getegid
#define __NR_unix_geteuid	__NR_geteuid
#define __NR_unix_getgid	__NR_getgid
#define __NR_unix_getitimer	__NR_getitimer
#define __NR_unix_getpgid	__NR_getpgid
#define __NR_unix_getpid	__NR_getpid
#define __NR_unix_gettimeofday	__NR_gettimeofday
#define __NR_unix_getuid	__NR_getuid
#define __NR_unix_ioctl		__NR_ioctl
#define __NR_unix_kill		__NR_kill
#define __NR_unix_lseek		__NR_lseek
#define __NR_unix_munmap	__NR_munmap
#define __NR_unix_nanosleep	__NR_nanosleep
#define __NR_unix_open		__NR_open
#define __NR_unix_pause		__NR_pause
#define __NR_unix_ptrace	__NR_ptrace
#define __NR_unix_read		__NR_read
#define __NR_unix_setitimer	__NR_setitimer
#define __NR_unix_settimeofday	__NR_settimeofday
#define __NR__unix_sigaction	__NR_rt_sigaction
#define __NR_unix_sigprocmask	__NR_sigprocmask
#define __NR_unix_sigsuspend	__NR_sigsuspend
#define __NR_unix_waitpid	__NR_waitpid
#define __NR_unix_write		__NR_write
#define __NR_unix_sigaltstack	__NR_sigaltstack

static inline _a386_syscall1 (int, unix_exit, int, code);
static inline _a386_syscall1 (int, unix_close, int, fd);
static inline _a386_syscall0 (int, unix_getegid);
static inline _a386_syscall0 (int, unix_geteuid);
static inline _a386_syscall0 (int, unix_getgid);
static inline _a386_syscall1 (int, unix_getpgid, int, pid);
static inline _a386_syscall0 (int, unix_getpid);
static inline _a386_syscall0 (int, unix_getuid);
static inline _a386_syscall2 (int, unix_kill, int, pid, int, sig);
static inline _a386_syscall1 (int, _unix_mmap, void *, args);
static inline _a386_syscall2 (int, unix_munmap, void *, address, int, length);
static inline _a386_syscall0 (int, unix_pause);
static inline _a386_syscall4 (int, unix_ptrace, int, request,
			      int, pid, void *, addr, int, data);
static inline _a386_syscall4 (int, _unix_sigaction, int, signum,
			      void *, action, void *, oldaction, int, s);
static inline _a386_syscall3 (int, unix_waitpid, int, pid,
			      int *, status, int, options);
static inline _a386_syscall3 (int, unix_lseek, int, fd,
			      int, offset, int, whence);
static inline _a386_syscall2 (int, unix_open, const char *, file, int, mode);
static inline _a386_syscall3 (int, unix_read, int, fd, void *, buf, int, len);
static inline _a386_syscall3 (int, unix_write, int, fd, void *, buf, int, len);
static inline _a386_syscall3 (int, unix_setitimer, int, which,
			      void *, new, void *, old);
static inline _a386_syscall2 (int, unix_getitimer, int, which,
			      void *, old);
static inline _a386_syscall2 (int, unix_gettimeofday, void *, tv,
			      void *, zone);
static inline _a386_syscall2 (int, unix_settimeofday, void *, tv,
			      void *, zone);
static inline _a386_syscall1 (int, unix_sigsuspend, void *, set);
static inline _a386_syscall3 (int, unix_fcntl, int, fd, int, cmd, long, arg);
static inline _a386_syscall3 (int, unix_ioctl, int, fd, int, cmd, void *, arg);
static inline _a386_syscall2 (int, unix_nanosleep, void *, req, void *, rem);
static inline _a386_syscall3 (int, unix_sigprocmask, int, how, void *, new,
			      void *, old);
static inline _a386_syscall0 (int, unix_fork);
static inline _a386_syscall2 (int, unix_sigaltstack, void *, a, void *, b);

typedef struct
{
  /* this must match the kernel's idea of a stack_t */
  void *ss_sp;
  int ss_flags;
  unsigned int ss_size;
} unix_stack_t;

typedef struct
{
  int x[2];
} unix_sigset_t;

#undef sa_handler
#undef sa_sigaction

struct unix_sigaction
{
#define sa_sigaction sa_handler
  void *sa_handler;
  unsigned long sa_flags;
  void (*sa_restorer) (void);
  unix_sigset_t sa_mask;
};

static inline int unix_sigaction (int sig, void *act, void *oact)
{
  int result = _unix_sigaction (sig, act, oact, sizeof (unix_sigset_t));
  if (result >= 0)
    return result;
  unix_errno = -result;
  return -1;
}

static inline int unix_mmap (void *address, int length, int protection,
			     int flags, int fd, int offset)
{
  struct 
  {
    void *address;
    int length;
    int protection;
    int flags;
    int fd;
    int offset;
  } args = { address, length, protection, flags, fd, offset };
  extern void a386_printf (const char *, ...);
  long result;
a386_printf ("\nunix_mmap (%p, %d, %x, %x, %d, %d)", address, length, protection, flags, fd, offset);
  result = _unix_mmap (&args);
a386_printf ("\nunix_mmap = %d = %x", result, result);
  if ((unsigned long)result < (unsigned long)(-125))
    return result;
  unix_errno = -result;
a386_printf ("\nunix_mmap: error %d", -result);
  return -1;
}

static inline int
unix_sigemptyset (void *x)
{
  int *y = x;
  int i;

  for (i = 0; i < 2; i++)
    *y++ = 0;

  return 0;
}

static inline int
unix_sigfillset (void *x)
{
  int *y = x;
  int i;

  for (i = 0; i < 2; i++)
    *y++ = -1;

  return 0;
}

static inline int
unix_sigaddset (void *x, int n)
{
  int *y = x;

  if (n >= 64)
    return 0;

  while (n >= 32)
    {
      n -= 32;
      y++;
    }

  *y |= 1 << n;

  return 0;
}

static inline int
unix_sigdelset (void *x, int n)
{
  int *y = x;

  if (n >= 64)
    return 0;

  while (n >= 32)
    {
      n -= 32;
      y++;
    }

  *y &= ~(1 << n);

  return 0;
}

#endif
