#ifndef _UNIX_A386_SET_USER_MODE_H
#define _UNIX_A386_SET_USER_MODE_H

#include <a386/a386_vectors.h>
#include <a386/savecontext.h>

extern int _a386_in_kernel_mode;
extern int _a386_interrupts_enabled;
extern a386_address_t _a386_usp;
extern a386_address_t _a386_ksp;
extern void _a386_memory_map (void);
extern void _a386_memory_unmap (void);
extern struct a386_vectors *_a386_vectors;
extern volatile int a386_old_kernel_mode;
extern void _a386_mmu_map_address (a386_address_t);
extern void _a386_switch_to_user_mode (void);

#define _a386_assert_kernel_mode() \
if (!_a386_in_kernel_mode) \
  { \
    _a386_set_kernel_mode (); \
    _a386_vectors->privilege_violation (__FUNCTION__); \
    _a386_restore_mode (); \
    return; \
  }

extern inline void
_a386_restore_mode_no_ksp (void)
{
  if (a386_old_kernel_mode)
    return;
  /*_a386_memory_unmap ();*/
  _a386_switch_to_user_mode ();
  _a386_in_kernel_mode = 0;
  a386_set_sp (_a386_usp);
}

extern inline void
_a386_restore_mode (void)
{
  if (a386_old_kernel_mode)
    return;
  _a386_ksp = a386_get_sp ();
  /*_a386_memory_unmap ();*/
  _a386_switch_to_user_mode ();
  _a386_in_kernel_mode = 0;
  a386_set_sp (_a386_usp);
}

extern inline void
_a386_set_kernel_mode_with_sp (a386_address_t sp)
{
  a386_old_kernel_mode = _a386_in_kernel_mode;
  if (_a386_in_kernel_mode)
    return;
  _a386_usp = sp;
  _a386_in_kernel_mode = 1;
  /*_a386_memory_map ();*/
}

extern inline void
_a386_set_kernel_mode (void)
{
  a386_old_kernel_mode = _a386_in_kernel_mode;
  if (_a386_in_kernel_mode)
    return;
  _a386_usp = a386_get_sp ();
  a386_set_sp (_a386_ksp);
  _a386_in_kernel_mode = 1;
  /*_a386_memory_map ();*/
}

extern inline void
a386_set_user_mode (void)
{
  _a386_assert_kernel_mode ();
  a386_old_kernel_mode = 0;
  _a386_restore_mode ();
}

extern inline void
a386_set_kernel_mode (void)
{
  _a386_assert_kernel_mode ();
  _a386_set_kernel_mode ();
}

extern inline void
_a386_trap_no_save (int n)
{
  _a386_set_kernel_mode ();
  a386_syscall_number = n;
  a386_set_ip ((a386_address_t)_a386_vectors->trap);
}

extern inline a386_result_t
a386_trap (int n)
{
  _a386_context_save (&&ret);
  _a386_trap_no_save (n);
 ret:
  return a386_get_result ();
}

#endif /* _UNIX_A386_SET_USER_MODE_H */
