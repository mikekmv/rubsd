#include <stdio.h>
#include <string.h>

#include <a386/a386.h>

int unix_errno;

int
main (int argc, char **argv)
{
  a386_address_t start;
  char *c;
  int i;

  printf ("Loading %s...", argv[1]);

  start = (a386_address_t)a386_load (argv[1]);
  if (start == 0)
    {
      fprintf (stderr, "\ncouldn't load %s: %s\n", argv[1], strerror (unix_errno));
      exit (1);
    }
  
  printf ("done\n");

  c = (char *)A386_MEMORY_BASE + 0x4000 + 2048;
  for (i = 2; i < argc; i++)
    {
      int n;

      n = strlen (argv[i]);
      memcpy (c, argv[i], n);
      c += n;
      *c++ = ' ';
    }
  *--c = 0;

  a386_set_ip (start);
}
