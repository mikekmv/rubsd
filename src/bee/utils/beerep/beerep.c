#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/ioctl.h>


#include <bee.h>
#include <db.h>
#include <res.h>
#include <links.h>
#include <logidx.h>

#include "beerep.h"

#ifndef MAXRECS
#define MAXRECS 32768
#endif

#define ONE_DAY (3600*24)

int         off_flags = 0;

long long   first = 0;
long long   last  = 0;

char      * logname = NULL;
logbase_t   Logbase;
long long   recs;

char      * idxname = NULL;

logrec_t    logrec;
logrec_t    prnrec;

char      * templ="DTABRCSEIHPst";
/*
 "DTABRCSE"
  Date, time, accno, balance, res, count, sum, result
 */

int         acc_cnt = 0;
acclist_t   acc_list[MAXRECS];


char       buf[256];
char       titlebuf[256];

int        fAll     = 0;
int        fIn      = 1;
int        fOut     = 1;
int        fCharge  = 1;
int        fIDX     = 0;
int        fLine    = 0;
int        fNoZeros = 0;
int        fNoBytes = 0;
int        fStdIn   = 1;
int        fNumbers = 0;

int        fCached  = 0;

int        SkipAcc  = (-1);

int        MaxSpeed = 1024; // (hack) too high speed 
u_int      filter_addr = 0;  // default - all
in_addr_t  filter_mask = 0;  // default - all

char     * HeadTemplFile = NULL;
char     * BodyTemplFile = NULL;

char     * HeadTempl = NULL;
char     * BodyTemplPre  = NULL;
char     * BodyTemplPost = NULL;

idxhead_t  idxhead;
long long  idxstart = 0;
long long  idxstop  = 0;

char       tabopts_def[]  = "border=0 cellspacing=1 cellpadding=4 class=\"txt\"";
char       headopts_def[] = "align=center bgcolor=#888888";
char       cellopts_def[] = "bgcolor=#eeeeee";
char       bodyopts_def[] = "bgcolor=white";

char     * dumpfile = NULL;
FILE     * dumpf = NULL;

