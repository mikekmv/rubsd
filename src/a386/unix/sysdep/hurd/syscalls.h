#ifndef A386_UNIX_OS_SYSCALLS_H
#define A386_UNIX_OS_SYSCALLS_H

#include "arch/unistd.h"
#include "/include/unistd.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>




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

static int (*unix_exit)(int code)=(void *)exit;
static int (*unix_close)(int fd)=(void *)close;
static int (*unix_getegid)()=(void *)getgid;
static int (*unix_geteuid)()=(void *)geteuid;
static int (*unix_getgid)()=(void *)getgid;
static int (*unix_getpgid)(int pid)=(void *)getpgid;
static int (*unix_getpid)()=(void *)getpid;
static int (*unix_getuid)()=(void *)getuid;
static int (*unix_kill)(int pid, int sig)=(void *)kill;
static int (*_unix_mmap)(void  *start, size_t length,
			int prot, int flags, int fd, off_t offset)
			=(void *)mmap;
static int (*unix_munmap)(void * address, int length)=(void *)munmap;
static int (*unix_pause)()=(void *)pause;
static int (*unix_ptrace)(int request, int pid,
			void * addr, int data)
			=(void *)ptrace;
static int (*_unix_sigaction)(int signum, void * action,
			void * oldaction)
			=(void *)sigaction;
static int (*unix_waitpid)(int pid, int * status, int options)=(void *)waitpid;
static int (*unix_lseek)(int fd, int offset, int whence)=(void *)lseek;
static int (*unix_open)(const char * file, int mode)=(void *)open;
static int (*unix_read)(int fd, void * buf, int len)=(void *)read;
static int (*unix_write)(int fd, void * buf, int len)=(void *)write;
static int (*unix_setitimer)(int which, void * new, void * old)
			=(void *)settimer;
static int (*unix_getitimer)(int which, void * old)=(void *)gettimer;
static int (*unix_gettimeofday)(void * tv, void * zone)=(void *)gettimeofday;
static int (*unix_settimeofday)(void * tv, void * zone)=(void *)settimeofday;
static int (*unix_sigsuspend)(void * set)=(void *)sigsuspend;
static int (*unix_fcntl)(int fd, int cmd, long arg)=(void *)fcntl;
static int (*unix_ioctl)(int fd, int cmd, long arg)=(void *)ioctl;
static int (*unix_nanosleep)(void * req, void * rem)=(void *)nanosleep;
static int (*unix_sigprocmask)(int how, void * new,  void * old)
			=(void *)sigprocmask;
static int (*unix_fork)()=(void *)fork;
//static int (*unix_sigaltstack)(void * a, void * b)=(void *)sigaltstack;

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
//  int result = _unix_sigaction (sig, act, oact, sizeof (unix_sigset_t));
  int result = _unix_sigaction (sig, act, oact);
  if (result >= 0)
    return result;
  unix_errno = -result;
  return -1;
}

static inline int unix_mmap (void *address, int length, int protection,
			     int flags, int fd, int offset)
{
//  extern void a386_printf (const char *, ...);
  long result;
a386_printf ("\nunix_mmap (%p, %d, %x, %x, %d, %d)", address, length, protection, flags, fd, offset);
  result = _unix_mmap (address, length, protection, flags, fd, offset);
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
