/* $RuOBSD: beetraff.c,v 1.6 2003/04/13 20:03:11 shadow Exp $ */

// DEBUG    - no bee connection
//#define DEBUG
// DUMP JOB - copy commands to stderr
//#define DUMP_JOB

#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>

#include <bee.h>
#include <res.h>
#include <ipc.h>
#include <beetraff.h>

#define EXCLUSIONS 16

char * resname ="inet";

static char  buf[MAX_STRLEN];
link_t	     lnk;
char       * host = BEE_ADDR;
int          port = BEE_SERVICE;

// Exclusions list
exclitem_t   exclist[EXCLUSIONS];
int	     exclcount = 0;

extern char * optarg;
void usage(int rc);

int main(int argc, char ** argv)
{
   char            * tok = NULL;
   char            * p, * count, * proto, * from, * to, * fromport, * toport;
   char              c;
   int               cnt;
   char            * mask, * uname;
   unsigned long     protocol;
   unsigned short    localport;
   int               fUpdate=0;
   int               i;
   int               flag_from;
   int               flag_to;

//   int               temp;
//   struct protoent * pe;
//   char            * tmp;

#ifndef DEBUG
   int               rc;
   char            * msg;
   char              linbuf[128];
#endif

/*
 r  1. Redefine resource name
 a  2. Redefine daemon address
 A  3. Redefine daemon port
 u  4. Send update command to billing switch
 n  5. Exclude dest address
 N  6. Include dest address
*/

#define OPTS "r:a:A:un:N:"
   while ((c = getopt(argc, argv, OPTS)) != -1)
   {  switch (c)
      {  case 'r':
           resname=optarg;
           break;

         case 'a':
           host=optarg;
           break;

         case 'A':
           port=strtol(optarg, NULL, 0);
           break;

         case 'u':
           fUpdate=1;
           break;

         case 'n':
	   if (exclcount < EXCLUSIONS) 
           {  exclist[exclcount].item = optarg;
              exclist[exclcount++].flag = 0;
           }
           else
           {  fprintf(stderr, "Exclusions table overflow - ignoring \"-n %s\"\n",
                      optarg);
           }
           break;

         case 'N':
	   if (exclcount < EXCLUSIONS) 
           {  exclist[exclcount].item = optarg;
              exclist[exclcount++].flag = ITMF_COUNT;
           }
           else
           {  fprintf(stderr, "Exclusions table overflow - ignoring \"-N %s\"\n",
                      optarg);
           }
           break;

         default:
           usage(-1);
      }
   }

#ifdef DUMP_JOB
   fprintf(stderr, "NEW SESSION\n");
#endif

#ifndef DEBUG
   rc = link_request(&lnk, host, port);
   if (rc == -1)
   {  fprintf(stderr, "Can't connect to billing service (%s:%d): %s", host, port,
                 strerror(errno));
      exit(-1);
   }
#ifdef DUMP_JOB
   fprintf(stderr, "CONNECTING BEE\n");
#endif

   rc = answait(&lnk, RET_SUCCESS, linbuf, sizeof(linbuf), &msg);

#ifdef DUMP_JOB
   fprintf(stderr, "BEE: %03d\n", rc);
#endif

   if (rc != RET_SUCCESS)
   {  if (rc == LINK_DOWN) fprintf(stderr, "Unexpected link down\n");
      if (rc == LINK_ERROR) perror("Link error");
      if (rc >= 400) fprintf(stderr, "Billing error : %s\n", msg);
      exit(-1);
   }

#ifdef DUMP_JOB
   fprintf(stderr, "machine\n");
#endif

   link_puts(&lnk, "machine");
   rc = answait(&lnk, RET_SUCCESS, linbuf, sizeof(linbuf), &msg);

#ifdef DUMP_JOB
   fprintf(stderr, "BEE: %03d\n", rc);
#endif

   if (rc != RET_SUCCESS)
   {  if (rc == LINK_DOWN) fprintf(stderr, "Unexpected link down\n");
      if (rc == LINK_ERROR) perror("Link error");
      if (rc >= 400) fprintf(stderr, "Billing error : %s\n", msg);
      exit(-1);
   }
#endif  /* ifndef DEBUG */

   while (fgets(buf, sizeof(buf), stdin))
   {  

#ifdef DUMP_JOB
      fprintf(stderr, "STR: %s", buf);  // no "\n"
#endif

      if (!(p = strchr(buf, '\n')))     // long line
      {

#ifdef DUMP_JOB
         fprintf(stderr, "\n");    // adding "\n"
#endif

           fprintf(stderr, "line too long: %s\n", buf);
      }
      else  // long line
      {  if (*buf > '9' || *buf < '0')  // if first is not digit 
         { 
#ifdef DUMP_JOB
fprintf(stderr, "SKIPPED - header\n");
#endif
             continue;  // skip header lines
         }

         count = proto = from = to = fromport = toport = mask = NULL;
	 tok = buf;
	 localport = 0;


         p = next_token(&tok, IPFSTAT_DELIM);     // from addr
         if (p == NULL) continue;
         from = p;

         p = next_token(&tok, IPFSTAT_DELIM);     // to addr
         if (p == NULL) continue;
         to = p;

         p = next_token(&tok, IPFSTAT_DELIM);     // packets (skipping)
         if (p == NULL) continue;

         p = next_token(&tok, IPFSTAT_DELIM);     // bytes
         if (p == NULL) continue;
         count = p;

// Filter local traffic (i.e. excluded host to excluded host)

         flag_to   = 1;   // i.e. global by default
         flag_from = 1;

         for (i=0; i < exclcount; i++)
         {  if (inaddr_cmp(to, exclist[i].item) == 0)
               flag_to = (exclist[i].flag & ITMF_COUNT) != 0;
            if (inaddr_cmp(from, exclist[i].item) == 0)
               flag_from = (exclist[i].flag & ITMF_COUNT) != 0;
         }
         if (flag_to == 0 && flag_from == 0)
         {
#ifdef DUMP_JOB
fprintf(stderr, "local -  %s <-> %s\n", from, to);
#endif
            continue;
         }

// get count value
         cnt  = strtol(count, NULL, 10);

// suppose, that client ("user") is sending data
         uname = from;
// and trafic is outbount
         protocol = 0x80000000; /* out */

// not used (no ports in statistic)
//       if (toport != NULL)   protocol |= strtoul(toport, NULL, 0);
//       if (fromport != NULL) localport = strtoul(fromport, NULL, 0);

// not used (no proto in statictic)
//         if (proto != NULL && strcmp("tcp/udp", proto))
//         {  if (!(pe = getprotobyname(proto)))
//            {  fprintf(stderr, "unable determine protocol: %s, %s\n",
//               proto, strerror(errno));
//              continue;
//            }
//            protocol |= (pe->p_proto << 0x10);
//         }

// not used (no mask in statistic)
//         if ((tmp = strchr(uname, '/')))
//         {  *tmp = '\0';
//             mask = tmp + 1;
//           if (*mask == '\0')
//           mask = "32";
//       }
//       else
// (stub)
         mask = "32";

         if (cnt != 0)
         {  while(1)
            {
#ifdef DEBUG
            printf("res %s %s/%s %d %lu %s %u\n", resname, uname,
                    mask, cnt, protocol, uname, localport);
#else

#ifdef DUMP_JOB
fprintf(stderr, "res %s %s/%s %d %lu %s %u\n", resname, uname,
                    mask, cnt, protocol, uname, localport);
#endif

               link_puts(&lnk, "res %s %s/%s %d %lu %s %u", resname, uname,
                    mask, cnt, protocol, uname, localport);
               rc = answait(&lnk, RET_SUCCESS, linbuf, sizeof(linbuf), &msg);
#ifdef DUMP_JOB
fprintf(stderr, "BEE: %03d\n", rc);
#endif
               if (rc != RET_SUCCESS)
               {  if (rc == LINK_DOWN)
                  {  fprintf(stderr, "Unexpected link down\n");
                     exit(-1);
                  }
                  if (rc == LINK_ERROR) perror("Link error");

// not found in table - swap addresses & invert direction
                  if (rc == 404 && uname == from)
                  {  uname = to;
                     protocol = 0;  // inbound traffic
                     continue;
                  }
                  if (rc >= 400) fprintf(stderr, "Billing error (%d): %s "
                                                 "(%s->%s)\n",
                                                 rc, msg, from, to);
               }
               break;
            }
#endif  /* DEBUG else */
         } // if counter not zero 
      } // not too long line 
   } // main cycle

#ifndef DEBUG
#ifdef DUMP_JOB
fprintf(stderr, "TERMINATING\n");
#endif
   if (fUpdate)
   {  link_puts(&lnk, "update");
      rc=answait(&lnk, RET_SUCCESS, linbuf, sizeof(linbuf), &msg);
      if (rc != RET_SUCCESS)
      {  if (rc == LINK_DOWN) fprintf(stderr, "Unexpected link down\n");
         if (rc == LINK_ERROR) perror("Link error");
         if (rc >= 400) fprintf(stderr, "Billing error : %s\n", msg);
         exit(-1);
      }
   }
   link_puts (&lnk, "exit");
   link_close(&lnk);
#endif
   return 0;
}

