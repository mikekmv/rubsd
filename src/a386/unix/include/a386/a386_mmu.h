#ifndef _A386_UNIX_A386_MMU_H
#define _A386_UNIX_A386_MMU_H

#define A386_PAGE_SHIFT 12
#define _A386_PAGE_SIZE (1 << A386_PAGE_SHIFT)
#define _A386_PAGE_MASK (_A386_PAGE_SIZE - 1)

#include "a386_types.h"

struct a386_page_table_entry
{
  a386_address_t present : 1;
  a386_address_t write_access : 1;
  a386_address_t user_access : 1;
  a386_address_t zero2 : 2;
  a386_address_t accessed : 1;
  a386_address_t dirty : 1;
  a386_address_t zero : 2;
  a386_address_t reserved : 3;
  a386_address_t page_frame : 20;
};

struct a386_page_directory_entry
{
  a386_address_t present : 1;
  a386_address_t write_access : 1;
  a386_address_t user_access : 1;
  a386_address_t zero2 : 2;
  a386_address_t accessed : 1;
  a386_address_t dirty : 1;
  a386_address_t zero : 2;
  a386_address_t reserved : 3;
  a386_address_t page_table : 20;
};

#endif /* _A386_UNIX_A386_MMU_H */
