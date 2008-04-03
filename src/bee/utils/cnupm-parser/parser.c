#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>

#include <bee.h>
#include <db.h>
#include <res.h>
#include <links.h>

//#define IP(o1, o2, o3, o4) ((o4) + (o3)*0x100 + (o2)*0x10000 + (o1)*0x1000000u)
#define IP(o1, o2, o3, o4) ((o1) + (o2)*0x100 + (o3)*0x10000 + (o4)*0x1000000u)

#define DELIM1      " \t\n\r"
#define EXCLUSIONS  (32768)

typedef struct
{  time_t    tstamp;
   u_int     src_ip;
   int       src_port;
   u_int     dst_ip;
   int       dst_port;
   int       proto;
   unsigned long long count;
} parserec_t;

typedef struct
{  time_t    tstamp;
   unsigned long long count_in;
   unsigned long long count_out;
} timerec_t;

typedef struct
{  u_int              src_ip;
   unsigned long long count_in;
   unsigned long long count_out;
   int                port_local;
   int                port_remote;
   int                recs;
} ipgroup_t;

typedef struct
{  u_int  addr;
   u_int  mask;
   int    flag;
} exclitem_t;

char   inbuf[512];
char   addrbuf1[32];
char   addrbuf2[32];
char   timebuf[48];

// Inclusions/exclusions list
exclitem_t  * itm_exclist = NULL;
int           cnt_exclist = 0;

int           itm_accs[EXCLUSIONS];
int           cnt_accs = 0;

ipgroup_t   * itm_group = NULL;
int           cnt_group = 0;

ipgroup_t   * itm_groupdst = NULL;
int           cnt_groupdst = 0;

int           cnt_loclist  = 9;
exclitem_t    itm_loclist[]=
{  { IP(192,168,0,0),    IP(255,255,0,0),     0},
   { IP(172,16,0,0),     IP(255,240,0,0),     0},
   { IP(10,0,0,0),       IP(255,0,0,0),       0},

   { IP(217,150,206,0),  IP(255,255,255,0),   0},
   { IP(217,150,206,10), IP(255,255,255,255), 1},
   { IP(82,194,234,0),   IP(255,255,254,0),   0},

   { IP(82,194,234,10),  IP(255,255,255,255), 1},
   { IP(89,208,94,0),    IP(255,255,255,0),   0},
   { IP(89,208,94,10),   IP(255,255,255,255), 1}
};

int    fproto      = (-1);
int    fstats      = 0;
int    fstatsd     = 0;
int    fdir        = 0;
int    fcompat     = 1;
int    fhuman      = 0;
int    fports      = 0;
int    fdns        = 0;
int    fquiet      = 0;
time_t time_from   = 0;
time_t time_to     = 0;
time_t tslice      = 0;
char * input_file  = NULL;
char * output_file = NULL;

unsigned long long total_in  = 0;
unsigned long long total_out = 0;

time_t  parse_time (char * strdate);
time_t  parse_time2(char * strdate);
int cmpgroups      (void * one, void * two);
int cmpgroups_ip   (void * one, void * two);
int cmpgroups_recs (void * one, void * two);
int cmpproto       (void * one, void * two);
void *  read_blk(int fd);

char   date_buf[128];

parserec_t txt_prec;

