static inline void
_a386_context_save (void *pc)
{
  extern int _a386_regs[];
  extern int _a386_in_kernel_mode;
  extern int _a386_interrupts_enabled;

#define reg(r, n) "str " #r ", " #n "\n\t"
  asm volatile (
      reg(r0, %0)
      reg(r1, %1)
      reg(r2, %2)
      reg(r3, %3)
      reg(r4, %4)
      reg(r5, %5)
      reg(r6, %6)
      reg(r7, %7)
      reg(r8, %8)
      reg(r9, %9)
      : "=m" (_a386_regs[0]),
        "=m" (_a386_regs[1]),
        "=m" (_a386_regs[2]),
        "=m" (_a386_regs[3]),
        "=m" (_a386_regs[4]),
        "=m" (_a386_regs[5]),
        "=m" (_a386_regs[6]),
        "=m" (_a386_regs[7]),
        "=m" (_a386_regs[8]),
        "=m" (_a386_regs[9]));
  asm volatile (
     reg(r10, %0)
     reg(fp, %1)
     reg(ip, %2)
     reg(sp, %3)
     reg(lr, %4)
     "ldr r1, %7\n\t"
     "mrs r2, cpsr\n\t"
     "str r1, %5\n\t"	/* pc */
     "str r2, %6\n\t"	/* cpsr */
     reg(r0, %7)
     : "=m" (_a386_regs[10]),
       "=m" (_a386_regs[11]),
       "=m" (_a386_regs[12]),
       "=m" (_a386_regs[13]),
       "=m" (_a386_regs[14]),
       "=m" (_a386_regs[15]),
       "=m" (_a386_regs[16]),
       "=m" (_a386_regs[17])
     : "m" (pc)
     : "r1", "r2");
  asm volatile (
     "ldr r1, %2\n\t"
     "ldr r2, %3\n\t"
     "str r1, %0\n\t"	/* kernel_mode */
     "str r2, %1\n\t"	/* interrupts_enabled */
     : "=m" (_a386_regs[18]),
       "=m" (_a386_regs[19])
     : "m" (_a386_in_kernel_mode),
       "m" (_a386_interrupts_enabled)
     : "r1", "r2");

#if 0
#define reg(r, n) "str " #r ", %0+4*" #n "\n\t"
 asm volatile (
     reg(r0, 0)
     reg(r1, 1)
     reg(r2, 2)
     reg(r3, 3)
     reg(r4, 4)
     reg(r5, 5)
     reg(r6, 6)
     reg(r7, 7)
     reg(r8, 8)
     reg(r9, 9)
     reg(r10, 10)
     reg(fp, 11)
     reg(ip, 12)
     reg(sp, 13)
     reg(lr, 14)
     "ldr r1, %3\n\t"
     "mrs r2, cpsr\n\t"
     "str r1, %0+4*15\n\t"	/* pc */
     "str r2, %0+4*16\n\t"	/* cpsr */
     reg(r0, 17)
     "ldr r1, %1\n\t"
     "ldr r2, %2\n\t"
     "str r1, %0+4*18\n\t"	/* kernel_mode */
     "str r2, %0+4*19\n\t"	/* interrupts_enabled */
     : "=m" (_a386_regs[0])
     : "m" (_a386_in_kernel_mode),
       "m" (_a386_interrupts_enabled),
       "m" (pc)
     : "r1", "r2");
#endif
#undef reg
}
