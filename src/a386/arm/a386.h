typedef int a386_word_t;

typedef a386_word_t a386_result_t;
typedef a386_word_t a386_syscall_arg_t;

static inline a386_result_t
a386_trap (int n)
{
  a386_result_t result;
  asm ("swi #%0;"
       "mov %1, r0"
       : "=r" (result)
       : "i" (n));
  return result;
}

static inline void
a386_enable_interrupts (void)
{
  unsigned long temp;
  asm volatile ("mrs %0, cpsr;"
		"bic %0, %0, #128;"
		"msr cpsr, %0"
		: "=r" (temp)
		:
		: "memory");
}

static inline void
a386_disenable_interrupts (void)
{
  unsigned long temp;
  asm volatile ("mrs %0, cpsr;"
		"orr %0, %0, #128;"
		"msr cpsr, %0"
		: "=r" (temp)
		:
		: "memory");
}

static inline int
a386_interrupts_allowed (void)
{
  return 0;
}