int main(int argc, char ** argv)
{  int    lineskip = 0;
   u_int  line     = 0;

   parserec_t * rec = &txt_prec;
   timerec_t    timerec = {0, 0, 0};
   exclitem_t   exclusion;
   int    rc;
   int    flag_to   = 0;
   int    flag_from = 0;
   int    flag      = 0;
   int    i, n;
   char   c;

   char * ptr;
   char * ptr2;
   char * str;
 
   int    fd_in  = (-1);
   int    fd_out = (-1);

   struct hostent * hent = NULL;

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

   while ((c = getopt(argc, argv, "n:N:p:dF:T:sSchPDl:t:i:o:a:q")) != -1)
   {  switch (c)
      {
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

               if (c == 'N') exclusion.flag = 1;

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
            i = (-1);
            n = cnt_accs;
            exclusion.flag = 0;
            while (lookup_resname(RES_LIST, optarg, &i) >= 0)
            {  if (cnt_accs < EXCLUSIONS)
               {  itm_accs[cnt_accs] = linktab[i].accno;
                  cnt_accs++;
               }
               else
                  fprintf(stderr, "ERROR: account table overflow\n");
            }

            break;

         case 'd':
            fdir = 1;
            break;

         case 's':
            fstats = 1;
            break;

         case 'S':
            fstatsd = 1;
            break;

         case 'p':
            fproto = strtol(optarg, NULL, 10);
            break;

         case 'F':
            time_from = parse_time(optarg);
            break;

         case 'T':
            time_to = parse_time(optarg);
            break;

         case 'c':
            fcompat = 0;
            break;

         case 'h':
            fhuman = 1;
            break;

         case 'P':
            fports = 1;
            break;

         case 'D':
            fdns = 1;
            break;

         case 't':
            tslice = strtol(optarg, NULL, 10);
            break;
 
         case 'i':
            input_file = optarg;
            break;

         case 'o':
            output_file = optarg;
            break;

         case 'a':
            itm_accs[cnt_accs] = strtol(optarg, NULL, 10);
            break;

         case 'q':
            fquiet = 1;
            break;
      }
   }     

/*
   fprintf(stderr, "ACCs: %d\n", cnt_accs);
   for (i=0; i < cnt_accs; i++)
   {
      fprintf(stderr, "%d, ", itm_accs[i]);
   }
*/

   for (n=0; n < cnt_accs; n++)
   {  i = (-1);
      while (lookup_accres(itm_accs[n], RES_INET, &i) >= 0)
      {
         rc = make_addrandmask (linktab[i].username, &(exclusion.addr), &(exclusion.mask));
         if (rc < 0)
         {  fprintf(stderr, "ERROR - Gate parse error (\"%s\", #%d)\n", linktab[i].username, itm_accs[n]);
            break;
         }

         rc = da_ins(&cnt_exclist, &itm_exclist, sizeof(*itm_exclist), (-1), &exclusion);
         if (rc < 0)
         {  fprintf(stderr, "ERROR - Address \"%s\" (#%d) system error\n", linktab[i].username, itm_accs[n]);
         }
      }   
   }

   cnt_accs = 0;           

/*
   fprintf(stderr, "IPs: %d\n", cnt_exclist);
   for (i=0; i < cnt_exclist; i++)
   {
      fprintf(stderr, "%s mask %s\n", 
             inet_ntop(AF_INET, &(itm_exclist[i].addr), addrbuf1, sizeof(addrbuf1)),
             inet_ntop(AF_INET, &(itm_exclist[i].mask), addrbuf2, sizeof(addrbuf2))  );
   }
*/

// Open input file
   if (input_file)
   {  fd_in = open(input_file, O_RDONLY, 0);
      if (fd_in < 0)
      {  fprintf(stderr, "%s: %s\n", input_file, strerror(errno));
         return (-1);
      }
   }

// Open output file
   if (output_file)
   {  fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0777);
      if (fd_out < 0)
      {  fprintf(stderr, "%s: %s\n", output_file, strerror(errno));
         return (-1);
      }
   }

   while(1)
   {
      if (input_file != NULL)
      {  rec = read_blk(fd_in);
         if (rec == NULL) break;
      } 
      else
      {  if (fgets(inbuf, sizeof(inbuf), stdin) == NULL) break;
         line++;

      // long line trap
         if (strchr(inbuf, '\n') == NULL)
         {  if (lineskip < 1) fprintf(stderr, "line too long (%u)\n", line);
            line--;
            lineskip += 1;
            continue;
         }

      // skip rest of long line
         if (lineskip > 0)
         {  lineskip -= 1;
            continue;
         }

         ptr = inbuf;
         memset(rec, 0, sizeof(*rec));

      // date
         str = next_token(&ptr, DELIM1);
         if (str == NULL) continue;

      // time
         ptr2 = next_token(&ptr, DELIM1);
         if (ptr2 == NULL) continue;
         snprintf(date_buf, sizeof(date_buf), "%s %s", str, ptr2);

      // src IP[:port]
         str  = next_token(&ptr, DELIM1);
         if (str == NULL) continue;
         ptr2 = str;
         str  = next_token(&ptr2, ":");
         if (str == NULL) continue;
         rc = inet_pton(AF_INET, str, &(rec->src_ip));
         if (rc < 0) continue;

         str = next_token(&ptr2, ":");
         if (str != NULL) rec->src_port = strtol(str, NULL, 10);

      // dst IP[:port]
         str  = next_token(&ptr, DELIM1);
         if (str == NULL) continue;
         ptr2 = str;
         str  = next_token(&ptr2, ":");
         if (str == NULL) continue;
         rc = inet_pton(AF_INET, str, &(rec->dst_ip));
         if (rc < 0) continue;

         str = next_token(&ptr2, ":");
         if (str != NULL) rec->dst_port = strtol(str, NULL, 10);
      
      // IP proto
         str = next_token(&ptr, DELIM1);
         if (str == NULL) continue;
         rec->proto = strtol(str, NULL, 10);
      }

// Filter by proto
      if (fproto >= 0 && fproto != rec->proto) continue;

      if (input_file == NULL)
      {
      // bytes
         str = next_token(&ptr, DELIM1);
         if (str == NULL) continue;
         rec->count = strtoll(str, NULL, 10);
      }

// Skip stub records
      if (rec->dst_ip == 0 && rec->src_ip == 0 && rec->count == 0) continue;

// Parse timestamp if needed
      if (input_file == NULL)
      {
      // count UTC
         if (time_from != 0 || time_to != 0 || tslice > 0 || output_file)
         {  rec->tstamp = parse_time2(date_buf);
            if (rec->tstamp == 0) continue;
         }
      }

// Filter by time
      if (time_from != 0 || time_to != 0)
      {  if (time_from != 0 && rec->tstamp < time_from) continue;
         if (time_to   != 0 && rec->tstamp > time_to)   break;
      }

// Filter by IP list
      if (cnt_exclist > 0)
      {  flag_to   = 0;   // i.e. skip by default
         flag_from = 0;

         for (i=0; i < cnt_exclist; i++)
         {  if ((rec->src_ip & itm_exclist[i].mask) == itm_exclist[i].addr)
               flag_from = (itm_exclist[i].flag & 1) == 0;
            if ((rec->dst_ip & itm_exclist[i].mask) == itm_exclist[i].addr)
               flag_to   = (itm_exclist[i].flag & 1) == 0;
         }

         if ((flag_to | flag_from) == 0) continue;
      }

// Write binary file
      if (output_file != NULL)
      {  rc = write(fd_out, rec, sizeof(*rec));
         if (rc < sizeof(*rec)) 
         {  fprintf(stderr, "ERROR: write() error\n");
            break;
         }
      }

// Skip output on quiet mode
      if (fquiet) continue;

// Direction 
      {  flag_to   = 0;   // global by default
         flag_from = 0;

         for (i=0; i < cnt_loclist; i++)
         {  if ((rec->src_ip & itm_loclist[i].mask) == itm_loclist[i].addr)
               flag_from = (itm_loclist[i].flag & 1) == 0;
            if ((rec->dst_ip & itm_loclist[i].mask) == itm_loclist[i].addr)
               flag_to   = (itm_loclist[i].flag & 1) == 0;

         }
      }

// Timeslice output
      if (tslice > 0)
      {  if (timerec.tstamp == 0) timerec.tstamp = (rec->tstamp / tslice) * tslice;

         if (timerec.tstamp != ((rec->tstamp / tslice) * tslice))
         {  printf("%d\t%llu\t%llu\n",
              timerec.tstamp,
              (timerec.count_in  + tslice/2) / tslice,
              (timerec.count_out + tslice/2) / tslice);

            timerec.tstamp    = (rec->tstamp / tslice) * tslice;
            timerec.count_in  = 0;
            timerec.count_out = 0;
         }
         else
         {  timerec.count_in  += flag_from ? 0 : rec->count;
            timerec.count_out += flag_from ? rec->count : 0;
         }
         continue;
      }
        
// Summary statistics output
      if (fstats || fstatsd)
      {
         total_in  += flag_from ? 0 : rec->count;
         total_out += flag_from ? rec->count : 0;

         flag = fstats ? flag_from : flag_to;

         for(i=0; i<cnt_group; i++)
         {
            if (!fports)
            {  if (itm_group[i].src_ip == (flag ? rec->src_ip : rec->dst_ip))
               {  itm_group[i].count_in  += flag_from ? 0 : rec->count;
                  itm_group[i].count_out += flag_from ? rec->count : 0;
                  itm_group[i].recs ++;
                  break;
               }
            }
            else
            {  if (itm_group[i].src_ip      == (flag ? rec->src_ip : rec->dst_ip) &&
                   itm_group[i].port_local  == (flag_from ? rec->src_port : rec->dst_port) &&
                   itm_group[i].port_remote == (flag_from ? rec->dst_port : rec->src_port))
               {  itm_group[i].count_in  += flag_from ? 0 : rec->count;
                  itm_group[i].count_out += flag_from ? rec->count : 0;
                  itm_group[i].recs ++;
                  break;
               }
            }
         }
         if (i >= cnt_group)
         {  da_new(&(cnt_group), &(itm_group), sizeof(*itm_group), (-1));
            memset(itm_group, 0, sizeof(*itm_group));
            itm_group[i].src_ip    = flag      ? rec->src_ip : rec->dst_ip;
            itm_group[i].count_in  = flag_from ? 0 : rec->count;
            itm_group[i].count_out = flag_from ? rec->count : 0;

            if (fports)
            {  itm_group[i].port_local  = flag_from ? rec->src_port : rec->dst_port;
               itm_group[i].port_remote = flag_from ? rec->dst_port : rec->src_port;
            }
         }
         continue;
      }

// Assemble time string for output
//      if (input_file)
      if (rec->tstamp != 0)
         strftime(date_buf, sizeof(date_buf), "%Y-%m-%d %H:%M:%S", localtime(&(rec->tstamp)));

// IN/OUT output
      if (fdir)
      {
         printf("%s\t%s\t%s\t%d\t%s\t%d\t%d\t%llu\n",
              date_buf,
              flag_from ? "OUT":"IN",
              flag_from ? inet_ntop(AF_INET, &(rec->src_ip), addrbuf1, sizeof(addrbuf1)) : inet_ntop(AF_INET, &(rec->dst_ip), addrbuf2, sizeof(addrbuf2)), 
              flag_from ? rec->src_port : rec->dst_port,
              flag_from ? inet_ntop(AF_INET, &(rec->dst_ip), addrbuf2, sizeof(addrbuf2)) : inet_ntop(AF_INET, &(rec->src_ip), addrbuf1, sizeof(addrbuf1)), 
              flag_from ? rec->dst_port : rec->src_port,
              rec->proto,
              rec->count);
         continue;
      }

// Normal output
      printf("%s\t%s%s%d\t%s%s%d\t%d\t%llu\n",
              date_buf,
              inet_ntop(AF_INET, &(rec->src_ip), addrbuf1, sizeof(addrbuf1)),
              fcompat ? ":" : "\t",
              rec->src_port,
              inet_ntop(AF_INET, &(rec->dst_ip), addrbuf2, sizeof(addrbuf2)), 
              fcompat ? ":" : "\t",
              rec->dst_port,
              rec->proto,
              rec->count);

   } // while

