#define CPU_NAME "ARM"

#define regaddr(reg) \
  (void *)((char *)&(((struct pt_regs *)0)->reg) - (char *)0)

#define REG_SP regaddr (ARM_sp)
#define REG_IP regaddr (ARM_pc)
#define REG_LR regaddr (ARM_lr)
#define REG_SYSCALL_NUBMER regaddr (ARM_ORIG_r0)
#define REG_ARG_1 regaddr (ARM_r0)
#define REG_ARG_2 regaddr (ARM_r1)
#define REG_ARG_3 regaddr (ARM_r2)
#define REG_ARG_4 regaddr (ARM_r3)
#define REG_ARG_5 regaddr (ARM_r4)

#define SET_RETURN_ADDRESS(pid, ip) \
  unix_ptrace (PTRACE_POKEUSER, pid, REG_LR, ip)

#define GET_SYSCALL_NUMBER(pid, n) \
do { \
  long ip; \
  unix_ptrace (PTRACE_PEEKUSER, pid, REG_IP, (int)&ip); \
  unix_ptrace (PTRACE_PEEKTEXT, pid, (void *)(ip - 4), (int)&n); \
  n &= 0x00ffffffL; \
} while (0)

#define GETREGS(pid, regs) \
do { \
  int i; \
  for (i = 0; i < 1; i++) \
    unix_ptrace (PTRACE_PEEKUSER, pid, (void *)(4 * i), (int)((regs)[i])); \
} while (0)
