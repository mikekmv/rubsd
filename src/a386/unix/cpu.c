/**********************************************************************
unix/cpu.c

Copyright (C) 1999-2000 Lars Brinkhoff.  See the file COPYING for
licensing terms and conditions.

This file implements CPU operations.

EXTERNAL INTERFACE

a386_init()			Initialize a386.
a386_load()			Load ELF kernel into physical memory.
a386_enable_kernel_mode()	Enable kernel mode.
a386_enable_user_mode()		Enable user mode.
a386_in_kernel_mode()		In kernel mode?
a386_in_user_mode()		In user mode?
a386_enable_trace_mode()	Enable trace mode.
a386_disable_trace_mode()	Disable trace mode.
a386_trace_mode_enabled()	Trace mode enabled?
a386_halt()			Halt processor.
a386_cpu_khz()			Determine CPU speed in kHz.
a386_get_usp()
a386_set_usp()
a386_panic()			Print a message and abort execution.
a386_get_arg()			Get syscall argument.
a386_set_arg()			Set syscall argument.

INTERNAL INTERFACE

_a386_in_kernel_mode		In kernel mode?
**********************************************************************/

#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ptrace.h>

#include "_a386.h"
#include "sysdep.h"
#include "sysdep/os/syscalls.h"

#define DEBUG_ON a386_debug_cpu
#define DEBUG_NAME "cpu"
#define DEBUG_DEFAULT 2
#include "debug_stub.h"

int _a386_trace_mode_enabled = 0;
int _a386_in_kernel_mode __attribute__ ((section (".udata"))) = 1;
int a386_old_kernel_mode;
int _a386_pid;
long _a386_regs[17];

a386_word_t a386_syscall_number __attribute__ ((section (".udata"))) = 0;
a386_arg_t a386_syscall_arg[5];
a386_address_t _a386_usp __attribute__ ((section (".udata"))) = 0;
a386_address_t _a386_ksp __attribute__ ((section (".udata"))) = 0;

char old_terminal_state[SIZEOF_TERMINAL_STATE];

const char *
a386_cpu_name (void)
{
  return CPU_NAME;
}

const char *
a386_version (void)
{
  return "0.0";
}

a386_address_t
a386_get_usp (void)
{
  _a386_assert_kernel_mode_return (a386_address_t);
  return _a386_usp;
}

void
a386_set_usp (a386_address_t usp)
{
  _a386_assert_kernel_mode ();
  _a386_usp = usp;
}

int
a386_in_kernel_mode (void)
{
  _a386_assert_kernel_mode_return (int);
  return _a386_in_kernel_mode;
}

int
a386_in_user_mode (void)
{
  _a386_assert_kernel_mode_return (int);
  return !_a386_in_kernel_mode;
}

void
_a386_switch_to_user_mode (void)
{
  unix_kill (unix_getpid (), SIGUSR1); /* FIXME: SIGUSR1 */ /* see catch_syscall() */
}

void
a386_enable_trace_mode (void)
{
  _a386_assert_kernel_mode ();
  _a386_trace_mode_enabled = 1;
}

void
a386_disable_trace_mode (void)
{
  _a386_assert_kernel_mode ();
  _a386_trace_mode_enabled = 0;
}

int
a386_trace_mode_enabled (void)
{
  _a386_assert_kernel_mode_return (int);
  return _a386_trace_mode_enabled;
}

void
a386_halt (void)
{
  _a386_assert_kernel_mode ();
  _a386_terminal_restore (0, old_terminal_state);
  unix_exit (0);
}

void
a386_panic (const char *message)
{
  long *sp;
  int i;

  a386_puts ("\n\na386 panic: ");
  a386_puts (message);
  a386_puts ("\n");

  a386_puts ("\nstack:");
  sp = (long *)a386_get_sp ();
  for (i = 0; i < 10 * 7; i++)
    {
      if (i % 7 == 0)
	a386_puts ("\n");
      a386_printf ("%08x ", sp[i]);
    }

  _a386_terminal_restore (0, old_terminal_state);
  unix_exit (9);
}

static int
find_cpu_speed (int fd, char *ch)
{
  static char *cpu_MHz = "cpu MHz";
  int strlen_cpu_MHz = 7;
  char c;
  int i, n;

  i = 0;
  while (i < strlen_cpu_MHz)
    {
      n = unix_read (fd, &c, 1);
      if (n <= 0)
	{
	  log_error ("couldn't find CPU MHz");
	  return -1;
	}

      if (c == cpu_MHz[i])
	i++;
      else
	i = 0;
    }

  do
    n = unix_read (fd, &c, 1);
  while (n > 0 && (c == ' ' || c == '\t' || c == ':'));

  if (n <= 0)
    return -1;

  *ch = c;
  return 1;
}

static int
parse_int (int fd, char *ch, int *i)
{
  int m, number;
  char c;
  int n;

  c = *ch;
  m = 1;
  n = 0;
  number = 0;
  while (m > 0 && c >= '0' && c <= '9')
    {
      n++;
      number = 10 * number + c - '0';
      m = unix_read (fd, &c, 1);
    }

  if (m < 0)
    return m;

  *i = number;
  *ch = c;
  return n;
}

static int
parse_cpu_khz (int fd)
{
  int n, a, b;
  char c;

  n = find_cpu_speed (fd, &c);
  if (n == -1)
    return -1;

  n = parse_int (fd, &c, &a);
  if (n <= 0)
    return -1;		/* no number found */
  a *= 1000;
  if (c != '.')
    return a;		/* no '.' after number */

  n = unix_read (fd, &c, 1);
  if (n <= 0)
    return a;

  n = parse_int (fd, &c, &b);
  if (n <= 0)
    return a;		/* no number found after '.' */

  while (n > 3)
    {
      n--;
      b = (b + 5) / 10;
    }

  return a + b;
}

