/* $RuOBSD: beetraff.c,v 1.1 2002/04/01 02:36:42 shadow Exp $ */

//#define DEBUG

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
#endif
   int               fUpdate=0;
   int               i;

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

#ifndef DEBUG
   rc = link_request(&lnk, host, port);
   if (rc == -1)
   {  perror("Can't connect to billing service");
      exit(-1);
   }
   rc=answait(&lnk, RET_SUCCESS, linbuf, sizeof(linbuf), &msg);
   if (rc != RET_SUCCESS)
   {  if (rc == LINK_DOWN) fprintf(stderr, "Unexpected link down\n");
      if (rc == LINK_ERROR) perror("Link error");
      if (rc >= 400) fprintf(stderr, "Billing error : %s\n", msg);
      exit(-1);
   }

   link_puts(&lnk, "machine");
   rc=answait(&lnk, RET_SUCCESS, linbuf, sizeof(linbuf), &msg);
   if (rc != RET_SUCCESS)
   {  if (rc == LINK_DOWN) fprintf(stderr, "Unexpected link down\n");
      if (rc == LINK_ERROR) perror("Link error");
      if (rc >= 400) fprintf(stderr, "Billing error : %s\n", msg);
      exit(-1);
   }
#endif

   while (fgets(buf, sizeof(buf), stdin))
   {  if (!(p = strchr(buf, '\n')))
           fprintf(stderr, "line too long: %s\n", buf);
      else
      {  if (*buf > '9' || *buf < '0') continue;  // skip header lines

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


         for (i=0; i<exclcount; i++)
            if (strcmp(to, exclist[i]) == 0) break;
         if (i < exclcount) continue; // if broken cycle

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
#ifdef DEBUG
 	    printf("res %s %s/%s %d %lu %s %u\n", resname, uname,
	            mask, incnt, protocol, uname, localport);
#else
	    link_puts(&lnk, "res %s %s/%s %d %lu %s %u", resname, uname,
	            mask, incnt, protocol, uname, localport);
            rc=answait(&lnk, RET_SUCCESS, linbuf, sizeof(linbuf), &msg);
            if (rc != RET_SUCCESS)
            {  if (rc == LINK_DOWN)
               {  fprintf(stderr, "Unexpected link down\n");
                  exit(-1);
               }
               if (rc == LINK_ERROR) perror("Link error");
               if (rc >= 400) fprintf(stderr, "Billing error : %s\n", msg);
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
	    link_puts(&lnk, "res %s %s/%s %d %lu %s %u", resname, uname,
	            mask, outcnt, protocol, uname, localport);
            rc=answait(&lnk, RET_SUCCESS, linbuf, sizeof(linbuf), &msg);
            if (rc != RET_SUCCESS)
            {  if (rc == LINK_DOWN)
               {  fprintf(stderr, "Unexpected link down\n");
                  exit(-1);
               }
               if (rc == LINK_ERROR) perror("Link error");
               if (rc >= 400) fprintf(stderr, "Billing error : %s\n", msg);
            }
#endif
         }
      }
   }
#ifndef DEBUG
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