int main(int argc, char ** argv)
{  long long    rc;
   tformat_t    tform;

   int          fsum    = 0;
   int          headers = 1;
   long long    i;
   int          i2, n;

   u_int64_t    wsc = 0;
   u_int64_t    sc  = 0;
   long double  wsm = 0;
   long double  sm  = 0;

   char       * ptr = NULL;
   char       * str;

   int          fd = (-1);
   char       * ptmpl = NULL;

   int          days = 0;
   struct tm    stmf;
   struct tm    stmt;

// Load configuration
   rc = conf_load(NULL);
   if (rc < 0)
   {  fprintf(stderr, "ERROR - Config loading failure\n");
      exit(-1);
   }

// Load gates
   rc = reslinks_load (LOCK_SH);
   if (rc < 0)
   {  fprintf(stderr, "ERROR - Gates loading failure\n");
      exit(-1);
   }

// Set defaults to config values
   logname = conf_logfile;
   idxname = conf_logindex;

// Initialize table format
   memset(&tform, 0, sizeof(tform));

   tform.tabopts   = tabopts_def;
   tform.headopts  = headopts_def;
   tform.cellopts  = cellopts_def;
   tform.bodyopts  = bodyopts_def;

#define PARAMS "a:Ab:B:c:C:dD:E:F:fghH:il:LM:mNn:or:RsS:t:T:x:z"

   while ((rc = getopt(argc, argv, PARAMS)) != -1)
   {
      switch (rc)
      {
         case 'm':
            fNoBytes = 1;
            break;

         case 'c':
            tform.tabopts = optarg;
            break;

         case 'C':
            tform.cellopts = optarg;
            break;

         case 'H':
            tform.headopts = optarg;
            break;

         case 'E':
            HeadTemplFile = optarg;
            break;

         case 'B':
            BodyTemplFile = optarg;
            break;

         case 'b':
            tform.bodyopts = optarg;
            break;

         case 'F':
            tform.from = parse_time(optarg);
            break;

         case 'T':
            tform.to = parse_time(optarg);
            break;

         case 'n':
            days = strtol(optarg, NULL, 10);
            break;

         case 'r':
            tform.res = strtol(optarg, NULL, 10);
            break;

         case 'R':
            tform.res = -1;
            break;

         case 'g':
            tform.flags |= FLAG_DIRGROUP;
            break;

         case 's':
            fsum = 1;
            break;

         case 't':
            tform.fields = optarg;
            break;

         case 'h':
            headers = 0;
            break;

         case 'a':
            fStdIn = 0;
            if (acc_cnt < MAXRECS)
            {  acc_list[acc_cnt].accno = strtol(optarg, &ptr, 10);
               acc_list[acc_cnt].count_in  = 0; 
               acc_list[acc_cnt].count_out = 0; 
               acc_list[acc_cnt].money_in  = 0; 
               acc_list[acc_cnt].money_out = 0; 
               acc_list[acc_cnt].money_charge = 0; 
               acc_list[acc_cnt].pays      = 0; 
               acc_list[acc_cnt].in_recs   = 0; 
               acc_list[acc_cnt].out_recs  = 0; 
               if (acc_list[acc_cnt].accno >= 0)
               {  acc_cnt++;
                  if (ptr != NULL)
                  {  if (*ptr != '\0') ptr++;
                     if (*ptr != '\0')
                     {  asprintf(&(acc_list[acc_cnt-1].descr), "%s", ptr);
                     }
                  } 
               }
            }
            else
            {  syslog(LOG_ERR, "FATAL: account table overflow");
               fprintf(stderr, "FATAL: account table overflow\n");  
               exit(-1);
            } 
            break;

         case 'A':
            fAll = 1;
            break;

         case 'i':
            fIn = 0;
            break;

         case 'o':
            fOut = 0;
            break;

         case 'f':
            fCharge = 0;
            break;

         case 'd':
            fIDX = 1;
            break;

         case 'z':
            fNoZeros = 1;   // no break

         case 'L':
            fLine = 1;
            tform.flags |= FLAG_DIRGROUP;
            break;

         case 'S':
            SkipAcc = strtol(optarg, NULL, 10);
            break;

         case 'l':         // load list
            i = (-1);
            n = acc_cnt;
            while (lookup_resname(RES_LIST, optarg, &i2) >= 0)
            {  if (acc_cnt < MAXRECS)
               {  acc_list[acc_cnt].accno     = linktab[i2].accno;
                  acc_list[acc_cnt].count_in  = 0;
                  acc_list[acc_cnt].count_out = 0;
                  acc_list[acc_cnt].money_in  = 0;
                  acc_list[acc_cnt].money_out = 0;
                  acc_list[acc_cnt].money_charge = 0;
                  acc_list[acc_cnt].pays      = 0;
                  acc_list[acc_cnt].in_recs   = 0;
                  acc_list[acc_cnt].out_recs  = 0;
                  acc_cnt++;
               }
               else
                  fprintf(stderr, "ERROR: account table overflow\n");
            }
            for (; n < acc_cnt; n++)
            {  i2 = (-1);
               if ((rc = lookup_accres(acc_list[n].accno, RES_LABEL, &i2))<0)
               {  i2 = (-1);
                  if ((rc = lookup_accres(acc_list[n].accno, RES_LOGIN, &i2))<0)
                  {  i2 = (-1);
                     rc = lookup_accres(acc_list[n].accno, RES_ADDER, &i2);
                  }
               }
               if (rc >= 0) acc_list[n].descr = linktab[i2].username;
            } 
            break;

         case 'N':
            fNumbers = 1;
            break;

         case 'M':
            MaxSpeed = strtol(optarg, NULL, 10);
            break;

         case 'x':
            make_addrandmask (optarg, &filter_addr, &filter_mask);
            break;

         case 'D':
            dumpfile = optarg;
            break;

         default:
            usage();
            exit(-1); 
      }
   }

// Open dump file
   if (dumpfile)
   {  dumpf = fopen(dumpfile, "w");
   }


// Do nothing if invalid -a switch
   if (fStdIn == 0 && acc_cnt == 0)
   {  syslog(LOG_ERR, "nothing to do");
      exit(-1);
   }

// process -n switch (no of days)
   if (days > 0)
   {  if (tform.from == 0 || tform.to != 0)
      {  syslog(LOG_ERR, "-n switch requires -F, but no -T");
         exit(-1);
      }
      tform.to = tform.from + days * ONE_DAY;

// check & correct time if DST changed 
      localtime_r(&(tform.from), &stmf);
      localtime_r(&(tform.to),   &stmt);

      if (stmf.tm_hour != stmt.tm_hour ||
          stmf.tm_min  != stmt.tm_min  ||
          stmf.tm_sec  != stmt.tm_sec)
      {  n = (stmf.tm_hour * 3600 + stmf.tm_min * 60 + stmf.tm_sec) - 
             (stmt.tm_hour * 3600 + stmt.tm_min * 60 + stmt.tm_sec);
         if (abs(n) > (18*3600)) n -= (24*3600) * (n < 0 ? -1 : 1);

         tform.to += n; 
      } 
   }

// make opts
   if (tform.headopts == headopts_def && tform.cellopts != cellopts_def)
      tform.headopts = tform.cellopts;

// Load head template
   if (HeadTemplFile != NULL)
   {  fd = open(HeadTemplFile, O_RDONLY, 0);
      if (fd < 0)
      {  syslog(LOG_ERR, "open(%s): %m", HeadTemplFile);
         exit(-1);
      }
      rc = ioctl(fd, FIONREAD, &i2);
      if (rc < 0)
      {  syslog(LOG_ERR, "ioctl(%s): %m", HeadTemplFile);
         exit(-1);
      }
      HeadTempl = (char*)calloc(1, i2 + 1);
      if (HeadTempl == NULL)
      {  syslog(LOG_ERR, "calloc(%s): %m", HeadTemplFile);
         exit(-1);
      }
      rc = read(fd, HeadTempl, i2);
      if (rc < i2)
      {  syslog(LOG_ERR, "read(%s): Error", HeadTemplFile);
         exit(-1);
      }
      close(fd);
   }

// Load body template
   if (BodyTemplFile != NULL)
   {  fd = open(BodyTemplFile, O_RDONLY, 0);
      if (fd < 0)
      {  syslog(LOG_ERR, "open(%s): %m", BodyTemplFile);
         exit(-1);
      }
      rc = ioctl(fd, FIONREAD, &i2);
      if (rc < 0)
      {  syslog(LOG_ERR, "ioctl(%s): %m", BodyTemplFile);
         exit(-1);
      }
      BodyTemplPre = (char*)calloc(1, i2 + 1);
      if (BodyTemplPre == NULL)
      {  syslog(LOG_ERR, "calloc(%s): %m", BodyTemplFile);
         exit(-1);
      }
      rc = read(fd, BodyTemplPre, i2);
      if (rc < i2)
      {  syslog(LOG_ERR, "read(%s): Error", BodyTemplFile);
         exit(-1);
      }
      close(fd);
      BodyTemplPost = strstr(BodyTemplPre, "%BODY%");
      if (BodyTemplPost != NULL)
      {  *BodyTemplPost = '\0';
         BodyTemplPost  += 6;
      }
   }


// Count first/last record with idx file
   if (fIDX)
   {  while (1) // fictive cycle
      {  fd = open(idxname, O_RDONLY | O_SHLOCK, 0644);
         if (fd < 0)
         {  syslog(LOG_ERR, "open(%s): %m", idxname);
            break;  
         }
         rc = read(fd, &idxhead, sizeof(idxhead));
         if (rc < sizeof(idxhead))
         {  syslog(LOG_ERR, "read(idxhead): %m");
            break;
         }
         // check file marker
         if (memcmp(idxhead.marker, IDXMARKER, sizeof(idxhead.marker)) != 0) break;
         // read start index
         i = ((tform.from - idxhead.first) / ONE_DAY - 2) * sizeof(long long) +
             sizeof(idxhead);
         if (i > 0)
         {  rc = lseek(fd, i, SEEK_SET);
            if (rc == i)
            {  rc = read(fd, &idxstart, sizeof(idxstart));
               if (rc != sizeof(idxstart))  idxstart = 0; 
            }
         }
         // read stop index
         i = ((tform.to - idxhead.first) / ONE_DAY + 1) * sizeof(long long) +
             sizeof(idxhead);
         if (i > 0)
         {  rc = lseek(fd, i, SEEK_SET);
            if (rc == i)
            {  rc = read(fd, &idxstop, sizeof(idxstop));
               if (rc != sizeof(idxstop))  idxstop = 0; 
            }
         }
         
         close(fd); fd = (-1); 
         break;
      }
   }

// STUB
   if (fAll)
   {  acc_cnt            = 1;
      acc_list[0].accno  = 0;
      acc_list[0].count_in  = 0;
      acc_list[0].count_out = 0;
      acc_list[0].money_in  = 0;
      acc_list[0].money_out = 0;
      acc_list[0].money_charge = 0;
      acc_list[0].pays      = 0;
      acc_list[0].in_recs   = 0;
      acc_list[0].out_recs  = 0;
      if (acc_list[0].descr == NULL) acc_list[0].descr = "ANY";
   }

// Load defaults (if not initialized)
   if (tform.fields == NULL)
   {  if (tform.res == 2)
      {  if ((tform.flags & FLAG_DIRGROUP) != 0) tform.fields = "S";
         else tform.fields = "DTS"; 
      }  
      else
      {  if ((tform.flags & FLAG_DIRGROUP) != 0) tform.fields = fLine ? "CS":"ICS";
         else tform.fields = "DTICSH";
      }
   }
   if (tform.title == NULL)
   {  
      tform.title = titlebuf;
   }
// Set summary flags
   tform.flags |= FLAG_SUMMONEY | (tform.res == 2 ? 0 : FLAG_SUMCOUNT); 

// Open Log base
   rc = log_baseopen_sr(&Logbase, logname);
   if (rc < 0)
   {  syslog(LOG_ERR, "main(log_baseopen) : Error");
      return (-1);
   }
   
   if (acc_cnt == 0)
   {  while (1)
      {  if (fgets(buf, sizeof(buf), stdin) == NULL) break;
         ptr = buf;
         str = next_token(&ptr, " \t\n\r:");
         if (str == NULL) continue;
         if (strcasecmp(str, "go") == 0) break;
         if (acc_cnt < MAXRECS)
         {  acc_list[acc_cnt].accno = strtol(buf, NULL, 10);
            acc_list[acc_cnt].count_in  = 0;
            acc_list[acc_cnt].count_out = 0;
            acc_list[acc_cnt].money_in  = 0;
            acc_list[acc_cnt].money_out = 0;
            acc_list[acc_cnt].money_charge = 0;
            acc_list[acc_cnt].pays      = 0;
            acc_list[acc_cnt].in_recs   = 0;
            acc_list[acc_cnt].out_recs  = 0;

            str = next_token(&ptr, "\n:");
            if (str == NULL) acc_list[acc_cnt].descr = "";
            else asprintf(&(acc_list[acc_cnt].descr), "%s", str); 
            acc_cnt ++;
            continue;
         }
         else
         {  syslog(LOG_ERR, "FATAL: account table overflow");
            fprintf(stderr, "FATAL: account table overflow\n");  
            exit(-1);
         } 
      } 
   }
   
// File headers
   if (headers != 0)
   {  printf("<html><head>\n");

      if (HeadTempl == NULL)
      {  printf("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=koi8-r\">\n");
         printf("<meta http-equiv=\"Content-Language\" content=\"ru\">\n");
         printf("<title>Отчет биллинга</title>");
      }
      else
         printf("%s", HeadTempl); 

      if (tform.tabopts == tabopts_def)
         printf("<style>\n.txt\n\t{\n\tfont-size: 9pt;\n\tfont-family: \"Arial\";\n\t}\n</style>\n");

      printf("</head><body %s>\n", tform.bodyopts);

      if (BodyTemplPre == NULL)
      {  printf("<font face=\"Arial, Helvetica, sans-serif\" size=\"-1\">");
         printf("<center>");
      }
      else
         printf("%s", BodyTemplPre);

      printf("<h3>Отчет за период<br>");
      if (tform.from != 0) printf("c %s<br>", strtime(tform.from));
      else printf("со времени запуска<br>");
      if (tform.to != 0) printf("по %s<br>", strtime(tform.to));
      else 
      {  tform.to = time(NULL);
         printf("по %s<br>", strtime(tform.to));
         tform.to = 0;
      }
      printf("</h3>");
   }

// Open table for line mode
   if (fLine)
   {  printf("<table %s>\n", tform.tabopts);
      // print headers
      
      ptmpl = tform.fields;
      printf("<tr %s>\n", tform.headopts);
      if (fNumbers) printf("<td><strong>No</strong></td>");
      printf("<td><strong>клиент</strong></td><td><strong>счет</strong></td>\n");
      while (*ptmpl != '\0')
      {  printf("<td><strong>");
         switch(*ptmpl)
         {  case 'D':   // date
               printf("дата");
               break;
            case 'T':   // time
               printf("время");
               break;                
            case 'A':   // accno
               printf("счет");
               break;
            case 'B':   // balance
               printf("баланс");
               break;
            case 'R':   // res
               printf("ресурс");
               break;
            case 'C':   // count
               printf("In</strong></td><td><strong>Out</strong></td><td><strong>Общий");
               break;
            case 'S':   // sum
               printf("сумма");
               break;
            case 'E':   // result
               printf("результат");
               break;
            case 'I':   // direction
               printf("напр.");
               break;
            case 'H':   // host
               printf("хост");
               break;
            case 'P':   // price rate
               printf("ср. цена");
               break;
            case 's':   // speed rate
               printf("ср/скор.");
               break;
            case 't':   // tariff
               printf("тариф");
               break;
            default:
               printf("?");
         }
         printf("</td>\n");
         ptmpl++;
      }
      printf("</tr>\n"); 
   }

   for (i=0; i < acc_cnt; i++)
   {
      if (!fLine)
      {  snprintf(titlebuf, sizeof(titlebuf), "Счет # %d%s%s%s", acc_list[i].accno, 
               acc_list[i].descr != NULL ? " (":"",
               acc_list[i].descr != NULL ? acc_list[i].descr:"",
               acc_list[i].descr != NULL ? ")":"");
      }
      else
      {  snprintf(titlebuf, sizeof(titlebuf), "%s", 
               acc_list[i].descr != NULL ? acc_list[i].descr : "n/a");
      }

      tform.accno  = acc_list[i].accno;
      print_table(&tform, &wsc, &wsm, i);

      if (!fLine && i < (acc_cnt-1) ) printf("<br>");

      sc += wsc;
      sm += wsm;
   }

   if (fsum != 0 && !fLine)
   {  printf("<br>\n<br>\n");
      printf("<H4>Всего:</H4><br>\n");
      if (tform.res != 2) 
      {  printf("<strong>счетчик: ");
         if (!fNoBytes || sc < 1024)
         {  printf("%.0Lf", (long double)sc);
            if (sc >= 1024) printf(" (");
         }
         if (sc >= 1048576) printf("%.2Lf M", (long double)sc / 1048576);
         else
         {  if (sc >= 1024) printf("%.2Lf K", (long double)sc / 1024);
         }
         if (!fNoBytes && sc >= 1024) printf(")");
         printf("</strong><br>\n");
      }
      printf("<strong>сумма: %.2Lf</strong><br>\n ", sm);
   }

   if (fsum != 0 && fLine)
   {  printf("<tr %s><td%s><div align=right><strong>Всего:</strong></div></td><td>&nbsp;</td>",
              tform.cellopts, fNumbers ? " colspan=2":"");
      ptmpl = tform.fields;
      while (*ptmpl != '\0')
      {  if (*ptmpl != 'C') printf("<td>");
         switch(*ptmpl)
         {
            case 'C':   // count
               printf("<td colspan=3>");                     /// ?
               if (tform.flags & FLAG_SUMCOUNT)
               {  printf("<strong><div align=right>");
                  if (!fNoBytes || sc < 1024)
                  {  printf("%llu", sc);
                     if (sc >= 1024) printf(" (");
                  }
                  if (sc >= 1048576) printf("%.2Lf M", (long double)sc / 1048576);
                  else
                  if (sc >= 1024) printf("%.2Lf K", (long double)sc / 1024);
                  if (!fNoBytes && sc >= 1024) printf(")"); 
                  printf("</div></strong>");
               }
               else printf("&nbsp;");
               break;
            case 'S':   // sum
               if (tform.flags & FLAG_SUMMONEY)
                  printf("<strong><div align=right>%.2Lf</div></strong>", sm);
               break;
            default:
               printf("&nbsp;");
         }
         printf("</td>\n");
         ptmpl++;
      }
      printf("</tr>\n"); 
   }
   
// Close table for lined mode
   if (fLine)
   {  printf("</table>");
   }


   if (headers != 0)
   {  
      if (BodyTemplPre == NULL)
      {  printf("</center>");
         printf("</font>");
      }
      else
         if (BodyTemplPost != NULL) printf("%s", BodyTemplPost);

      printf("</body></html>\n");
   }
   
   return 0;
}