void usage(int rc)
{
   fprintf(stderr, "%s",
" beecisco - ipstatd Cisco-like statistic parser\n"
"     Usage: dumstat stat | beecisco [<switches>]\n"
" r - resource name          (default - inet)\n"
" a - daemon host address    (compiled-in default)\n"
" A - daemon tcp port number (compiled-in default)\n"
" n - Exclude given dest address (do not count)\n"
" N - Include given dest address (do count)\n"
" u - pass update command    (default - no)\n");
   exit(rc);
}

// Address compare related functions

unsigned long make_addr_mask(int bits)
{  return swap32(~((1L << (32-bits))-1));
}

int make_addr(const char * straddr, unsigned long * addr, int * bits)
{  char   buf[32];
   char * ptr=buf;
   char * str;

   strlcpy(buf, straddr, sizeof(buf));
   str=next_token(&ptr, "/");
   if (str==NULL) return (-1);
   if (inet_aton(str, (struct in_addr *)addr) != 1) return (-1);
   str=next_token(&ptr, "/");
   if (str != NULL) *bits=strtol(str, NULL, 0);
   else *bits=32;
   *addr &= make_addr_mask(*bits);
   return 0;
}

// Compare user inet address in form n.n.n.n[/b]
// with resource link address

int inaddr_cmp(char * user, char * link)
{  unsigned long   uaddr;
   int             ubits;
   unsigned long   laddr;
   int             lbits;

   if (make_addr(link, &laddr, &lbits)==(-1)) return (-1);
   if (make_addr(user, &uaddr, &ubits)==(-1)) return (-1);
   if (ubits<lbits) return 1;
   if (ubits>lbits) uaddr &= make_addr_mask(lbits);
   return (uaddr != laddr);  // zero if equal
}

