/* $RuOBSD$ */

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

#include <bee.h>
#include "ipchk.h"

char        delim[]  = " \t\n\r";    // parse delimiter
char        delim3[] = "/";          // mask delimiter  

u_char      buf[512];                // parse buffer

char      * rulefile = NULL;         // rules path
int         verbose  = 0;            // verbose output flag
int         paranoic = 0;            // paranoic mode (deny on line errors)
int         fmatch   = 0;            // first matching mode

inaddr_t    host_addr;               // host address to check

/* * * * * * * * * * * * *\
 *     Main function     *
\* * * * * * * * * * * * */
int main(int argc, char ** argv)
{
   int         i;
   FILE      * f = stdin;  // parse stdin by defaut
   int         rc;
   int         line;
   int         skip = 0;
   int         len;
   u_char    * ptr;	 // parsing variables
   u_char    * str;
   u_char    * ptr2;     // secondary parse       
   inaddr_t    addr;
   int         mask;
   int         rule;
   int         access;

/* Parse command line */
   while ((rc = getopt(argc, argv, OPTS)) != (-1))
   {  switch (rc)
      {  case 'f':         /* ACL filename */
            setrealpath(optarg, &rulefile);
            break;

         case 'v':        /* verbose mode */
            verbose += 1;
            break;

         case 's':        /* paranoic mode */
            paranoic += 1;
            break;

         case 'F':        /* first matching mode */
            fmatch  += 1;
            break;

         case '?':        /* Show usage */
         default:
            usage();
            return ACCESS_DENIED; 
      }
   }

   argc -= optind;
   argv += optind;

   if (argc < 1)
   {  fprintf(stderr, "ERROR: not enough arguments !\n");
      usage();
      return ACCESS_DENIED;
   }

   if (inet_aton(argv[0], &(host_addr.inaddr)) < 1)
   {  fprintf(stderr, "ERROR: invalid argument !\n");
      usage();
      return ACCESS_DENIED;
   }

/* Open rules file */
   if (rulefile != NULL)
   {  f = fopen(rulefile, "r");
      if (f == NULL)
      {  fprintf(stderr, "ERROR: fopen(%s): %s\n", rulefile, strerror(errno));
         return ACCESS_DENIED;
      }
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
         {  if (verbose > 0) fprintf(stderr, "%s:%d: line too long, skipped\n", rulefile, line);
            skip = 1;
         }
         if (!paranoic) continue;
         else
         {  if (verbose > 0) printf("DENIED\n");
            return ACCESS_DENIED;
         }
      }
// delete EOL
      buf[len-1] = '\0';
// skip rest of long line (with EOL)
      if (skip != 0) 
      {  skip = 0;
         continue;
      }
// skip comments & empty lines
      if (*buf == '#' || *buf == '\0') continue;

// parse rule directive
      ptr = buf;
      str = next_token((char**)&ptr, delim);
// skip whitespaceful (i.e. empty) line
      if (str == NULL) continue;

// on default (address only) rule is "grant"
      rule = ACCESS_GRANTED;

// parse PF-list or pass/block ACL
      if (str[0] == '!' ||
          (str[0] >= '0' &&  str[0] <= '9') ||
          strcasecmp(str, "any") == 0)
      {  while(1)
         {  while(str[0] == '!')
            {  rule = !rule;
               str++;
            } 
            if (str[0] == '\0')
            {  str = next_token((char**)&ptr, delim);
               if (str != NULL) continue;
            } 
            break;
         }
      }
      else
      {  if (strcasecmp(str, "pass") == 0) rule = ACCESS_GRANTED;
         else
         {  if (strcasecmp(str, "block") == 0) rule = ACCESS_DENIED;
            else
            {  if (verbose > 0) 
                  fprintf(stderr, "%s:%d: invalid rule \"%s\", skipped\n", rulefile, line, str);
               if (!paranoic) continue;
               else
               {  if (verbose > 0) printf("DENIED\n");
                  return ACCESS_DENIED;
               }
            }
         }
         str = next_token((char**)&ptr, delim);
      }  
     
      if (str == NULL) 
      {  if (verbose > 0) 
            fprintf(stderr, "%s:%d: no address, skipped\n", rulefile, line);
         if (!paranoic) continue;
         else
         {  if (verbose > 0) printf("DENIED\n");
            return ACCESS_DENIED;
         }
      }

// special word "any" trap
      if (strcasecmp(str, "any") == 0)
      {  access = rule;
         if (verbose > 1) fprintf(stderr, "%s any                  MATCH - %s\n", 
                  rule   ? "pass ":"block",
                  access ? "grant":"deny");
         if (!fmatch) continue;
         else
         {  if (verbose > 0) printf("%s\n", access ? "GRANTED" : "DENIED");
            return access;
         }
      }

// parse address substrings
      ptr2       = str;
      addr.dword = 0;
      mask       = 0;
      str = next_token((char**)&ptr2, delim3);
      if (str == NULL)
      {  if (verbose > 0) fprintf(stderr, "%s:%d: invalid address, skipped\n", rulefile, line);
         if (!paranoic) continue;
         else
         {  if (verbose > 0) printf("DENIED\n");
            return ACCESS_DENIED;
         }
      }

      if (inet_aton(str, &(addr.inaddr)) < 1)
      {  if (verbose > 0) fprintf(stderr, "%s:%d: invalid address, skipped\n", rulefile, line);
         if (!paranoic) continue;
         else
         {  if (verbose > 0) printf("DENIED\n");
            return ACCESS_DENIED;
         }
      }

      str = next_token((char**)&ptr2, delim3);
      if (str == NULL)
      {  
      // auto-count mask
         for (i=3, mask=32; i>=0; i--, mask-=8)
            if (addr.byte[i] != 0) break;
      }
      else
      {  mask = strtol(str, NULL, 10);
         if (mask < 0 || mask > 32)
         {  if (verbose > 0) fprintf(stderr, "%s:%d: invalid mask, skipped\n", rulefile, line);
            if (!paranoic) continue;
            else
            {  if (verbose > 0) printf("DENIED\n");
               return ACCESS_DENIED;
            }
         }
      }

      if (verbose > 1) 
         fprintf(stderr, "%s %03d.%03d.%03d.%03d/%02d   ", rule ? "pass ":"block", 
            addr.byte[0], addr.byte[1], addr.byte[2], addr.byte[3], mask);

      if (ipcmp(host_addr.inaddr.s_addr, addr.inaddr.s_addr, mask))
      {  access = rule;
         if (verbose > 1) 
            fprintf(stderr, "MATCH - %s\n", access ? "grant" : "deny");
         if (fmatch)
         {  if (verbose > 0) printf("%s\n", access ? "GRANTED" : "DENIED");
            return access;
         }
      }
      else
         if (verbose > 1) fprintf(stderr, "UNMATCH\n");
   }      

/* Open rules file */
   if (rulefile != NULL) fclose(f);

   if (verbose > 0) printf("%s\n", access ? "GRANTED" : "DENIED");

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
"   rule/acl list IP checker\n"
"Usage: ipchk [-f file] [options] IP\n"
"       ipchk -? \n"
"      available options:\n"
"  f file - rules/acl file (default - stdin)\n"
"  v      - verbose mode (-vv more verbose)\n"
"  s      - secure mode (deny on errors)\n"
"  F      - first (default - last) matching mode\n"
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
