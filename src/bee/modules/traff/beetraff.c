/* $RuOBSD: beetraff.c,v 1.14 2008/11/27 10:47:54 shadow Exp $ */

// Hack to output traffic statistics for SQL
//#define SQLSTAT_HACK

// DUMP JOB - copy commands to stderr
//#define DUMP_JOB

#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include <bee.h>
#include <db.h>
#include <res.h>
#include <links.h>
#include <ipc.h>

#include "beetraff.h"

#define EXCLUSIONS 1024

char   * resname = "inet";        // Resource name
char   * host    = NULL;          // core address
int      port    = 0;             // core port
link_t	 lnk;                     // beeipc link

int	 fLock    = 0;            // lock DB flag
int      fUpdate  = 0;            // send update flag
int      fCnupm   = 1;            // using cnupm instead of ipstatd
int      fNoDetail= 0;            // use gate addresses instead of original (perfomance hack)
int      fAccBased= 0;            // account-based matching (perfomance hack)
char *   filename = NULL;         // Input filename

char *   outfile  = NULL;         // Output filename (filtered statistics)

time_t   hack_time = 0;

// Exclusions list
exclitem_t  * itm_exclist = NULL;
int	      cnt_exclist = 0;

// Statistics list
accsum_t   * itm_statlist = NULL;
int          cnt_statlist = 0;

char     buf[MAX_STRLEN];
char     addrbuf[32];
char     addrbuf2[32];
char     linbuf[128];

/* * * * * * * * * * * * * * *\
 *   Display program usage   *
\* * * * * * * * * * * * * * */

void usage(int rc)
{
   fprintf(stderr, "%s",
" beetraff - traffic collector (cnupm or ipstatd) dump parser\n"
"     Usage: bee-cnupmdump.sh | beetraff [<switches>]\n"
"            dumpstat stat | beetraff -c [<switches>]\n"
" r - resource name                 (default - inet)\n"
" a - daemon host address           (compiled-in default)\n"
" A - daemon tcp port number        (compiled-in default)\n"
" n - Exclude given dest address    (do not count)\n"
" N - Include given dest address    (do count)\n"
" u - pass update command           (default - no)\n"
" l - pass lock command             (default - no)\n"
" c - use ipstatd dump              (default - cnupm)\n"
" f - input file                    (default - stdin)\n"
" o - output file (SQLSTAT hack)    (no by default)\n");

   exit(rc);
}

/* * * * * * * * * * * * * * *\
 *       Main  function      *
\* * * * * * * * * * * * * * */

