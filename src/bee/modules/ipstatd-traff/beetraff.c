/* $RuOBSD: beetraff.c,v 1.4 2002/08/06 05:14:03 shadow Exp $ */

//#define DEBUG
#define DUMP_JOB

#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>

#include <bee.h>
#include <res.h>
#include <ipc.h>
#include <beetraff.h>

#define EXCLUSIONS 8

char * resname ="inet";

static char  buf[MAX_STRLEN];
link_t	     lnk;
char       * host = BEE_ADDR;
int          port = BEE_SERVICE;
// Exclusions list
char       * exclist[EXCLUSIONS];
int	     exclcount=0;

extern char * optarg;
void usage(int rc);

int main(int argc, char ** argv)
{
   char  * tok = NULL;
   char  * p, * count, * proto, * from, * to, * fromport, * toport;
   char  * incount, * outcount;
   char c;
   int               incnt, outcnt;

   char            * mask, * tmp, * uname;
   unsigned long     protocol;
   unsigned short    localport;
   struct protoent * pe;
#ifndef DEBUG
   int               rc;
   char            * msg;
   char              linbuf[128];
   int               i1, i2;
#endif
   int               fUpdate=0;
   int               i;
   int               temp;

   temp=temp;
/*
 r  1. Redefine resource name
 a  2. Redefine daemon address
 A  3. Redefine daemon port
 u  4. Send update command to billing switch
 n  5. Exclude dest address
*/

#define OPTS "r:a:A:un:"
   while ((c=getopt(argc, argv, OPTS)) != -1)
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
	   if (exclcount < EXCLUSIONS) exclist[exclcount++] = optarg;
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

   rc=answait(&lnk, RET_SUCCESS, linbuf, sizeof(linbuf), &msg);
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
#ifdef DUMP_JOB
fprintf(stderr, "BEE: %03d\n", rc);
#endif
   rc=answait(&lnk, RET_SUCCESS, linbuf, sizeof(linbuf), &msg);
   if (rc != RET_SUCCESS)
   {  if (rc == LINK_DOWN) fprintf(stderr, "Unexpected link down\n");
      if (rc == LINK_ERROR) perror("Link error");
      if (rc >= 400) fprintf(stderr, "Billing error : %s\n", msg);
      exit(-1);
   }
#endif

   while (fgets(buf, sizeof(buf), stdin))
   {  
#ifdef DUMP_JOB
fprintf(stderr, "STR: %s\n", buf);
#endif
      if (!(p = strchr(buf, '\n')))
           fprintf(stderr, "line too long: %s\n", buf);
      else
      {  if (*buf > '9' || *buf < '0') 
         { 
#ifdef DUMP_JOB
fprintf(stderr, "SKIPPED - header\n");
#endif
             continue;  // skip header lines
         }

         count = proto = from = to = fromport = toport = mask = NULL;
	 tok = buf;
	 localport = 0;


	 p = next_token(&tok, IPFSTAT_DELIM);
         if (p == NULL) continue;
         from = p;

	 p = next_token(&tok, IPFSTAT_DELIM);
         if (p == NULL) continue;
         to = p;

	 p = next_token(&tok, IPFSTAT_DELIM);
         if (p == NULL) continue;
         outcount = p;

	 p = next_token(&tok, IPFSTAT_DELIM);
         if (p == NULL) continue;
         incount = p;


//         for (i=0; i<exclcount; i++)
//            if (inaddr_cmp(to, exclist[i]) == 0 ||
//                inaddr_cmp(from, exclist[i]) == 0) break;
//         if (i < exclcount) continue; // if broken cycle

         for (i=0; i<exclcount; i++)
            if (inaddr_cmp(to, exclist[i]) == 0) break; 
         if (i < exclcount) // if broken cycle
         {  
#ifdef DUMP_JOB
            i1 = i;
#endif
            for (i=0; i<exclcount; i++)
               if (inaddr_cmp(from, exclist[i]) == 0) break; 
            if (i < exclcount) // if broken cycle
            {
#ifdef DUMP_JOB
            i2 = i;
fprintf(stderr, "local - masks: %s -> %s\n", exclist[i1], exclist[i2]);
#endif
               continue; 
            }
         }
         incnt  = strtol(incount, NULL, 10);
         outcnt = strtol(outcount, NULL, 10);

         uname = from;
         protocol = 0;	/* in */
         if (toport != NULL) protocol |= strtoul(toport, NULL, 0);
         if (fromport != NULL) localport = strtoul(fromport,NULL, 0);
         if (proto != NULL && strcmp("tcp/udp", proto))
         {  if (!(pe = getprotobyname(proto)))
            {  fprintf(stderr, "unable determine protocol: %s, %s\n",
               proto, strerror(errno));
               continue;
            }
            protocol |= (pe->p_proto << 0x10);
         }

         if ((tmp = strchr(uname, '/')))
         {  *tmp = '\0';
             mask = tmp + 1;
	     if (*mask == '\0')
	     mask = "32";
	 }
         else mask = "32";

         if (incnt != 0)
         {
            while(1)
	    {  
#ifdef DEBUG
 	    printf("res %s %s/%s %d %lu %s %u\n", resname, uname,
	            mask, incnt, protocol, uname, localport);
#else

#ifdef DUMP_JOB
fprintf(stderr, "res %s %s/%s %d %lu %s %u\n", resname, uname,
	            mask, incnt, protocol, uname, localport);
#endif

               link_puts(&lnk, "res %s %s/%s %d %lu %s %u", resname, uname,
	            mask, incnt, protocol, uname, localport);
               rc=answait(&lnk, RET_SUCCESS, linbuf, sizeof(linbuf), &msg);
#ifdef DUMP_JOB
fprintf(stderr, "BEE: %03d\n", rc);
#endif
               if (rc != RET_SUCCESS)
               {  if (rc == LINK_DOWN)
                  {  fprintf(stderr, "Unexpected link down\n");
                     exit(-1);
                  }
                  if (rc == LINK_ERROR) perror("Link error");
               
                  if (rc == 404 && uname == from)
                  {  uname = to;
// do not swap counts
//                     temp   = incnt;
//                     incnt  = outcnt;
//                     outcnt = temp;                      
                     continue;
                  }
                  if (rc >= 400) fprintf(stderr, "Billing error (%d): %s "
                                                 "(%s->%s)\n",
                                                 rc, msg, from, to);
               }
                  break;  
            }
#endif
         }

         if (outcnt != 0)
         {
	    protocol |= 0x80000000;	/* out */
#ifdef DEBUG
	    printf("res %s %s/%s %d %lu %s %u\n", resname, uname,
	            mask, outcnt, protocol, uname, localport);
#else

#ifdef DUMP_JOB
fprintf(stderr, "res %s %s/%s %d %lu %s %u\n", resname, uname,
	            mask, outcnt, protocol, uname, localport);
#endif

	    link_puts(&lnk, "res %s %s/%s %d %lu %s %u", resname, uname,
	            mask, outcnt, protocol, uname, localport);
            rc=answait(&lnk, RET_SUCCESS, linbuf, sizeof(linbuf), &msg);
#ifdef DUMP_JOB
fprintf(stderr, "BEE: %03d\n", rc);
#endif
            if (rc != RET_SUCCESS)
            {  if (rc == LINK_DOWN)
               {  fprintf(stderr, "Unexpected link down\n");
                  exit(-1);
               }
               if (rc == LINK_ERROR) perror("Link error");
               if (rc >= 400) fprintf(stderr, "Billing error (%d): %s "
                                                 "(%s->%s)\n",
                                                 rc, msg, from, to);
               if (rc >= 400) fprintf(stderr, "Billing error (%d): %s (%s)\n",
			       rc, msg, uname);

            }
#endif
         }
      }
   }


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
" beetraff-pf - PFLOGD statistic parser\n"
"     Usage: ipfstat -aio | beetraff [<switches>]\n"
" r - resource name          (default - inet)\n"
" a - daemon host address    (compiled-in default)\n"
" A - daemon tcp port number (compiled-in default)\n"
" n - Exclude given dest address (do not count)\n"
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

