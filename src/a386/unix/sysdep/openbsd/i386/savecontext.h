/* $RuOBSD$ */

static inline void
_a386_context_save (void *ip)
{
  extern int _a386_regs[];
  extern int _a386_in_kernel_mode;
  extern int _a386_interrupts_enabled;

#define reg(r, n) "movl %%" #r ", %0+4*" #n "\n\t"
  asm volatile (
     reg (ebx, 0)
     reg (ecx, 1)
     reg (edx, 2)
     reg (esi, 3)
     reg (edi, 4)
     reg (ebp, 5)
     reg (eax, 6)
     "movl $0,   %0+4*7\n\t"	/* xds */
     "movl $0,   %0+4*8\n\t"	/* xes */
     reg (eax, 9)		/* orig_eax */
     "movl %3,   %%eax\n\t"
     "movl %%eax,%0+4*10\n\t"	/* eip */
     "movl $0,   %0+4*11\n\t"	/* xcs */
     "pushf\n\t"
     "popl       %0+4*12\n\t"	/* eflags */
     reg (esp, 13)
     "movl $0,   %0+4*14\n\t"	/* xss */
     "movl %1,%%eax\n\t"
     "movl %%eax,%0+4*15\n\t"	/* kernel_mode */
     "movl %2,%%eax\n\t"
     "movl %%eax,%0+4*16\n\t"	/* interrupts_enabled */
     : "=m" (_a386_regs[0])
     : "m" (_a386_in_kernel_mode),
       "m" (_a386_interrupts_enabled),
       "m" (ip)
     : "eax");
#undef reg
}
