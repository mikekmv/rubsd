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


#include <bee.h>
#include <db.h>
#include <res.h>

#include <beerep.h>

#ifndef MAXRECS
#define MAXRECS 4096
#endif

int         off_flags = 0;

int         first = 0;
int         last  = 0;

char      * logname="/var/bee/beelog.dat";
logbase_t   Logbase;
int         recs;

char      * idxname="/var/bee/beelog.idx";

logrec_t    logrec;
logrec_t    prnrec;

char      * templ="DTABRCSEIHP";
/*
 "DTABRCSE"
  Date, time, accno, balance, res, count, sum, result
 */

int        acc_cnt = 0;
int        acc_array[MAXRECS];
char     * acc_descr[MAXRECS];


char       buf[256];
char       titlebuf[256];

int        fAll     = 0;
int        fIn      = 1;
int        fOut     = 1;
int        fIDX     = 0;
int        fLine    = 0;
int        fNoZeros = 0;

idxhead_t  idxhead;
u_int      idxstart = 0;
u_int      idxstop  = 0;

char     * index_name = NULL;

int main(int argc, char ** argv)
{  int          rc;
   tformat_t    tform;

   int          fsum    = 0;
   int          headers = 1;
   int          i;

   u_int64_t    wsc = 0;
   u_int64_t    sc  = 0;
   long double  wsm = 0;
   long double  sm  = 0;

   char       * ptr = NULL;
   char       * str;

   indexes_t    indfile;

   int          fd = (-1);
   char       * ptmpl = NULL;

// Initialize table format
   memset(&tform, 0, sizeof(tform));

#define PARAMS "F:T:r:t:gsa:hAioI:c:dRLz"

   while ((rc = getopt(argc, argv, PARAMS)) != -1)
   {
      switch (rc)
      {
         case 'c':
            tform.opts = optarg;
            break;

         case 'F':
            tform.from = parse_time(optarg);
            break;

         case 'T':
            tform.to = parse_time(optarg);
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
            if (acc_cnt < MAXRECS)
            {  acc_array[acc_cnt++] = strtol(optarg, &ptr, 10);
               if (ptr != NULL)
               {  if (*ptr != '\0') ptr++;
                  if (*ptr != '\0')
                  {  asprintf(&(acc_descr[acc_cnt-1]), "%s", ptr);
                  }
               } 
            }
            else
            {  syslog(LOG_ERR, "FATAL: account table overflow");
               fprintf(stderr, "FATAL: account table overflow");  
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

         case 'I':
            index_name = optarg;
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

         default:
            usage();
            exit(-1); 
      }
   }

#define ONE_DAY (3600*24)

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
         i = ((tform.from - idxhead.first) / ONE_DAY - 1) * sizeof(u_int) +
             sizeof(idxhead);
         if (i > 0)
         {  rc = lseek(fd, i, SEEK_SET);
            if (rc == i)
            {  rc = read(fd, &idxstart, sizeof(idxstart));
               if (rc != sizeof(idxstart))  idxstart = 0; 
            }
         }
         // read stop index
         i = ((tform.to - idxhead.first) / ONE_DAY) * sizeof(u_int) +
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

// Load index file
   if (index_name != NULL)
   {  fd = open(index_name, O_RDWR | O_EXLOCK | O_CREAT, 0777);
      if (fd >= 0)
      {  if (read(fd, &indfile, sizeof(indfile)) == sizeof(indfile))
         {  if (tform.from != 0 && indfile.time_from != 0 && 
                indfile.ind_from != 0 && indfile.time_from == tform.from)
            {  first = indfile.ind_from;
               off_flags |= OFLAG_FIRST;  
            }
            if (tform.to != 0 && indfile.time_to != 0 && 
                indfile.ind_to != 0 && indfile.time_to == tform.to)
            {  last = indfile.ind_to;
               off_flags |= OFLAG_LAST;  
            }
         }
      }  
   }


// STUB
   if (fAll)
   {  acc_cnt      = 1;
      acc_array[0] = 0;
      if (acc_descr[0] == NULL) acc_descr[0] = "ANY";
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
   rc = log_baseopen(&Logbase, logname);
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
         {  acc_array[acc_cnt] = strtol(buf, NULL, 10);
            str = next_token(&ptr, "\n:");
            if (str == NULL) acc_descr[acc_cnt] = "";
            else asprintf(acc_descr + acc_cnt, "%s", str); 
            acc_cnt ++;
            continue;
         }
         else
         {  syslog(LOG_ERR, "FATAL: account table overflow");
            fprintf(stderr, "FATAL: account table overflow");  
            exit(-1);
         } 
      } 
   }
   
// File headers
   if (headers != 0)
   {  printf("<html><head>\n");
      printf("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=koi8-r\">\n");
      printf("<meta http-equiv=\"Content-Language\" content=\"ru\">\n");
      printf("<title>Отчет биллинга</title>");
      if (tform.opts == NULL)
         printf("<style>\n.txt\n\t{\n\tfont-size: 9pt;\n\tfont-family: \"Arial\";\n\t}\n</style>\n");
      printf("</head><body>\n");
      printf("<font face=\"Arial, Helvetica, sans-serif\" size=\"-1\">");
      printf("<center>");
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
   {  printf("<table border=1 cellpadding=4 %s>\n",
        tform.opts != NULL ? tform.opts : "class=\"txt\"");
      // print headers
      
      ptmpl = tform.fields;
      printf("<tr align=center>\n");
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
               printf("In</strong></td><td><strong>Out");
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
      {  snprintf(titlebuf, sizeof(titlebuf), "Счет # %d%s%s%s", acc_array[i], 
               acc_descr[i] != NULL ? " (":"",
               acc_descr[i] != NULL ? acc_descr[i]:"",
               acc_descr[i] != NULL ? ")":"");
      }
      else
      {  snprintf(titlebuf, sizeof(titlebuf), "%s", 
               acc_descr[i] != NULL ? acc_descr[i] : "n/a");
      }

      tform.accno  = acc_array[i];
      print_table(&tform, &wsc, &wsm);

      if (!fLine) printf("<br>");

      sc += wsc;
      sm += wsm;
   }

   if (fsum != 0 && !fLine)
   {  printf("<br>\n<br>\n");
      printf("<H4>Всего:</H4><br>\n");
      if (tform.res != 2) 
      {  printf("<strong>счетчик: %.0Lf ", (long double)sc);
         if (sc > 1048576) printf("(%.2Lf M)", (long double)sc / 1048576);
         else
         {  if (sc > 1024) printf("(%.2Lf K)", (long double)sc / 1024);
         }
      }
      printf("</strong><br>\n");
      printf("<strong>сумма: %Lg ", sm);
   }

   if (fsum != 0 && fLine)
   {  printf("<tr><td><div align=right><strong>Всего:</strong></div></td><td>&nbsp;</td>");
      ptmpl = tform.fields;
      while (*ptmpl != '\0')
      {  if (*ptmpl != 'C') printf("<td>");
         switch(*ptmpl)
         {
            case 'C':   // count
               printf("<td colspan=2>");
               if (tform.flags & FLAG_SUMCOUNT)
               {  printf("<strong><div align=right>%llu", sc);
                  if (sc > 1048576) printf(" (%.2Lf M)", (long double)sc / 1048576);
                  else
                  if (sc > 1024) printf(" (%.2Lf K)", (long double)sc / 1024);
                  printf("</div></strong>");
               }
               else printf("&nbsp;");
               break;
            case 'S':   // sum
               if (tform.flags & FLAG_SUMMONEY)
                  printf("<strong><div align=right>%Lg</div></strong>", sm);
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
   {  printf("</table><br>\n");
   }


   if (headers != 0)
   {  
      printf("</center>");
      printf("</font>");
      printf("</body></html>\n");
   }
   
// Save index file
   if (index_name != NULL)
   {
//      fd = open(index_name, O_WRONLY | O_CREAT, 0777);
      if (fd >= 0)
      {  
         lseek(fd, 0, SEEK_SET);
         indfile.time_from = tform.from;
         indfile.ind_from  = first;
         indfile.time_to   = tform.to;
         indfile.ind_to    = last; 
         write(fd, &indfile, sizeof(indfile));
         close(fd);      
      }  
   }

   return 0;
}


int print_table(tformat_t * tform, u_int64_t * sc,  long double * sm)
{  char        * ptmpl;
   int           i;
   int           recs;
   int           rc;

   long double   summoney = 0;
   long double   sumpays  = 0;  // adder summary for unires mode (tform.res < 0)
   u_int64_t     sumcount = 0;

   logrec_t      inrec;
   u_int64_t     incount;
   long double   insum;

   logrec_t      outrec;
   u_int64_t     outcount;
   long double   outsum;

   int           istart;
   int           istop;
   
   *sc = 0;
   *sm = 0;

   memset(&inrec,  0, sizeof(inrec));
   incount = 0;
   insum   = 0;
   memset(&outrec, 0, sizeof(outrec));
   outcount = 0;
   outsum   = 0;

   inrec.time  = time(NULL);
   outrec.time = inrec.time;

   outrec.isdata.proto_id = 0x80000000;

// Get log records count
   recs = log_reccount(&Logbase);
   if (recs < 0) 
   {  syslog(LOG_ERR, "main(log_reccount): Error, stopped");
      return (-1);
   }

   if (fLine && !fNoZeros) printf("<tr>\n");

// Printf table caption
   if ((tform->title != NULL || fLine) && !fNoZeros)
   {  if (!fLine)
         printf("%s", tform->title);
      else
         printf("<td>%s</td><td><div align=right>%d</div></td>", 
                tform->title ? tform->title : "&nbsp;", tform->accno);
   }

// Table begin tag
   if (!fLine)
      printf("<table border=1 cellpadding=4 %s>\n", 
        tform->opts != NULL ? tform->opts : "class=\"txt\"");
     

// Print headers
   if (!fLine)
   {
   ptmpl = tform->fields;
   printf ("<tr align=center>\n");
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
   for (i=istart; i<istop; i++)
   {

// Get record
       rc = log_get(&Logbase, i, &logrec);
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

// Filter by accno (if given)
       if (fAll == 0)
          if (tform->accno >= 0 && logrec.accno != tform->accno) continue;
// Filter by resource (if given)
       if (tform->res >= 0   && logrec.isdata.res_id != tform->res) continue;
// Filter by direction
       if (fIn  == 0 && (logrec.isdata.proto_id & 0x80000000) == 0) continue;
       if (fOut == 0 && (logrec.isdata.proto_id & 0x80000000) != 0) continue;

       prnrec = logrec;

/*
 "DTABRCSE"
  Date, time, accno, balance, res, count, sum, result
 */
// Print record (or group it)
       if ((tform->flags & FLAG_DIRGROUP) == 0 )
       {  print_record(&logrec, 0, 0, tform);
       }
       else
       {  if (logrec.serrno != ACC_DELETED &&
              logrec.serrno != ACC_BROKEN)
          {  if ((logrec.isdata.proto_id &0x80000000) == NULL)
             {  insum    += logrec.sum;
                incount  += logrec.isdata.value;
             }
             else
             {  outsum   += logrec.sum;
                outcount += logrec.isdata.value;
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

   if ((off_flags & OFLAG_LAST) == 0 && last != 0) 
   {  off_flags |= OFLAG_LAST;
   }

   if (!fLine)
   {  if ((tform->flags & FLAG_DIRGROUP) != 0 )
      {  if (fIn)  print_record(&inrec, incount, insum, tform);
         if (fOut) print_record(&outrec, outcount, outsum, tform);
      }
   }
   else
   {  
      if (fNoZeros && ((incount | outcount) != 0 || insum >= 0.01 || outsum >= 0.01))
      {  printf("<tr>\n");
         printf("<td>%s</td><td><div align=right>%d</div></td>", 
                tform->title ? tform->title : "&nbsp;", tform->accno);
      }
      if (!fNoZeros || (incount | outcount) != 0 || insum >= 0.01 || outsum >= 0.01)
      {  print_line_record(incount, outcount, insum, outsum, tform);
         printf("</tr>\n");
      }  
   }
   
   if (!fLine)
   {
      if (sumpays != 0)
      {  printf("<tr><td><strong>Платежи:</strong></td>");
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
         printf("<tr><td><strong>Расход:</strong></td>");
      else
         printf("<tr><td><strong>Итого:</strong></td>");

      ptmpl = tform->fields + 1;
      while (*ptmpl != '\0')
      {  printf("<td>");
         switch(*ptmpl)
         {
            case 'C':   // count
               if (tform->flags & FLAG_SUMCOUNT)
               {  printf("<strong><div align=right>%llu", sumcount);
                  if (sumcount > 1048576) printf("<br>(%.2Lf M)", (long double)sumcount / 1048576);
                  else
                  if (sumcount > 1024) printf("<br>(%.2Lf K)", (long double)sumcount / 1024);
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
      printf("</table>\n");
   }

   *sc = sumcount;
   *sm = summoney;

   return 0;
}

int print_record(logrec_t * rec, u_int64_t count, long double sum, tformat_t * tform)
{  char       * ptmpl;
   struct tm    stm;


   localtime_r(&(rec->time), &stm); 

   ptmpl = tform->fields;
   printf ("<tr>\n");
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
            if (rec->isdata.res_id == 2)
            {  printf("&nbsp;");
               break;
            } 
            if (count == 0)
               printf("<div align=right>%lu</div>", rec->isdata.value);
            else
            {  printf("<div align=right>%llu ", count);
               if (count > 1048576)   printf("(%.2f M)", (double)count/1048576);
               else if (count > 1024) printf("(%.2f K)", (double)count/1024);
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
            if ((rec->isdata.proto_id & 0x80000000) == 0)
               printf("<center>IN</center>");
            else
               printf("<center>OUT</center>");
            break; 
         case 'H':
            if (rec->isdata.res_id == 2)
            {  printf("&nbsp;");
               break;
            } 
            printf("%s", inet_ntop(AF_INET, &(rec->isdata.host), buf, sizeof(buf)));
            break;
         case 'P':
            if (rec->isdata.res_id == 2)
            {  printf("&nbsp;");
               break;
            } 
            if (sum == 0)
               printf("<div align=right>%.2f</div>", -(rec->sum * 1048576 / rec->isdata.value));
            else
               printf("<div align=right>%.2Lf</div>", -(sum * 1048576 / count));
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

   ptmpl = tform->fields;
   while (*ptmpl != '\0')
   {  printf("<td>");
      switch(*ptmpl)
      {
         case 'C':   // count
            printf("<div align=right>%llu ", count_in);
            if (count_in > 1048576)   printf("(%.2f M)", (double)count_in/1048576);
            else if (count_in > 1024) printf("(%.2f K)", (double)count_in/1024);
            printf("</div></td><td>");

            printf("<div align=right>%llu ", count_out);
            if (count_out > 1048576)   printf("(%.2f M)", (double)count_out/1048576);
            else if (count_out > 1024) printf("(%.2f K)", (double)count_out/1024);
            printf("</div>");

            break;
         case 'S':   // sum
            printf("<div align=right>%+.2Lf</div>", sum_in + sum_out);
            break;
         case 'P':
            printf("<div align=right>%.2Lf</div>", -((sum_in + sum_out) * 1048576 / (count_in + count_out)));
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
"r N              - resource id 2-\"adder\" (default - 0, inet)\n"
"R                - all resources (analog -r -1)\n"
"g                - group data by direction\n"
"s                - count summary info\n"
"a N[:descr]      - account number (allowed multiply instances)\n"
"A                - ALL accounts\n"
"i                - skip inbound traffic\n"
"o                - skip outbound traffic\n"
"I file           - use one-range-index file (load & update)\n"
"c str            - options on <table> tag\n"
"d                - use full index (generated by logidx utility)\n"
"L                - line mode, output accounts on single table (forces -g)\n"
"z                - skip zero count/sum lines (forces -L & -g)"
"h                - suppress HTML-page prologue & epilogue\n"
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
"     H - client host\n\n"
);

}