// Close files
   if (input_file)  close(fd_in);
   if (output_file) close(fd_out);

   if (!fports)
      da_bsort(&(cnt_group), &(itm_group), sizeof(*itm_group), cmpgroups);
   else
   {
      da_bsort(&(cnt_group), &(itm_group), sizeof(*itm_group), cmpgroups_recs);
      da_bsort(&(cnt_group), &(itm_group), sizeof(*itm_group), cmpgroups_ip);
   }   

   if (fstats || fstatsd)
   { 
      for(i=0; i<cnt_group; i++)
      {
         if (fdns) hent = gethostbyaddr(&(itm_group[i].src_ip), sizeof(itm_group[i].src_ip), AF_INET);
         if (!fports)
         {
         if (!fhuman)
         {
                printf("%15s %10llu %10llu %10llu %s\n",
                inet_ntop(AF_INET, &(itm_group[i].src_ip), addrbuf1, sizeof(addrbuf1)),
                itm_group[i].count_in,
                itm_group[i].count_out,
                itm_group[i].count_in + itm_group[i].count_out,
                hent ? hent->h_name : "");
         }
         else
         {  printf("%15s %8lluMB %8lluMB %8lluMB %8llu%% %s\n",
                inet_ntop(AF_INET, &(itm_group[i].src_ip), addrbuf1, sizeof(addrbuf1)),
                (itm_group[i].count_in  + 524288) / 1048576,
                (itm_group[i].count_out + 524288) / 1048576,
                (itm_group[i].count_in + itm_group[i].count_out + 524288) / 1048576,
                (((itm_group[i].count_in + itm_group[i].count_out) * 100) + ((total_in+total_out)/2)) / (total_in+total_out),
                hent ? hent->h_name : "");
         }
         }
         else 
         {  printf("%15s %10llu %10llu %10llu %8d %8d %8d\n",
                inet_ntop(AF_INET, &(itm_group[i].src_ip), addrbuf1, sizeof(addrbuf1)),
                itm_group[i].count_in,
                itm_group[i].count_out,
                itm_group[i].count_in + itm_group[i].count_out,
                itm_group[i].port_local,
                itm_group[i].port_remote,
                itm_group[i].recs);
         }
         
      }
         if (!fhuman)
            printf("        (total) %10llu %10llu %10llu\n", total_in, total_out, total_in+total_out);
         else
            printf("        (всего) %8lluMB %8lluMB %8lluMB\n", 
                   (total_in + 524288) / 1048576, 
                   (total_out + 524288) / 1048576, 
                   (total_in+total_out + 524288) / 1048576);
   }

   return 0;
}

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

