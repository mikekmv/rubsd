#define CPU_NAME "SPARC"

/* PT_* are byte offsets for the pt_regs struct. */

#define REG_SP PT_FP
#define REG_IP PT_NPC
#define REG_SYSCALL_NUBMER PT_G1
#define REG_ARG_1 PT_I0
#define REG_ARG_2 PT_I1
#define REG_ARG_3 PT_I2
#define REG_ARG_4 PT_I3
#define REG_ARG_5 PT_I4
#define REG_RETPC PT_I7 /* == UREG_RETPC */

#define SET_RETURN_ADDRESS(pid, ip) \
do { \
  unix_ptrace (PTRACE_PEEKUSER, pid, REG_RETPC, ip); \
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