int print_table(tformat_t * tform, u_int64_t * sc,  long double * sm, int ind)
{  char        * ptmpl;
   long long     i;
   long long     recs;
   int           rc;

   long double   summoney = 0;
   long double   sumpays  = 0;  // adder summary for unires mode (tform.res < 0)
   u_int64_t     sumcount = 0;

   logrec_t      inrec;
   u_int64_t     incount;
   long double   insum;
   int           in_recs = 0;

   logrec_t      outrec;
   u_int64_t     outcount;
   long double   outsum;
   int           out_recs = 0;

   logrec_t      chargerec;
   long double   chargesum;

   long long     istart;
   long long     istop;

   int           a;
   
   *sc = 0;
   *sm = 0;

   memset(&inrec,  0, sizeof(inrec));
   incount = 0;
   insum   = 0;
   memset(&outrec, 0, sizeof(outrec));
   outcount = 0;
   outsum   = 0;

   memset(&chargerec, 0, sizeof(chargerec));
   chargesum = 0;

   inrec.time  = time(NULL);
   outrec.time = inrec.time;

   outrec.isdata.proto_id = 0x80000000;

   chargerec.isdata.proto_id = 0x40000000;

// Get log records count
   recs = log_reccount(&Logbase);
   if (recs < 0) 
   {  syslog(LOG_ERR, "main(log_reccount): Error, stopped");
      return (-1);
   }

   if (fLine && !fNoZeros) printf("<tr %s>\n", tform->cellopts);

// Printf table caption
   if (tform->title != NULL && !fLine)
      printf("%s", tform->title);

// Table begin tag
   if (!fLine)
      printf("<table %s>\n", tform->tabopts);
     

// Print headers
   if (!fLine)
   {
   ptmpl = tform->fields;
   printf ("<tr %s>\n", tform->headopts);
   while (*ptmpl != '\0')
   {  printf("<td><strong>");
      switch(*ptmpl)
      {
         case 'D':   // date
            printf("дата");
            break;
         case 'T':   // time
            printf("время");
            break;                
         case 'A':   // accno
            printf("счет");
            break;
         case 'B':   // balance
            printf("баланс");
            break;
         case 'R':   // res
            printf("ресурс");
            break;
         case 'C':   // count
            printf("счетчик");
            break;
         case 'S':   // sum
            printf("сумма");
            break;
         case 'E':   // result
            printf("результат");
            break;
         case 'I':   // direction
            printf("напр.");
            break;
         case 'H':   // host
            printf("хост");
            break;
         case 'P':   // price rate
            printf("ср. цена");
            break;
         case 's':   // speed rate
            printf("ср/скор.");
            break;
         case 't':   // tariff
            printf("тариф");
            break;
         default:
            printf("?");
      }
          printf("</strong></td>\n");
          ptmpl++;
   }
   printf("</tr>\n"); 
   }

   istart = idxstart;
   istop  = idxstop ? idxstop : recs;

   if ((off_flags & OFLAG_FIRST) != 0) istart = first;
   if ((off_flags & OFLAG_LAST ) != 0) istop  = last;

// Print table
if (! fCached)
{
   for (i=istart; i<istop; i++)
   {

// Get record
       rc = logi_get(&Logbase, i, &logrec);
       if (rc == IO_ERROR || rc == NOT_FOUND)
       {  syslog(LOG_ERR, "main(log_get): Error");
          return 0;
       }
       
// Filter by from (time >= from)
       if (tform->from > 0   && logrec.time < tform->from) continue;
// Filter by to (time <= to)
       if (tform->to > 0   && logrec.time > tform->to) continue;

// Store first index
       if ((off_flags & OFLAG_FIRST) == 0) 
       {  first      = i;
          off_flags |= OFLAG_FIRST;
       } 

// Store current index as last candidate
       if ((off_flags & OFLAG_LAST) == 0) 
       {  last = i+1;
       }

// Filter by resource (if given)
       if (tform->res >= 0   && logrec.isdata.res_id != tform->res) continue;
// Filter by direction
       if (fIn     == 0 && (logrec.isdata.proto_id & 0xC4000000) == 0) continue;
       if (fOut    == 0 && (logrec.isdata.proto_id & 0x80000000) != 0) continue;
       if (fCharge == 0 && (logrec.isdata.proto_id & 0x44000000) != 0) continue;

// Filter by host
       if ((logrec.isdata.host.s_addr & filter_mask) != filter_addr) continue;

// Count group sums for all given accounts
       if ((tform->flags & FLAG_DIRGROUP) != 0 )  
       {  for (a=1; a < acc_cnt; a++)
          {  if (logrec.accno == acc_list[a].accno) 
             {  if (logrec.serrno != ACC_DELETED && logrec.serrno != ACC_BROKEN)
                {  if ((logrec.isdata.proto_id &0x44000000) == 0)
                   {  if ((logrec.isdata.proto_id &0x80000000) == 0)
                      {  if (tform->res >= 0 || logrec.isdata.res_id != 2)
                         {  acc_list[a].money_in += logrec.sum;
                            acc_list[a].count_in += logrec.isdata.value;
                            acc_list[a].in_recs++;
                          
                         }
                      }
                      else
                      {  acc_list[a].money_out += logrec.sum;
                         acc_list[a].count_out += logrec.isdata.value;
                         acc_list[a].out_recs++;
                      }
                   }
                   else
                   {  acc_list[a].money_charge += logrec.sum;
                   }
 
                }
                if (tform->res < 0 && logrec.isdata.res_id == 2)
                   acc_list[a].pays += logrec.sum;
             }
          }
       }

// Filter by accno (if given)
       if (fAll == 0)
          if (tform->accno >= 0 && logrec.accno != tform->accno) continue;

// skip account
       if (SkipAcc >= 0 && logrec.accno == SkipAcc) continue;

       prnrec = logrec;

/*
 "DTABRCSE"
  Date, time, accno, balance, res, count, sum, result
 */


// Print record (or group it)
       if ((tform->flags & FLAG_DIRGROUP) == 0 )
       {  print_record(&logrec, 0, 0, 0, tform);
       }
       else
       {  if (logrec.serrno != ACC_DELETED &&
              logrec.serrno != ACC_BROKEN)
          {  if ((logrec.isdata.proto_id &0x44000000) == 0)
             {  if ((logrec.isdata.proto_id &0x80000000) == 0)
                {  if (tform->res >= 0 || logrec.isdata.res_id != 2)
                   {  insum    += logrec.sum;
                      incount  += logrec.isdata.value;
                      in_recs++;
                   }
                }
                else
                {  outsum   += logrec.sum;
                   outcount += logrec.isdata.value;
                   out_recs++;
                }
             }
             else
             {  chargesum += logrec.sum;
             }
          }
       }

       if (tform->res < 0 && logrec.isdata.res_id == 2)
          sumpays += logrec.sum;
       else
       {  // Sum counts
          sumcount += logrec.isdata.value;
          // Sum money
          summoney += logrec.sum;
       }
   }
   if ((tform->flags & FLAG_DIRGROUP) != 0) fCached = 1;
}
else
{  insum    =  acc_list[ind].money_in;
   outsum   =  acc_list[ind].money_out;
   chargesum=  acc_list[ind].money_charge;
   incount  =  acc_list[ind].count_in;
   outcount =  acc_list[ind].count_out;
   incount  =  acc_list[ind].count_in;
   sumpays  =  acc_list[ind].pays;
   in_recs  =  acc_list[ind].in_recs;
   out_recs =  acc_list[ind].out_recs;
   sumcount += incount + outcount;
   summoney += insum + outsum + chargesum;
}

   if ((off_flags & OFLAG_LAST) == 0 && last != 0) 
   {  off_flags |= OFLAG_LAST;
   }

   if (!fLine)
   {  if ((tform->flags & FLAG_DIRGROUP) != 0 )
      {  if (fIn)  print_record(&inrec, incount, insum, in_recs, tform);
         if (fOut) print_record(&outrec, outcount, outsum, out_recs, tform);
         if (chargesum != 0) print_record(&chargerec, 0, chargesum, 0, tform);
      }
   }
   else
   {  
      if (!fNoZeros || (incount | outcount) != 0 || insum >= 0.01 || outsum >= 0.01)
      {  printf("<tr %s>\n", tform->cellopts);

         if (fNumbers) printf("<td><div align=right>%d</div></td>", ind);

         printf("</td><td>%s</td><td><div align=right>%d</div></td>",
                tform->title ? tform->title : "&nbsp;", tform->accno);

         print_line_record(incount, outcount, insum, outsum, tform);
         printf("</tr>\n");
      }
   }
   
   if (!fLine)
   {
      if (sumpays != 0)
      {  printf("<tr %s><td><strong>Платежи:</strong></td>", tform->cellopts);
         ptmpl = tform->fields + 1;
         while (*ptmpl != '\0')
         {  printf("<td>");
            switch(*ptmpl)
            {  case 'S':   // sum
                  printf("<strong><div align=right>%+.2Lf</div></strong>", sumpays);
                  break;
               default:
                  printf("&nbsp;");
            }
            printf("</td>\n");
            ptmpl++;
         }
         printf("</tr>\n");
      }
   
      if (tform->res < 0)
         printf("<tr %s><td><strong>Расход:</strong></td>", tform->cellopts);
      else
         printf("<tr %s><td><strong>Итого:</strong></td>", tform->cellopts);

      ptmpl = tform->fields + 1;
      while (*ptmpl != '\0')
      {  printf("<td>");
         switch(*ptmpl)
         {
            case 'C':   // count
               if (tform->flags & FLAG_SUMCOUNT)
               {  printf("<strong><div align=right>");
                  if (!fNoBytes || sumcount < 1024)
                  {  printf("%llu", sumcount);
                     if (sumcount >= 1024) printf(" (");
                  }
                  if (sumcount >= 1048576) printf("%.2Lf M", (long double)sumcount / 1048576);
                  else
                  if (sumcount >= 1024) printf("%.2Lf K", (long double)sumcount / 1024);
                  if (!fNoBytes && sumcount >= 1024) printf(")");
                  printf("</div></strong>");
               }
               break;
            case 'S':   // sum
               if (tform->flags & FLAG_SUMMONEY)
                  printf("<strong><div align=right>%+.2Lf</div></strong>", summoney);
               break;
            default:
               printf("&nbsp;");
         }
         printf("</td>\n");
         ptmpl++;
      }
      printf("</tr>\n"); 
      printf("</table>");
   }

   *sc = sumcount;
   *sm = summoney;

   return 0;
}