time_t  parse_time2(char * strdate)
{  char      * ptr = strdate;
   char      * str;
   struct tm   stm;
   time_t      rval;
   int         temp;

   memset(&stm, 0, sizeof(stm));

// year
   str = next_token(&ptr, delim);
   if (str == NULL) return 0;
   temp = strtol(str, NULL, 10);
   if (temp < 100) temp += 100;
   else temp -= 1900;
   if (temp < 0) return 0;
   stm.tm_year = temp;

// month
   str = next_token(&ptr, delim);
   if (str == NULL) return 0;
   stm.tm_mon = strtol(str, NULL, 10) - 1;

// month day
   str = next_token(&ptr, delim);
   if (str == NULL) return 0;
   stm.tm_mday = strtol(str, NULL, 10);

// hour
   str = next_token(&ptr, delim);
   if (str == NULL) return 0;
   stm.tm_hour = strtol(str, NULL, 10);

// minute
   str = next_token(&ptr, delim);
   if (str == NULL) return 0;
   stm.tm_min = strtol(str, NULL, 10);

// seconds
   str = next_token(&ptr, delim);
   if (str == NULL) return 0;
   stm.tm_sec = strtol(str, NULL, 10);

// assemble time_t
   stm.tm_isdst = -1;
   rval = mktime(&stm);
   if (rval < 0) return 0;

   return rval;
}

int cmpgroups(void * one, void * two)
{
   return ((ipgroup_t*)(one))->count_in + ((ipgroup_t*)(one))->count_out 
              < 
          ((ipgroup_t*)(two))->count_in + ((ipgroup_t*)(two))->count_out;
}

int cmpgroups_ip (void * one, void * two)
{
   return ((ipgroup_t*)(one))->src_ip < ((ipgroup_t*)(two))->src_ip;
}

int cmpgroups_recs (void * one, void * two)
{
   return ((ipgroup_t*)(one))->recs < ((ipgroup_t*)(two))->recs;
}

parserec_t read_buf[1024];
int        blocks         = 0;
int        next_block     = 0;

void * read_blk(int fd)
{  int rc;

   if (next_block >= blocks)
   {  next_block = 0;
      blocks     = 0;
      rc = read(fd, read_buf, sizeof(read_buf));
      if (rc <= 0) return NULL;
      blocks = rc / sizeof(parserec_t);
   }

   return read_buf + next_block++;
}
