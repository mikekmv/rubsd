static inline void
_a386_context_save (void *ip)
{
  extern int _a386_regs[];
  extern int _a386_in_kernel_mode;
  extern int _a386_interrupts_enabled;

#define reg(r, n) "st %%" #r ", [%%l0+4*" #n "]\n\t"
  asm volatile (
     "ld %0, %%l0\n\t"		/* dummy */
     "sethi %%hi(_a386_regs), %%l0\n\t"
     "or %%l0, %%lo(_a386_regs), %%l0\n\t"
     reg (g0, 0)
     reg (g1, 1)
     reg (g2, 2)
     reg (g3, 3)
     reg (g4, 4)
     reg (g5, 5)
     reg (g6, 6)
     reg (g7, 7)
     "mov %%psr, %%l1\n\t"
     reg (l1, 8)
     "mov %%sp, %%l1\n\t"
     reg (l1, 9)

     /*
     "mov %%pc, %%l1\n\t"
     reg (l1, 10)
     "mov %%npc, %%l1\n\t"
     reg (l1, 11)
     */

     "mov %%y, %%l1\n\t"
     reg (l1, 10)
     "ld %1,%%l1\n\t"		/* kernel_mode */
     reg (l1, 11)
     "ld %2,%%l1\n\t"		/* interrupts_enabled */
     reg (l1, 12)
     "ld %3,%%l1\n\t"		/* ip */
     reg (l1, 13)
     : "=m" (_a386_regs[0])
     : "m" (_a386_in_kernel_mode),
       "m" (_a386_interrupts_enabled),
       "m" (ip)
     : "l0", "l1");
#undef reg
}