int print_record(logrec_t * rec, u_int64_t count, long double sum, int reccnt, tformat_t * tform)
{  char       * ptmpl;
   struct tm    stm;
   long double  val;

   localtime_r(&(rec->time), &stm); 


   if (dumpf)
   {  fprintf(dumpf, "INSERT INTO log (time, acc_id, sum, balance, err, res_id, count, proto, plan, subplan, host) VALUES (");
      fprintf(dumpf, "'%04d-%02d-%02d %02d:%02d:%02d.000000%+02ld'", 
              stm.tm_year+1900, 
              stm.tm_mon+1,
              stm.tm_mday,
              stm.tm_hour,
              stm.tm_min,
              stm.tm_sec,
              stm.tm_gmtoff/3600);
      fprintf(dumpf, ", %d", rec->accno);

      if (sum == 0)
         fprintf(dumpf, ", %g", rec->sum);
      else
         fprintf(dumpf, ", %Lg", sum);
 
      if (rec->balance == BALANCE_NA)
         fprintf(dumpf, ", NULL");
      else
         fprintf(dumpf, ", %g", rec->balance);

      fprintf(dumpf, ", %d", rec->serrno);
      fprintf(dumpf, ", %d", rec->isdata.res_id);
      
      if (count == 0)
         fprintf(dumpf, ", %u", rec->isdata.value);
      else
         fprintf(dumpf, ", %llu", count);

      fprintf(dumpf, ", %u", rec->isdata.proto_id);
      fprintf(dumpf, ", %d, %d", (rec->isdata.proto2 & PROTO2_TPLAN)>>16, (rec->isdata.proto2 & PROTO2_SUBTPLAN)>>24);
          

      if (rec->isdata.res_id == 2 || (rec->isdata.proto_id & 0x44000000) != 0)
         fprintf(dumpf, ", NULL");
      else
         fprintf(dumpf, ", '%s'", inet_ntop(AF_INET, &(rec->isdata.host), buf, sizeof(buf)));

      fprintf(dumpf, ");\n");  

   }

   ptmpl = tform->fields;
   printf ("<tr %s>\n", tform->cellopts);
   while (*ptmpl != '\0')
   {  printf("<td>");
      switch(*ptmpl)
      {
         case 'D':   // date
            printf("%02d/%02d/%04d", 
            stm.tm_mday, stm.tm_mon+1, stm.tm_year+1900);
            break;
         case 'T':   // time
            printf("%02d:%02d", 
                    stm.tm_hour, stm.tm_min);
            break;                
         case 'A':   // accno
            printf("%d", rec->accno);
            break;
         case 'B':   // balance
            if (rec->balance == BALANCE_NA)
               printf("<center>N/A</center>");
            else
               printf("<div align=right>%+.2f</div>", rec->balance);
            break;
         case 'R':   // res
            printf("%d", rec->isdata.res_id);
            break;
         case 'C':   // count
            if (rec->isdata.res_id == 2 || (rec->isdata.proto_id & 0x44000000) != 0)
            {  printf("&nbsp;");
               break;
            } 
            if (count == 0)
               printf("<div align=right>%u</div>", rec->isdata.value);
            else
            {  printf("<div align=right>");
               if (!fNoBytes || count < 1024)
               {  printf("%llu", count);
                  if (count >= 1024) printf(" (");
               } 
               if (count >= 1048576)   printf("%.2f M", (double)count/1048576);
               else if (count >= 1024) printf("%.2f K", (double)count/1024);
               if (!fNoBytes && count >= 1024) printf(")"); 
               printf("</div>");
            }
            break;
         case 'S':   // sum
            if (sum == 0)
               printf("<div align=right>%+.2f</div>", rec->sum);
            else
               printf("<div align=right>%+.2Lf</div>", sum);
            break;
         case 'E':   // result
            printf("(%d)", rec->serrno);
            break;
         case 'I':
            if (rec->isdata.res_id == 2)
            {  printf("&nbsp;");
               break;
            } 
            if ((rec->isdata.proto_id & 0x44000000) == 0)
            {  if ((rec->isdata.proto_id & 0x80000000) == 0)
                  printf("<center>IN</center>");
               else
                  printf("<center>OUT</center>");
            }
            else
               printf("<center>аб/плата</center>");
            break; 
         case 'H':
            if (rec->isdata.res_id == 2 || (rec->isdata.proto_id & 0x44000000) != 0)
            {  printf("&nbsp;");
               break;
            } 
            printf("%s", inet_ntop(AF_INET, &(rec->isdata.host), buf, sizeof(buf)));
            break;
         case 'P':
            if (rec->isdata.res_id == 2 || (rec->isdata.proto_id & 0x44000000) != 0)
            {  printf("&nbsp;");
               break;
            } 
            if ((sum == 0 && rec->isdata.value == 0) || (sum != 0 && count == 0)) printf("&nbsp;");
            else 
            {  if (sum == 0)
                  printf("<div align=right>%.2f</div>", -(rec->sum * 1048576 / rec->isdata.value));
               else
                  printf("<div align=right>%.2Lf</div>", -(sum * 1048576 / count));
            }
            break;
         case 's':
            if (rec->isdata.res_id == 2)
            {  printf("&nbsp;");
               break;
            } 
            if (rec->isdata.value == 0 && count == 0) printf("&nbsp;");
            else 
            {  if (count == 0)
                  val = (((long double)rec->isdata.value) / 600 / 1024 / (reccnt ? reccnt : 1));
               else
                  val = (((long double)count) / 600 / 1024 / (reccnt ? reccnt : 1)); 

               printf("<div align=right>%s%.2Lf%s</div>", 
                        val > MaxSpeed ? "<b>":"",
                        val,
                        val > MaxSpeed ? "</b>":"");
            }
            break;
         case 't':
            printf("<div align=right>%d.%d</div>", (rec->isdata.proto2 & PROTO2_TPLAN)>>16,
                    (rec->isdata.proto2 & PROTO2_SUBTPLAN)>>24 );
            break;
         default:
            printf("<center>?</center>");
      }
      printf("</td>\n");
      ptmpl++;
   }
   printf("</tr>\n"); 

   return 0;
}