int main(int argc, char ** argv)
{
   FILE      * f  = stdin;
#ifdef SQLSTAT_HACK
   FILE      * of = NULL;
#endif /* SQLSTAT_HACK */
   char      * str;
   char      * p;
   char      * p2;
   char      * msg;
   u_int       from;
   u_int       to;
   u_int       count;
   int         fromport;
   int         toport;
   int         proto;
   char        c;
   int         n;
   int         i;
   int         rc;
   int         flag_from;
   int         flag_to;

   int         n_from = (-1);
   int         n_to   = (-1);

   exclitem_t  exclusion;
   accsum_t    statitem;

   time_t      curtime;
   struct tm   stm;

/*
 r  1. Redefine resource name
 a  2. Redefine daemon address
 A  3. Redefine daemon port
 u  4. Send update command to billing switch
 n  5. Exclude dest address
 N  6. Include dest address
 l  7. Lock database (undesirable)
*/

// Prepare timestampt for output file
   curtime = time(NULL) - 300;
   localtime_r(&curtime, &stm);

// Load bee config
   rc = conf_load(NULL);
   if (rc < 0)
   {  fprintf(stderr, "ERROR - Configuration loading failure\n");
      exit(-1);
   }
   host = conf_coreaddr;
   port = conf_coreport;

   while ((c = getopt(argc, argv, OPTS)) != -1)
   {  switch (c)
      {  case 'r':            // resorce name
            resname = optarg;
            break;

         case 'a':            // core address
            host = optarg;
            break;

         case 'A':            // core tcp port
            port = strtol(optarg, NULL, 10);
            break;

         case 'u':            // send update command flag
            fUpdate = 1;
            break;

         case 'n':
         case 'N':
	    if (cnt_exclist < EXCLUSIONS) 
            {  bzero(&exclusion, sizeof(exclusion));

               rc = make_addrandmask (optarg, &(exclusion.addr), &(exclusion.mask));
               if (rc < 0)
               {  fprintf(stderr, "ERROR - Exclusion \"-%c %s\" parse error\n", 
                          c, optarg);
                  break;
               }

               if (c == 'N') exclusion.flag = ITMF_COUNT;

               rc = da_ins(&cnt_exclist, &itm_exclist, sizeof(*itm_exclist), (-1), &exclusion);
               if (rc < 0)
               {  fprintf(stderr, "ERROR - Exclusion \"-%c %s\" system error\n",
                          c, optarg);
               }
            }
            else
            {  fprintf(stderr, "ERROR - Overflow, exclusion \"-%c %s\" skipped\n",
                       c, optarg);
               break;
            }
            break;

         case 'l':
            fLock = 1;
            break;

         case 'f':
            filename = optarg;
            break;

         case 'o':
            outfile = optarg;
            break;

         case 'c':
            fCnupm = !fCnupm;
            break;

         case 'h':
            hack_time = strtol(optarg, NULL, 10);
            break;

         case 'd':
            fNoDetail = !fNoDetail;
            break;

         case 'D':
            fAccBased = !fAccBased; // toggle flag
            fNoDetail = fAccBased;  // force "no detail" mode
            break;

         default:
            usage(-1);
      }
   }

#ifdef SQLSTAT_HACK
// Open output file if any given (ignore error)
   if (outfile != NULL) of = fopen(outfile, "a");
#endif /* SQLSTAT_HACK */

// Load gates
   rc = reslinks_load(LOCK_SH);
   if (rc < 0)
   {  fprintf(stderr, "ERROR - Gates loading failure\n");
      exit(-1);  
   }

// Open input file (if any)
   if (filename != NULL)
   {  f = fopen(filename, "r");
      if (f == NULL)
      {  fprintf(stderr, "ERROR - Unable to open input \"%s\"\n", filename);
         exit(-1);  
      }
   }


// Input parse cycle
   while (fgets(buf, sizeof(buf), f))
   {

#ifdef DUMP_JOB
      fprintf(stderr, "STR: %s", buf);
#endif

      if (!(p = strchr(buf, '\n')))     // long line
      {
#ifdef DUMP_JOB
         fprintf(stderr, "\n");         // appending "\n"
#endif
         fprintf(stderr, "line too long: %s\n", buf);
         continue; 
      }

      if (*buf > '9' || *buf < '0')     // if first char is not digit
      {
#ifdef DUMP_JOB
         fprintf(stderr, "SKIPPED - header\n");
#endif
         continue;  // skip header lines
      }

      str = buf;

      if (fCnupm != 0)
      {  p = next_token(&str, IPFSTAT_DELIM);  // start date (skip)
         if (p == NULL) continue;
         p = next_token(&str, IPFSTAT_DELIM);  // start time (skip)
         if (p == NULL) continue;
      }

      p = next_token(&str, IPFSTAT_DELIM);     // from addr
      if (p == NULL) continue;
      p2 = next_token(&p, ":");
      if (p2 == NULL) continue;      
      rc = inet_pton(AF_INET, p2, &from); 
      if (rc < 0) continue;

      fromport = (-1);
      if (fCnupm)                         // from port (cnupm only)
      {  p2 = next_token(&p, ":");
         if (p2 != NULL)
         {  fromport = strtol(p2, NULL, 10);
         } 
      }

      p = next_token(&str, IPFSTAT_DELIM);     // to addr
      if (p == NULL) continue;
      p2 = next_token(&p, ":");
      if (p2 == NULL) continue;
      rc = inet_pton(AF_INET, p2, &to);
      if (rc < 0) continue;

      toport = (-1);
      if (fCnupm)                         // to port (cnupm only)
      {  p2 = next_token(&p, ":");
         if (p2 != NULL)
         {  toport = strtol(p2, NULL, 10);
         }
      }

      proto = (-1);
      p = next_token(&str, IPFSTAT_DELIM);     // packets/proto
      if (p == NULL) continue;
      if (fCnupm != 0)
      {  proto = strtol(p, NULL, 10);
      }

      p = next_token(&str, IPFSTAT_DELIM);     // bytes
      if (p == NULL) continue;
      count = strtoul(p, NULL, 10);

      if (count == 0) continue;

// Filter excluded traffic (i.e. excluded to excluded)

      flag_to   = 1;   // i.e. global by default
      flag_from = 1;

      for (i=0; i < cnt_exclist; i++)
      {  if ((to   & itm_exclist[i].mask) == itm_exclist[i].addr)
            flag_to   = (itm_exclist[i].flag & ITMF_COUNT) != 0;
         if ((from & itm_exclist[i].mask) == itm_exclist[i].addr)
            flag_from = (itm_exclist[i].flag & ITMF_COUNT) != 0;
      }
      if ((flag_to | flag_from) == 0)
      {
#ifdef DUMP_JOB
         fprintf(stderr, "LOCAL -  %s -> %s\n", 
             inet_ntop(AF_INET, &from, addrbuf, sizeof(addrbuf)),
             inet_ntop(AF_INET, &to, addrbuf2, sizeof(addrbuf2)));
#endif
         continue;
      }

// Gate filter (skip unknown or client -> client)      
      flag_to   = 0;  // unknown by default
      flag_from = 0;

      n = (-1); rc = lookup_baddr(to, &n);
      if (rc >= 0)
      {  flag_to = 1;
         n_to    = n;
      } 

      n = (-1); rc = lookup_baddr(from, &n);
      if (rc >= 0)
      {  flag_from = 1;
         n_from    = n;
      }

      if ((flag_from | flag_to) == 0) 
      {  fprintf(stderr, "UNKNOWN -  %s -> %s\n", 
             inet_ntop(AF_INET, &from, addrbuf, sizeof(addrbuf)),
             inet_ntop(AF_INET, &to, addrbuf2, sizeof(addrbuf2)));
         continue;
      } 
      
      if (flag_from == flag_to) 
      {  fprintf(stderr, "INTERCLIENT -  %s -> %s\n", 
             inet_ntop(AF_INET, &from, addrbuf, sizeof(addrbuf)),
             inet_ntop(AF_INET, &to, addrbuf2, sizeof(addrbuf2)));
         continue;
      } 

// Fill stat item
      if (flag_from != 0)
      {  statitem.addr  = fNoDetail ? linktab[n_from].addr : from;
         statitem.in    = 0;
         statitem.out   = count;
         statitem.accno = linktab[n_from].accno;
      }
      else
      {  statitem.addr = fNoDetail ? linktab[n_to].addr : to;
         statitem.in   = count;
         statitem.out  = 0;
         statitem.accno = linktab[n_to].accno;
      }

#ifdef SQLSTAT_HACK
// Write filtered data to output file
      if (of != NULL)
      {  fprintf(of, "SELECT add_traffstat('%04d-%02d-%02d %02d:00:00', ", 
                     stm.tm_year + 1900,
                     stm.tm_mon + 1,
                     stm.tm_mday,
                     stm.tm_hour);
         fprintf(of, "'%s', ", inet_ntop(AF_INET, (flag_from ? &from : &to), addrbuf, sizeof(addrbuf)));
         fprintf(of, "'%s', ", inet_ntop(AF_INET, (flag_from ? &to : &from), addrbuf, sizeof(addrbuf)));
         if (flag_from) 
            fprintf(of, "0, %ld);\n", count);
         else
            fprintf(of, "%ld, 0);\n", count);    
      }
#endif /* SQLSTAT_HACK */


// Lookup matched stat item
      for (i=0; i<cnt_statlist; i++)
         if ((fAccBased != 0 && statitem.accno == itm_statlist[i].accno) ||
             (fAccBased == 0 && statitem.addr == itm_statlist[i].addr)) break;

      if (i < cnt_statlist)    // i.e. found
      {  itm_statlist[i].in  += statitem.in;
         itm_statlist[i].out += statitem.out;
#ifdef DUMP_JOB
         fprintf(stderr, "APPENDED\n");
#endif
      }  
      else
      {  rc = da_ins(&cnt_statlist, &itm_statlist, sizeof(*itm_statlist), (-1), &statitem);
         if (rc < 0)
         {  fprintf(stderr, "ERROR - Unable to insert  %s -> %s (%u bytes)\n", 
                inet_ntop(AF_INET, &from, addrbuf, sizeof(addrbuf)),
                inet_ntop(AF_INET, &to, addrbuf2, sizeof(addrbuf2)), count);
            continue;
         } 
#ifdef DUMP_JOB
         fprintf(stderr, "INSERTED\n");
#endif
      }
   } // Input parse cycle
   
// Send counts onto core   

#ifdef DUMP_JOB
   fprintf(stderr, "NEW SESSION\n");
#endif

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

   if (fLock)
   {
#ifdef DUMP_JOB
      fprintf(stderr, "lock\n");
#endif

      link_puts(&lnk, "lock");
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
   }

   if (hack_time)
   {
      fprintf(stderr, "_hacktime %d\n", hack_time);

      link_puts(&lnk, "_hacktime %d", hack_time);
      rc = answait(&lnk, RET_SUCCESS, linbuf, sizeof(linbuf), &msg);

      fprintf(stderr, "BEE: %03d\n", rc);

      if (rc != RET_SUCCESS)
      {  if (rc == LINK_DOWN) fprintf(stderr, "Unexpected link down\n");
         if (rc == LINK_ERROR) perror("Link error");
         if (rc >= 400) fprintf(stderr, "Billing error : %s\n", msg);
         exit(-1);
      }
   }

   for (i=0; i < cnt_statlist; i++)
   {  
      inet_ntop(AF_INET, &(itm_statlist[i].addr), addrbuf, sizeof(addrbuf));

#ifdef DUMP_JOB
      fprintf(stderr, "ITEM: %s in:%d out:%d\n", addrbuf,
              itm_statlist[i].in, itm_statlist[i].out);
#endif

//      if (itm_statlist[i].in  != 0)
      {  snprintf(buf, sizeof(buf), "bires %x %u %u %x",
                   itm_statlist[i].addr, itm_statlist[i].in, itm_statlist[i].out, fAccBased ? 0 : itm_statlist[i].addr);  
//      {  snprintf(buf, sizeof(buf), "res %s %s %u %u %s", resname,
//                   addrbuf, itm_statlist[i].in, 0, "0.0.0.0");  
// //                   addrbuf, itm_statlist[i].in, 0, addrbuf);  
#ifdef DUMP_JOB
         fprintf(stderr, "CMD: %s\n", buf);
#endif
         link_puts(&lnk, "%s", buf);

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
            if (rc >= 400) fprintf(stderr, "Billing error (%d): %s "
                                           "(%s)\n",
                                           rc, msg, addrbuf);
         }
      }

#ifdef NEVER

      if (itm_statlist[i].out != 0)
      {  snprintf(buf, sizeof(buf), "res %s %s %u %u %s", resname,
                   addrbuf, itm_statlist[i].out, 0x80000000, "0.0.0.0");  
//                   addrbuf, itm_statlist[i].out, 0x80000000, addrbuf);  
#ifdef DUMP_JOB
         fprintf(stderr, "CMD: %s\n", buf);
#endif
         link_puts(&lnk, "%s", buf);

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
            if (rc >= 400) fprintf(stderr, "Billing error (%d): %s "
                                           "(%s)\n",
                                           rc, msg, addrbuf);
         }
      }
#endif

   }

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

   return 0;
}
