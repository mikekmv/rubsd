/* $RuOBSD$ */

#define CPU_NAME "x86"

#define regaddr(reg) (void *)((reg) * 4)

#define REG_SP regaddr (UESP)
#define REG_IP regaddr (EIP)
#define REG_SYSCALL_NUBMER regaddr (ORIG_EAX)
#define REG_ARG_1 regaddr (EBX)
#define REG_ARG_2 regaddr (ECX)
#define REG_ARG_3 regaddr (EDX)
#define REG_ARG_4 regaddr (ESI)
#define REG_ARG_5 regaddr (EDI)

#define SET_RETURN_ADDRESS(pid, ip) \
do { \
  long sp; \
  unix_ptrace (PTRACE_PEEKUSER, pid, REG_SP, (int)&sp); \
  unix_ptrace (PTRACE_POKEUSER, pid, REG_SP, sp - 4); \
  unix_ptrace (PTRACE_POKEDATA, pid, (void *)(sp - 4), ip); \
} while (0)

#define GET_SYSCALL_NUMBER(pid, n) \
  unix_ptrace (PTRACE_PEEKUSER, pid, REG_SYSCALL_NUBMER, (int)&n)

#define GETREGS(pid, regs) \
do { \
  int i; \
  unix_ptrace (PTRACE_GETREGS, pid, 0, (int)regs); \
  for (i = 9; i < 15; i++) \
    regs[i] = regs[i + 2]; \
  unix_ptrace (PTRACE_PEEKDATA, pid, \
               &_a386_in_kernel_mode, (int)&regs[15]); \
  unix_ptrace (PTRACE_PEEKDATA, pid, \
               &_a386_interrupts_enabled,(int)&regs[16]); \
} while (0)