int print_line_record(u_int64_t count_in, u_int64_t count_out, long double sum_in, long double sum_out, tformat_t * tform)
{  char       * ptmpl;
   u_int64_t    cnt;

   ptmpl = tform->fields;
   while (*ptmpl != '\0')
   {  printf("<td>");
      switch(*ptmpl)
      {

         case 'C':   // count
            printf("<div align=right><font color=#404040>");
            if (!fNoBytes || count_in < 1024)
            {  printf("%llu", count_in);
               if (count_in >= 1024) printf(" (");
            } 
            if (count_in >= 1048576)   printf("%.2f M", (double)count_in/1048576);
            else if (count_in >= 1024) printf("%.2f K", (double)count_in/1024);
            if (!fNoBytes && count_in >= 1024) printf(")");
            printf("</font></div></td><td><div align=right><font color=#404040>");

            if (!fNoBytes || count_out < 1024)
            {  printf("%llu", count_out);
               if (count_out >= 1024) printf(" (");
            } 
            if (count_out >= 1048576)   printf("%.2f M", (double)count_out/1048576);
            else if (count_out >= 1024) printf("%.2f K", (double)count_out/1024);
            if (!fNoBytes && count_out >= 1024) printf(")");
            printf("</font></div></td><td bgcolor=#dedede><div align=right>");

            cnt = count_out + count_in;
            if (!fNoBytes || cnt < 1024)
            {  printf("%llu", cnt);
               if (cnt >= 1024) printf(" (");
            }
            if (cnt >= 1048576)   printf("%.2f M", (double)cnt/1048576);
            else if (cnt >= 1024) printf("%.2f K", (double)cnt/1024);
            if (!fNoBytes && cnt >= 1024) printf(")");
            printf("</div>");

            break;
         case 'S':   // sum
            printf("<div align=right>%+.2Lf</div>", sum_in + sum_out);
            break;
         case 'P':
            if ((count_in + count_out) == 0) printf("&nbsp;");
            else printf("<div align=right>%.2Lf</div>", -((sum_in + sum_out) * 1048576 / (count_in + count_out)));
            break;
         default:
            printf("<center>N/A</center>");
      }
      printf("</td>\n");
      ptmpl++;
   }

   return 0;
}



