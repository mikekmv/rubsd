/* $Bee$ */

#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>

#include <bee.h>
#include <res.h>
#include <ipc.h>
#include <beetraff.h>

char * res_host="any";
char * resname ="inet";

static char  buf[MAX_IPFSTAT_STRLEN];
link_t	     lnk;
char       * host = BEE_ADDR; 
int          port = BEE_SERVICE;

extern char * optarg;
void usage(int rc);

int main(int argc, char ** argv)
{
   char  * tok = NULL;
   char  * p, * count, * proto, * from, * to, * fromport, * toport;
   char c;
   int               ff;
   int               tf;
   char            * mask, * tmp, * uname;
   unsigned long     protocol;
   unsigned short    localport;
   struct protoent * pe;
   int               rc;
   int               int_count;
   int               inb_summ = 0;       // summary count inbound
   int               out_summ = 0;       // summary count outbound
   int               w;
   char            * msg;
   char              linbuf[128];
   int               fUpdate=0;
/* 
 *  0-nothing (looking for identifier), 
 *  1-count,
 *  2-proto,
 *  3- from,
 *  4-to,
 *  5-<skip "=">,
 *  6-fromport,
 *  7-toport;
 */

/*
   TODO:
 r  1. Redefine resource name
 a  2. Redefine daemon address
 A  3. Redefine daemon port
 d  4. Redefine destination address
 u  5. Send update command to billing switch
*/

#define OPTS "r:a:A:d:u"
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
         case 'd':
           res_host=optarg;
           break;
         case 'u':
           fUpdate=1;
           break;
         default:
           usage(-1);         
      }
   }


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
      
   while (fgets(buf, sizeof(buf), stdin))
   {  if (!(p = strchr(buf, '\n'))) 
           fprintf(stderr, "line too long: %s\n", buf);
      else
      {	 count = proto = from = to = fromport = toport = mask = NULL;
	 tok = buf;
	 localport = 0;
	 w = 1;			/* waiting for count */
	 do
	 {  p = next_token(&tok, IPFSTAT_DELIM);
	    if (p != NULL)
	    {  switch (w)
	       {
	       case 0:		/* nothing to wait */
		  if (!strcmp("proto", p))
		     w = 2;
		  if (!strcmp("from", p))
		     w = 3;
		  if (!strcmp("to", p))
		     w = 4;
		  if (!strcmp("port", p))
		     w = 5;
		  break;
	       case 1:
		  count = p;
		  w = 0;
		  break;
	       case 2:
		  proto = p;
		  w = 0;
		  break;
	       case 3:
		  from = p;
		  w = 0;
		  break;
	       case 4:
		  to = p;
		  w = 0;
		  break;
	       case 5:
		  w = to == NULL ? 6 : 7;
		  break;
	       case 6:
		  fromport = p;
		  w = 0;
		  break;
	       case 7:
		  toport = p;
		  w = 0;
		  break;
	       }
	    }
	 } while (p != NULL);

         if (from == NULL || to == NULL) continue; 

	 ff = !strcmp(res_host, from);
	 tf = !strcmp(res_host, to);

	 if (ff == tf)
	 {  fprintf(stderr, "unprocessible rule: from %s to %s\n", from, to);
	    continue;
	 }
	 if (tf)
	 {  uname = from;
	    protocol = 0x80000000;	/* out */
	    if (toport != NULL) protocol |= strtoul(toport, NULL, 0);
	    if (fromport != NULL) localport = strtoul(fromport, NULL, 0);
	 } else
	 {
	    uname = to;
	    protocol = 0;	/* in */
	    if (fromport != NULL)
	       protocol |= strtoul(fromport, NULL, 0);
	    if (toport != NULL)
	       localport = strtoul(toport, NULL, 0);
	 }

	 if (proto != NULL && strcmp("tcp/udp", proto))
	 {
	    if (!(pe = getprotobyname(proto)))
	    {
	       fprintf(stderr, "unable determine protocol: %s, %s\n", proto, strerror(errno));
	       continue;
	    }
	    protocol |= (pe->p_proto << 0x10);
	 }
	 if ((tmp = strchr(uname, '/')))
	 {
	    *tmp = '\0';
	    mask = tmp + 1;
	    if (*mask == '\0')
	       mask = "32";
	 } else
	    mask = "32";

         int_count=strtol(count, NULL, 0);
         
         // if no protocols defined - decrease count to other protocols
         // count sum (for same direction) 
         if ((protocol & (~0x80000000))==0 && fromport==NULL &&
                                             toport==NULL)
	 {  if (protocol & 0x80000000)  // if outband
            {  int_count -= out_summ;
               out_summ=0;
            }
            else
            {  int_count -= inb_summ;
               inb_summ=0;
            }
         }
         else
	 {  if (protocol & 0x80000000) out_summ += int_count;
            else inb_summ += int_count;
         }

         if (int_count == 0) continue;

//	 printf("res %s name %s/%s %d %lu %s %u\n", resname, uname,
//	            mask, int_count, protocol, uname, localport);
	 link_puts(&lnk, "res %s name %s/%s %d %lu %s %u", resname, uname,
	            mask, int_count, protocol, uname, localport);
         rc=answait(&lnk, RET_SUCCESS, linbuf, sizeof(linbuf), &msg);
         if (rc != RET_SUCCESS)
         {  if (rc == LINK_DOWN) 
            {  fprintf(stderr, "Unexpected link down\n");
               exit(-1);
            }
            if (rc == LINK_ERROR) perror("Link error");
            if (rc >= 400) fprintf(stderr, "Billing error : %s\n", msg);
         }
      }
   }

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
   return 0;
}

void usage(int rc)
{
   fprintf(stderr, "%s",
" beetraff - IPF \"count\" statistic parser\n"
"     Usage: ipfstat -aio | beetraff [<switches>]\n"
" r - resource name          (default - inet)\n"
" a - daemon host address    (compiled-in default)\n"
" A - daemon tcp port number (compiled-in default)\n"
" d - destination address    (default - \"any\")\n"
" u - pass update command    (default - no)\n");
   exit(rc);
}


