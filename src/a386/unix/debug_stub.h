#ifdef A386_DEBUG

int DEBUG_ON = DEBUG_DEFAULT;

#define debug(format, args...) \
do { \
  if (DEBUG_ON >= 2) \
    a386_printf ("\na386 " DEBUG_NAME ": " format , ## args); \
} while (0)

#define log_error(format, args...) \
do { \
  if (DEBUG_ON >= 1) \
    a386_printf ("\na386 " DEBUG_NAME ": " format , ## args); \
} while (0)

#else /* !A386_DEBUG */

#define debug() do { } while (0)
#define log_error() do { } while (0)

#endif /* !A386_DEBUG */