// 1.01.2002.20.53.42

char   delim[]=" \n\t:./\\;,'\"`-";

time_t  parse_time(char * strdate)
{  char      * ptr = strdate;
   char      * str;
   struct tm   stm;
   time_t      rval;
   int         temp;
   int         eol = 0;

   memset(&stm, 0, sizeof(stm));

// month day 
   str = next_token(&ptr, delim);
   if (str == NULL) return 0;
   stm.tm_mday = strtol(str, NULL, 10);
   if (stm.tm_mday > 1000000000)
   {  str = next_token(&ptr, delim);
      if (str != NULL) return 0;      
      return stm.tm_mday;    // return raw UTC
   }
// month
   str = next_token(&ptr, delim);
   if (str == NULL) return 0;
   stm.tm_mon = strtol(str, NULL, 10) - 1;
// year
   str = next_token(&ptr, delim);
   if (str == NULL) return 0;
   temp = strtol(str, NULL, 10);
   if (temp < 100) temp += 100;
   else temp -= 1900;
   if (temp < 0) return 0;
   stm.tm_year = temp;
// hour (optional)
   str = next_token(&ptr, delim);
   if (str == NULL) eol = 1;
   else stm.tm_hour = strtol(str, NULL, 10);
// minute (optional)
   if (! eol)      
   {  str = next_token(&ptr, delim);
      if (str == NULL) eol = 1;
      else stm.tm_min = strtol(str, NULL, 10);
   }
// seconds (optional)
   if (! eol)      
   {  str = next_token(&ptr, delim);
      if (str == NULL) eol = 1;
      else stm.tm_sec = strtol(str, NULL, 10);
   }
// assemble time_t
   stm.tm_isdst = -1;
   rval = mktime(&stm);
   if (rval < 0) return 0;

   return rval;
}

