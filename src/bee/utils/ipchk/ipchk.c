/* $oganer$ */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <syslog.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "misc.h"
#include "ipchk.h"

char        delim[]  = " \t\n\r";    // parse delimiter
char        delim2[] = ".";          // address octets delimiter
char        delim3[] = "/\\";        // mask delimiter  

char        buf[512];                // parse buffer

char      * rulefile;                // rules path
int         fverboze = 0;            // verboze output flag

inaddr_t    host_addr;               // host address to check

/* * * * * * * * * * * * *\
 *     Main function     *
\* * * * * * * * * * * * */
int main(int argc, char ** argv)
{
   int                i;
   int                n;
   FILE             * f;
   int                rc;
   int                line;
   int                skip;
   int                len;

//   char             * argent;   // new argv entry

   char             * ptr;	// parsing variables
   char             * str;

   char             * ptr2;     // secondary parse       

   inaddr_t           addr;
   int                mask;

   int                rule;
   int                access;
   

// Force initialize hostaddr
   host_addr.dword = 0;

/* Parse command line */
   while ((rc = getopt(argc, argv, OPTS)) != (-1))
   {  switch (rc)
      {  case 'f':         /* configuration filename */
            setrealpath(optarg, &rulefile);
            break;

         case 'v':        /* verboze mode */
            fverboze = 1;
            break;

         case 'h':        /* address */
            if (inet_aton(optarg, &(host_addr.inaddr)) < 1)
            {  fprintf(stderr, "ERROR: invalid host address\n");
               return ACCESS_DENIED;
            }
            break;

         case '?':        /* Show usage */
         default:
            usage();
            return ACCESS_DENIED; 
      }
   }

/* Check required arguments */
   if (host_addr.dword == 0 || rulefile == NULL)
   {  fprintf(stderr, "ERROR: not enough arguments (-? for help)\n");
      return ACCESS_DENIED;
   }

/* Open rules file */
   f = fopen(rulefile, "r");
   if (f == NULL)
   {  fprintf(stderr, "ERROR: fopen(%s): %s\n", rulefile, strerror(errno));
      return ACCESS_DENIED;
   }

/* Parse & check host addr */
   access = ACCESS_DENIED;   // denied by default
   line = 0;
   skip = 0;
   while(fgets(buf, sizeof(buf), f) != NULL)
   {  
// count line number (1-based)
      if (skip == 0) line++;
// locate line end
      len = strlen(buf);
// skip abnormal empty (safety)
      if (len == 0) continue;
// check for too long lines (no EOL)
      if (buf[len-1] != '\n')
      {  if (skip == 0)
         {  if (fverboze) fprintf(stderr, "%s:%d: line too long, skipped\n", rulefile, line);
            skip = 1;
         }
         continue;
      }
// delete EOL
      buf[len-1] = '\0';
// skip rest of long line (with EOL)
	      if (skip != 0) 
      {  skip = 0;
         continue;
      }
// skip comments & empty
      if (*buf == '#' || *buf == '\0') continue;

// parse rule directive
      ptr = buf;
      str = next_token(&ptr, delim);
// skip whitespaceful (i.e. empty) line
      if (str == NULL) continue;

// check rule directive
      if (strcasecmp(str, "pass") == 0) rule = ACCESS_GRANTED;
      else
      {  if (strcasecmp(str, "block") == 0) rule = ACCESS_DENIED;
         else
         {  if (fverboze) 
               fprintf(stderr, "%s:%d: invalid rule \"%s\", skipped\n", rulefile, line, str);
            continue;
         }
      }  

// parse rule address
      str = next_token(&ptr, delim);
      if (str == NULL)
      {  if (fverboze) fprintf(stderr, "%s:%d: no address, skipped\n", rulefile, line);
         continue;
      }
// special word "any" trap
      if (strcasecmp(str, "any") == 0)
      {  access = rule;
         if (fverboze) fprintf(stderr, "%s any                  MATCH - %s\n", 
                  rule   ? "pass ":"block",
                  access ? "grant":"deny");
         continue;
      }
// parse address substrings
      ptr2       = str;
      addr.dword = 0;
      mask       = 0;
      for (i=0; i<4; i++)
      {  str = next_token(&ptr2, i<3 ? delim2 : delim3);
         if (str == NULL)
         {  if (fverboze) fprintf(stderr, "%s:%d: invalid address, skipped\n", rulefile, line);
            break;
         }
         n = strtol(str, NULL, 10);
         if (n < 0 || n > 255)
         {  if (fverboze) fprintf(stderr, "%s:%d: invalid address, skipped\n", rulefile, line);
            break;
         }
         addr.byte[i] = n;
      }

      if (i < 4) continue; // if cycle was broken
         
      str = next_token(&ptr2, delim3);
      if (str == NULL)
      {  
      // auto-count mask
         for (i=3, mask=32; i>=0; i--, mask-=8)
            if (addr.byte[i] != 0) break;
      }
      else
      {  mask = strtol(str, NULL, 10);
         if (mask < 0 || mask > 32)
         {  if (fverboze) fprintf(stderr, "%s:%d: invalid mask, skipped\n", rulefile, line);
            continue;
         }
      }

      if (fverboze) 
         fprintf(stderr, "%s %03d.%03d.%03d.%03d/%02d   ", rule ? "pass ":"block", 
            addr.byte[0], addr.byte[1], addr.byte[2], addr.byte[3], mask);

      if (ipcmp(host_addr.inaddr.s_addr, addr.inaddr.s_addr, mask))
      {  access = rule;
         if (fverboze) 
            fprintf(stderr, "MATCH - %s\n", access ? "grant" : "deny");
      }
      else
         if (fverboze) fprintf(stderr, "UNMATCH\n");
   }

   fclose(f);
   f = NULL;

   printf("%s\n", access ? "GRANTED" : "DENIED");

   return access;
}

int ipcmp(in_addr_t host, in_addr_t rule, int mask)
{
   in_addr_t   bit;
   in_addr_t   and_mask = 0;

   for (bit=0x80000000; mask>0; mask--, bit >>= 1)
      and_mask |= bit;

   and_mask = htonl(and_mask);

   return (host & and_mask) == (rule & and_mask);
}


/* * * * * * * * * * * * * * * * * * *\
 *    Print USAGE info to STDERR     *
\* * * * * * * * * * * * * * * * * * */
void usage()
{
   fprintf(stderr,
"Small billing solutions project (BEE)\n"
"   rule list IP checker\n"
"(c) Special Equipment Software Section, JSC Oganer-Service, Norilsk\n"
"Usage: ipchk -f rulefile -h ip [-v]\n"
"       ipchk -? \n"
"      available options:\n"
"  f file - rules file               (required)\n"
"  h ip   - host ip address to check (required)\n"
"  v      - verboze mode\n"
"  ?      - show this usage\n");

}

/* * * * * * * * * * * * * * * * * * * *\
 *   Get absolute path from relative   *
 *          allocate string            *
 *  store pointer to given location    *
\* * * * * * * * * * * * * * * * * * * */

int setrealpath(char * file, char ** ptr)
{  char * buf;

   buf = calloc(1, MAXPATHLEN+1);
   if (buf == NULL)
   {  syslog(LOG_ERR, "setrealpath(calloc(%d)): %m", MAXPATHLEN+1);
      return (-1);
   }
   if (realpath(file, buf) == NULL)
   {  syslog(LOG_ERR, "setrealpath(realpath(%s)): %m", file);
      free(buf);
      return (-1);
   }
   *ptr = buf;

   return 0;
}

