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

logrec_t    logrec;
logrec_t    prnrec;

char      * templ="DTABRCSEIH";
/*
 "DTABRCSE"
  Date, time, accno, balance, res, count, sum, result
 */

int        acc_cnt = 0;
int        acc_array[MAXRECS];
char     * acc_descr[MAXRECS];


char       buf[256];
char       titlebuf[256];

int        fAll = 0;
int        fIn  = 1;
int        fOut = 1;

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

   char       * ptr;
   char       * str;

   indexes_t    indfile;

   int          fd;

// Initialize table format
   memset(&tform, 0, sizeof(tform));

#define PARAMS "F:T:r:t:gsa:hAioI:"

   while ((rc = getopt(argc, argv, PARAMS)) != -1)
   {
      switch (rc)
      {
         case 'F':
            tform.from = parse_time(optarg);
            break;

         case 'T':
            tform.to = parse_time(optarg);
            break;

         case 'r':
            tform.res = strtol(optarg, NULL, 10);
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
               acc_array[acc_cnt++] = strtol(optarg, NULL, 10);
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

         default:
            printf("unexpected switch \"%c\"\n", rc);
      }
   }

// Load index file
   if (index_name != NULL)
   {  fd = open(index_name, O_RDONLY, 0777);
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
         close(fd);      
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
      {  if ((tform.flags & FLAG_DIRGROUP) != 0) tform.fields = "ICSH";
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
      printf("</head><body>\n");
      printf("<font face=\"Arial, Helvetica, sans-serif\">");
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

   for (i=0; i < acc_cnt; i++)
   {
      snprintf(titlebuf, sizeof(titlebuf), "Счет # %d (%s)", acc_array[i], acc_descr[i]);
      tform.accno  = acc_array[i];
      print_table(&tform, &wsc, &wsm);
      printf("<br>");
      sc += wsc;
      sm += wsm;
   }

   if (fsum != 0)
   {  printf("<br>\n<br>\n");
      printf("<H4>Всего:</H4><br>\n");
      if (tform.res != 2) 
      {  printf("<strong>счетчик: %.0Lf ", (long double)sc);
         if (sc > 1048576) printf("(%.0f M)", rint(sc/1048576));
         else
         {  if (sc > 1024) printf("(%.0f K)", rint(sc/1024));
         }
      }
      printf("</strong><br>\n");
      printf("<strong>сумма: %Lg ", sm);
   }

   if (headers != 0)
   {  
      printf("</center>");
      printf("</font>");
      printf("</body></html>\n");
   }
   
// Save index file
   if (index_name != NULL)
   {  fd = open(index_name, O_WRONLY | O_CREAT, 0777);
      if (fd >= 0)
      {  indfile.time_from = tform.from;
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

// Printf table caption
   if (tform->title != NULL)
      printf("%s", tform->title);

// Table begin tag
   printf("<table border=1 cellpadding=4>\n");

// Print headers
   ptmpl = tform->fields;
   printf ("<tr>\n");
   while (*ptmpl != '\0')
   {  printf("<th>");
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
         default:
            printf("?");
      }
          printf("</th>\n");
          ptmpl++;
   }
   printf("</tr>\n"); 

   istart = 0;
   istop  = recs;

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
       {  if (logrec.errno != ACC_DELETED &&
              logrec.errno != ACC_BROKEN)
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


// Sum counts
       sumcount += logrec.isdata.value;
// Sum money
       summoney += logrec.sum;
   }

   if ((off_flags & OFLAG_LAST) == 0 && last != 0) 
   {  off_flags |= OFLAG_LAST;
   }

   if ((tform->flags & FLAG_DIRGROUP) != 0 )
   {  if (fIn)  print_record(&inrec, incount, insum, tform);
      if (fOut) print_record(&outrec, outcount, outsum, tform);
   }
   
   printf("<tr><td><strong>Итого:</strong></td>");
   ptmpl = tform->fields + 1;
   while (*ptmpl != '\0')
   {  printf("<td>");
      switch(*ptmpl)
      {
         case 'C':   // count
            if (tform->flags & FLAG_SUMCOUNT)
            {  printf("<strong><div align=right>%llu", sumcount);
               if (sumcount > 1048576) printf("<br>(%llu M)", sumcount/1048576);
               else
               if (sumcount > 1024) printf("<br>(%llu K)", sumcount/1024);
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
            if (count == 0)
               printf("<div align=right>%lu</div>", rec->isdata.value);
            else
               printf("<div align=right>%llu</div>", count);
            break;
         case 'S':   // sum
            if (sum == 0)
               printf("<div align=right>%+.2f</div>", rec->sum);
            else
               printf("<div align=right>%+.2Lf</div>", sum);
            break;
         case 'E':   // result
            printf("(%d)", rec->errno);
            break;
         case 'I':
            if ((rec->isdata.proto_id & 0x80000000) == 0)
               printf("<center>IN</center>");
            else
               printf("<center>OUT</center>");
            break; 
         case 'H':
            printf("%s", inet_ntop(AF_INET, &(rec->isdata.host), buf, sizeof(buf)));
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