char * strtime(time_t utc)
{ 
   struct tm     stm;
   static char   tbuf[64];
 
   if (localtime_r(&utc, &stm) == NULL) return "не определено";

   snprintf(tbuf, sizeof(tbuf), "%02d:%02d:%04d %02d:%02d:%02d",
            stm.tm_mday, stm.tm_mon+1, stm.tm_year+1900,
            stm.tm_hour, stm.tm_min, stm.tm_sec);

   return tbuf;
}

void usage()
{
   fprintf(stderr,
"BEE - Small billing solutions project ver. 0.1 \n"
"   Report generator utility\n\n"
"BSD licensed, see LICENSE for details. OpenBSD.RU project\n\n"
" Usage:\n"
"   [cat acc_list |] beerep [options] > report.html \n"
"      Available options:\n"
"F D:M:Y[h:m[:s]] - from given time (default - most early)\n"
"T D:M:Y[h:m[:s]] - to given time (default - most late)\n"
"F UTCseconds     - from given time (seconds since Epoch)\n"
"T UTCseconds     - to given time (seconds since Epoch)\n"
"n N              - given number of days (requires -F, can't work with -T)\n"
"r N              - resource id 2-\"adder\" (default - 0, inet)\n"
"R                - all resources (analog -r -1)\n"
"g                - group data by direction\n"
"s                - count summary info\n"
"a N[:descr]      - account number (allowed multiply instances)\n"
"A                - ALL accounts\n"
"l listname       - accounts w/ named list gate (with label)\n"
"i                - skip inbound traffic\n"
"o                - skip outbound traffic\n"
"f                - skip charge transactions\n"
"I file           - use one-range-index file (load & update)\n"
"c str            - <table> options\n"
"d                - use full index (generated by beelogidx utility)\n"
"L                - line mode, output accounts on single table (forces -g)\n"
"z                - skip zero count/sum lines (forces -L & -g)\n"
"h                - suppress HTML-page prologue & epilogue\n"
"C str            - <tr> options for cells\n"
"H str            - <tr> options for heads\n"
"b str            - <body> options\n"
"m                - print only Kbytes/Mbytes on counter\n"
"E file           - <head></head> lines file\n"
"B file           - <body></body> template file (%%BODY%% - program output)\n"
"S N              - account to skip (hack)\n"
"N                - print line numbers on line mode (-L)\n"
"M N              - indicate speed (KB/s) greater then given\n"
"x IP[/CIDR]      - filter by IP range\n"
"D file           - dump as SQL INSERTS to given file\n"
"t str            - force redefine columns template string\n"
"     D - date\n"
"     T - time\n"
"     A - account number\n"
"     B - balance before transaction\n"
"     R - resource id\n"
"     C - counter\n"
"     S - sum\n"
"     P - average price\n"
"     E - result code\n"
"     I - direction\n"
"     H - client host\n"
"     s - average speed rate\n"
"     t - tariff id\n"
"\n"
);

}
