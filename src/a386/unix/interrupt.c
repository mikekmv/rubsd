/**********************************************************************
unix/interrupt.c

Copyright (C) 1999-2000 Lars Brinkhoff.  See the file COPYING for
licensing terms and conditions.

EXTERNAL INTERFACE

a386_interrupt_wait()			Wait for interrupt.
a386_interrupts_enable()		Enable interrupts.
a386_interrupts_disable()		Disable interrupts.
a386_interrupts_enabled()		Are interrupts enabled?
a386_set_vectors()
a386_get_vectors()

INTERNAL INTERFACE

_a386_vectors				Vector base register.
_a386_interrupt_init()
_a386_interrupts_enabled
_a386_device_interrupt()		Generate a device interrupt.
**********************************************************************/

#include <errno.h>
#include <signal.h>
#include <sys/time.h>

#include "_a386.h"
#include "sysdep/os/syscalls.h"

#if 0 /* still useless in Linux 2.3.22 */
#  define USE_SIGINFO
#endif

#ifdef i386
#  define REG_ADDR cr2
#  define REG_REASON err
#  define REG_SP esp_at_signal
#endif
#ifdef arm
   /* need special patch */
#  define REG_ADDR address
#  define REG_REASON error_code
#  define REG_SP arm_sp
#endif
#ifdef __sparc__
/* I can't find the appropriate variables to use here. */
#  define REG_ADDR sigc_pc
#  define REG_REASON sigc_g1
#  define REG_SP sigc_sp
#endif

#ifdef USE_SIGINFO
#  define GET_SIGCONTEXT(info, ucontext) \
	((struct ucontext *)ucontext)->mcontext
#  define GET_SEGV_ADDRESS(info, ucontext) info->si_addr
#else
#  define GET_SIGCONTEXT(info, ucontext) ((struct sigcontext *)&info)
#  define GET_SEGV_ADDRESS(info, ucontext) \
	GET_SIGCONTEXT(info, ucontext)->REG_ADDR
#  define GET_SEGV_REASON(info, ucontext) \
	GET_SIGCONTEXT(info, ucontext)->REG_REASON
#  define GET_SIGNAL_SP(info, ucontext) \
	GET_SIGCONTEXT(info, ucontext)->REG_SP
#endif

int _a386_interrupts_enabled = 0;
static int interrupts_enabled_mask[2];
static int interrupts_disabled_mask[2];
static int segfault_mask[2];
static int altstack[4096 / sizeof (int)];
struct a386_vectors *_a386_vectors;

void
a386_interrupts_enable (void)
{
  _a386_assert_kernel_mode ();
  _a386_interrupts_enabled = 1;
  unix_sigprocmask (SIG_SETMASK, &interrupts_enabled_mask, 0);
}

void
a386_interrupts_disable (void)
{
  _a386_assert_kernel_mode ();
  _a386_interrupts_enabled = 0;
  unix_sigprocmask (SIG_SETMASK, &interrupts_disabled_mask, 0);
}

int
a386_interrupts_enabled (void)
{
  _a386_assert_kernel_mode_return (int);
  return _a386_interrupts_enabled;
}

void
a386_set_vectors (struct a386_vectors *v)
{
  _a386_assert_kernel_mode ();
  _a386_vectors = v;
}

struct a386_vectors *
a386_get_vectors (void)
{
  _a386_assert_kernel_mode_return (struct a386_vectors *);
  return _a386_vectors;
}