int
a386_cpu_khz (void)
{
  int fd, khz;

  fd = unix_open ("/proc/cpuinfo", O_RDONLY);
  if (fd == -1)
    {
      log_error ("couldn't open /proc/cpuinfo");
      return -1;
    }

  khz = parse_cpu_khz (fd);

  unix_close (fd);
  debug ("khz = %d", khz);
  return khz;
}

a386_arg_t
a386_get_arg (int n)
{
  return a386_syscall_arg[n];
}

void
a386_set_arg (int n, a386_arg_t arg)
{
  a386_syscall_arg[n] = arg;
}

static a386_result_t
_a386_trap_wrapper (void)
{
  _a386_trap_no_save (a386_syscall_number);
  return a386_get_result ();
}

void
catch_syscall (int child)
{
  int status;

  for (;;)
    {
      unix_waitpid (child, &status, WUNTRACED);

      GETREGS (child, _a386_regs);

if ((unsigned)_a386_regs[10] < 0x08000000U || (unsigned)_a386_regs[13] < 0x90000000U)
{
  int i;
  long *r = (long *)_a386_regs;
  static char *regname[] =
  { "ebx", "ecx", "edx", "esi", "edi", "ebp", "eax", "xds",
    "xes", "orig_eax", "eip", "xcs", "eflags", "esp", "xss",
    "kernel_mode", "interrupts_enabled" };
  a386_printf ("\ncatch_syscall: register dump:");
  for (i = 0; i < 17; i++)
    a386_printf ("\n%s = %08lx", regname[i], r[i]);
}
      {
	int i;
	int *x = (int *)_a386_regs;
	for (i = 0; i < 17; i++)
	  {
	    unix_ptrace (PTRACE_POKEDATA, child, &_a386_regs[i], *x++);
	  }
      }

      if (WIFSTOPPED (status))
	{
	  if (WSTOPSIG (status) == SIGTRAP)
	    return;
	  else if (WSTOPSIG (status) == SIGUSR1) /*see _a386_set_user_mode()*/
	    {
	      unix_ptrace (PTRACE_SYSCALL, child, 0, 0);
	    }
	  else
	    {
switch (WSTOPSIG (status))
  {
  case SIGVTALRM:
  case SIGSEGV:
    break;
  default:
    a386_printf ("\ncatch_syscall: signal %d", WSTOPSIG (status));
  }
              /* FIXME: don't set for SIG_DFL, SIG_IGN signal handlers? */
	      unix_ptrace (PTRACE_CONT, child, 0, WSTOPSIG (status));
	      /* child now in kernel mode */
	    }
	}
      else if (WIFEXITED (status))
	{
	  a386_printf ("\ncatch_syscall(): child exited with status %d\n",
			WEXITSTATUS (status));
	  unix_exit (0);
	}
      else if (WIFSIGNALED (status))
	{
	  a386_printf ("\ncatch_syscall(): child terminated with signal %d\n",
			WTERMSIG (status));
	  /* FIXME: dump registers, stack, etc */
	  {
	    int i;
	    long *r = (long *)_a386_regs;
	    static char *regname[] =
	    { "ebx", "ecx", "edx", "esi", "edi", "ebp", "eax", "xds",
	      "xes", "orig_eax", "eip", "xcs", "eflags", "esp", "xss",
	      "kernel_mode", "interrupts_enabled" };
	    a386_puts ("\ncatch_syscall: registers:");
	    for (i = 0; i < 17; i++)
	      a386_printf ("\n%s = %08lx", regname[i], r[i]);
	  }
	  unix_exit (1);
	}
      else
	{
	  a386_puts ("\ncatch_syscall(): child stopped for uknown reason\n");
	  unix_exit (1);
	}
    }
}

void
redirect_syscall (int child)
{
  int in_kernel_mode;
  int old_syscall;
  int status;

  /* FIXME: maybe use _a386_in_kernel_mode? */
  unix_ptrace (PTRACE_PEEKDATA,
	       child,
	       &_a386_in_kernel_mode,
	       (int)&in_kernel_mode);
  if (in_kernel_mode)
    {
int n;
unix_ptrace (PTRACE_PEEKUSER, child, (void *)(4*11) , (int)&n);
a386_printf ("redirect_syscall: kernel syscall %d => PTRACE_CONT\n", n);
      unix_ptrace (PTRACE_CONT, child, 0, 0);
      return;
    }

  _a386_get_syscall_arguments (child, a386_syscall_arg);

  old_syscall = _a386_cancel_syscall (child);
  if (old_syscall == -1)
    return;

  unix_ptrace (PTRACE_SYSCALL, child, 0, 0);

  unix_waitpid (child, &status, WUNTRACED);
  unix_ptrace (PTRACE_POKEDATA, child, &a386_syscall_number, old_syscall);
  _a386_jump_to_address (child, _a386_trap_wrapper);
  unix_ptrace (PTRACE_CONT, child, 0, 0);
  /* child now in kernel mode */
}

void
redirect_syscalls (void)
{
  int child;

  child = unix_fork ();
  if (child)
    {
      for (;;)
	{
	  catch_syscall (child);
	  redirect_syscall (child);
	}
    }
}

void
a386_init (void)
{
  _a386_terminal_save (0, old_terminal_state);
  _a386_pid = unix_getpid ();

  _a386_in_kernel_mode = 1;
  _a386_memory_init ();
  _a386_mmu_init ();
  _a386_interrupt_init ();
  _a386_device_init ();

  redirect_syscalls ();
  unix_ptrace (PTRACE_TRACEME, 0, 0, 0);
}
