#ifndef _UNIX_A386_VECTORS_H
#define _UNIX_A386_VECTORS_H

#include "a386_params.h"
#include "a386_types.h"
#include "a386_context.h"

struct a386_vectors
{
  void (*illegal_instruction) (struct a386_context *);
  a386_result_t (*privilege_violation) (void *);
  a386_result_t (*trap) (void);
  void (*trace) (struct a386_context *);
  void (*fpe) (struct a386_context *);
  void (*bus_error) (void);
  void (*address_error) (void);
  void (*page_fault) (void);
  void (*segmentation_violation) (void);
  void (*timer) (struct a386_context *);
  void (*device_interrupt[A386_DEVICE_INTERRUPTS]) (int number);
  void (*serial_interrupt) (void);
};

#endif /* _UNIX_A386_VECTORS_H */