#define call_vector(vec, context) \
do { \
  _a386_set_kernel_mode (); \
  if (_a386_vectors->vec == 0) \
    a386_panic ("vector " #vec " is NULL\n"); \
  _a386_vectors->vec (0); \
  _a386_restore_mode (); \
} while (0)

#define verify_vector(vec) \
do { \
  if (_a386_vectors->vec == 0) \
    a386_panic ("vector " #vec " is NULL!"); \
} while (0)

static void sigill_handler (int sig, siginfo_t *info, void *context)
     __attribute__ ((section (".utext")));
static void sigfpe_handler (int sig, siginfo_t *info, void *context)
     __attribute__ ((section (".utext")));
static void sigtrap_handler (int sig, siginfo_t *info, void *context)
     __attribute__ ((section (".utext")));
static void sigsegv_handler (int sig, siginfo_t *info, void *context)
     __attribute__ ((section (".utext")));
static void sigvtalrm_handler (int sig, siginfo_t *info, void *context)
     __attribute__ ((section (".utext")));

static void
sigill_handler (int sig, siginfo_t *info, void *sigcontext)
{
	  {
	    int i;
	    extern long _a386_regs[];
	    long *r = (long *)_a386_regs;
	    static char *regname[] =
	    { "ebx", "ecx", "edx", "esi", "edi", "ebp", "eax", "xds",
	      "xes", "orig_eax", "eip", "xcs", "eflags", "esp", "xss",
	      "kernel_mode", "interrupts_enabled" };
	    for (i = 0; i < 17; i++)
	      a386_printf ("%s = %08lx\n", regname[i], r[i]);
	  }

  a386_panic ("illegal instruction");

#if 0
  _a386_set_kernel_mode ();
  if (_a386_vectors->illegal_instruction == 0)
    a386_panic ("vector illegal_instruction is NULL!\n");
  _a386_vectors->illegal_instruction (0);
  _a386_restore_mode ();
#endif
}

static void
sigterm_handler (int sig, siginfo_t *info, void *sigcontext)
{
  a386_halt ();
}

static void
sigbus_handler (int sig, siginfo_t *info, void *sigcontext)
{
  a386_panic ("bus error");
}

static void
sigfpe_handler (int sig, siginfo_t *info, void *context)
{
  a386_panic ("FPE exception");

#if 0
  call_vector (fpe, context);
#endif
}

static void
sigtrap_handler (int sig, siginfo_t *info, void *context)
{
  call_vector (trace, context);
}

static int
_a386_make_reason (int os_reason, int kernel_mode)
{
  int reason = 0;

  reason |= os_reason & 1 ? A386_REASON_PROT : A386_REASON_NOPAGE;
  reason |= os_reason & 2 ? A386_REASON_WRITE : A386_REASON_READ;
  reason |= kernel_mode ? A386_REASON_KERNEL : A386_REASON_USER;

  return reason;
}

static void
sigsegv_handler (int sig, siginfo_t *info, void *ucontext)
{
  static int in_segv_handler = 0, reason;
  a386_address_t address, ksp;
  //a386_putchar ('<');
  //if (_a386_in_kernel_mode) a386_putchar ('K');
  if (in_segv_handler)
    {
      a386_panic ("fault in sigsegv_handler");
      //unix_exit (99);
    }
  in_segv_handler = 1;
  unix_sigprocmask (SIG_UNBLOCK, segfault_mask, 0);
  address = GET_SEGV_ADDRESS (info, ucontext);
  reason = GET_SEGV_REASON (info, ucontext);
  _a386_set_kernel_mode_with_sp (GET_SIGNAL_SP (info, ucontext));
if (a386_get_sp () < (a386_address_t)0xa0000000) a386_putchar ('-');
  reason = _a386_make_reason (reason, a386_old_kernel_mode);
#if 0
  a386_printf ("sigsegv_handler: ip = %p, sp = %p, cr2 = %p, err = %x, km = %d\n",
	       GET_SIGCONTEXT (info, ucontext)->eip,
	       GET_SIGCONTEXT (info, ucontext)->esp,
	       GET_SIGCONTEXT (info, ucontext)->cr2,
	       GET_SIGCONTEXT (info, ucontext)->err,
	       a386_old_kernel_mode);
#endif
  in_segv_handler = 0;
  if (a386_old_kernel_mode)
    ksp = GET_SIGNAL_SP (info, ucontext);
  else
    ksp = _a386_ksp;
#if 0
a386_printf ("[sp = %p]", a386_get_sp ());
a386_printf ("[_ksp = %p]", _a386_ksp);
a386_printf ("[signal sp = %p]", GET_SIGNAL_SP (info, ucontext));
a386_printf ("[ksp = %p]", ksp);
a386_printf ("[altstack = %p]", altstack);
#endif
  _a386_page_fault_handler (address, reason, ksp);
  //a386_putchar ('>');
if (a386_get_sp () < (a386_address_t)0xa0000000) a386_putchar ('~');
  _a386_restore_mode_no_ksp ();
}

void
sigvtalrm_handler (int sig, siginfo_t *info, void *ucontext)
{
  a386_address_t ksp;

  verify_vector (timer);
  
  if (_a386_in_kernel_mode)
    ksp = GET_SIGNAL_SP (info, ucontext);
  else
    ksp = _a386_ksp;

  _a386_set_kernel_mode_with_sp (GET_SIGNAL_SP (info, ucontext));
  a386_set_sp (ksp);
  _a386_serial_poll ();
  _a386_vectors->timer (0);
  _a386_restore_mode_no_ksp ();
}

void
sigvtalrm_handler_no_altstack (void)
{
  verify_vector (timer);
  _a386_set_kernel_mode ();
  _a386_serial_poll ();
  _a386_vectors->timer (0);
  _a386_restore_mode ();
}

void
_a386_device_interrupt (int number)
{
  if (!_a386_interrupts_enabled)
    return; /* FIXME: add to pending interrupts */

  _a386_set_kernel_mode ();
  if (_a386_vectors->device_interrupt == 0)
    a386_panic ("vector device_interrupt is NULL!\n");
  _a386_vectors->device_interrupt[number] (number);
  _a386_restore_mode ();
}

static void
_a386_signal (int sig, void *handler)
{
  struct unix_sigaction sa;

#ifdef USE_SIGINFO
#  define MAYBE_SA_SIGINFO		SA_SIGINFO
#  define SA_HANDLER_OR_SIGACTION	sa_sigaction
#else
#  define MAYBE_SA_SIGINFO		0
#  define SA_HANDLER_OR_SIGACTION	sa_handler
#endif

  unix_sigemptyset (&sa.sa_mask);
  sa.sa_flags = SA_RESTART | SA_ONSTACK | MAYBE_SA_SIGINFO;
  sa.SA_HANDLER_OR_SIGACTION = handler;

  if (unix_sigaction (sig, &sa, 0) < 0)
    a386_panic ("sigaction failed");
}

void
_a386_interrupt_init (void)
{
  unix_stack_t ss;

  ss.ss_sp = altstack - 16;
  ss.ss_size = sizeof altstack;
  ss.ss_flags = 0;
  if (unix_sigaltstack (&ss, 0) < 0)
    a386_panic ("sigaltstack failed");

  unix_sigemptyset (&interrupts_enabled_mask);
  unix_sigfillset (&interrupts_disabled_mask);
  unix_sigemptyset (&segfault_mask);
  unix_sigaddset (&segfault_mask, SIGSEGV);

  a386_interrupts_disable ();

  _a386_signal (SIGILL, sigill_handler);
  _a386_signal (SIGFPE, sigfpe_handler);
  _a386_signal (SIGBUS, sigbus_handler);
  _a386_signal (SIGTRAP, sigtrap_handler);
  _a386_signal (SIGTERM, sigterm_handler);
  _a386_signal (SIGSEGV, sigsegv_handler);
  _a386_signal (SIGVTALRM, sigvtalrm_handler);
}

/*
 * a386_interrupt_wait() nanosleeps() until the next SIGVTALRM should
 * have occurred if we didn't sleep.  We do this because the virtual
 * timer decrements only when the process is executing.  We could have
 * used the real time timer, but then we'd be waisting cpu when other
 * processes are executing.
 *
 * Of course, nanosleep() can be interrupted by some other signal, in
 * which case we adjust the value of the virtual timer accordingly.
 */

void
a386_interrupt_wait (void)
{
  struct itimerval itv;
  struct timespec ts, ts2;
  int err;

  _a386_assert_kernel_mode ();

  a386_interrupts_enable ();

  unix_getitimer (ITIMER_VIRTUAL, &itv);
  ts.tv_sec = itv.it_value.tv_sec;
  ts.tv_nsec = itv.it_value.tv_usec * 1000;
  err = unix_nanosleep (&ts, &ts2);

  if (err == EINTR)
    {
a386_panic ("a386_interrupt_wait: err = EINTR: never happened before");
      itv.it_value.tv_sec = ts2.tv_sec;
      itv.it_value.tv_usec = (ts2.tv_nsec + 500) / 1000;
a386_printf ("\nsetitimer %d.%06d", ts2.tv_sec, itv.it_value.tv_usec);
      unix_setitimer (ITIMER_VIRTUAL, &itv, 0);
    }
  else
    {
      _a386_context_save (&&ret);
      sigvtalrm_handler_no_altstack ();
    ret:
      itv.it_value.tv_sec = itv.it_interval.tv_sec;
      itv.it_value.tv_usec = itv.it_interval.tv_usec;
      unix_setitimer (ITIMER_VIRTUAL, &itv, 0);
    }
}
