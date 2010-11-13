#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>

#include <bee.h>
#include <db.h>
#include <res.h>
#include <links.h>

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

int          cnt_recs;
parserec_t * itm_recs;

int    fappend     = 0;
int    funzip      = 0;

char * output_file = NULL;
char * input_file  = NULL;

unsigned long long total_in  = 0;
unsigned long long total_out = 0;

int cmprecs(void * one, void * two);
void usage();
FILE * open_input(const char *path);
int close_input(FILE * fd);

int main(int argc, char ** argv)
{
   FILE * fd_in  = NULL;
   int    fd_out = (-1);

   struct stat  sb;
   size_t       bytes;   

   off_t  offs;

   int    c, rc;

   while ((c = getopt(argc, argv, "o:Oz")) != -1)
   {  switch (c)
      {
         case 'o':
            output_file = optarg;
            if (strcmp(optarg, "-") == 0) fd_out = fileno(stdout);
            break;

         case 'O':
            fappend = 1;
            break;

         case '?': 
         default:
            usage();
            exit(0);
      }
   }     

   argc -= optind;
   argv += optind;

// Open input file
   if (argc)
   {  input_file = *(argv++);
      argc--;
      if (strcmp(input_file, "-") == 0) fd_in  = stdin;
   }

   if (input_file && fd_in == NULL)
   {  fd_in = open_input(input_file);
      if (fd_in == NULL)
      {  fprintf(stderr, "%s: %s\n", input_file, strerror(errno));
         return (-1);
      }
   }

// Allocate buffer
   rc = fstat(fileno(fd_in), &sb);
   if (rc < 0)
   {  syslog(LOG_ERR, "fstat(): %m");
      exit(-1);
   }

   itm_recs = calloc(1, sb.st_size);
   if (itm_recs == NULL)
   {  syslog(LOG_ERR, "calloc(%lld): %m", sb.st_size);
      exit(-1);
   }
   cnt_recs = sb.st_size / sizeof(*itm_recs);

// Load file
   bytes = fread(itm_recs, 1, sb.st_size, fd_in);
   if (bytes < sb.st_size)
   {  syslog(LOG_ERR, "fread(%lld): %m (bytes=%ld)", sb.st_size, bytes);
      exit(-1);
   }

   fprintf(stderr, "size=%lld, recs=%d, size=%lu\n", sb.st_size, cnt_recs, sizeof(*itm_recs));

// Close input file
   close_input(fd_in);

// Sort file
   rc = da_bsort(&cnt_recs, &itm_recs, sizeof(*itm_recs), cmprecs);
   if (rc < 0)
   {  syslog(LOG_ERR, "da_bsort(): error");
      exit(-1);
   }

   fprintf(stderr, "opening %s\n", output_file);

// Open output file
   if (output_file)
   {  if (strcmp(output_file, "-") == 0) fd_out = fileno(stdout);

      if (fd_out < 0)
      {  if (fappend != 0) 
            fd_out = open(output_file, O_WRONLY | O_CREAT, 0777);
         else 
            fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0777);
         if (fd_out < 0)
         {  fprintf(stderr, "%s: %s\n", output_file, strerror(errno));
            return (-1);
         }
   // Seek to end for append mode
         if (fappend != 0)
         {  offs = lseek(fd_out, 0, SEEK_END);
      // check record alignment
            if (offs % sizeof(parserec_t) != 0)
            {  syslog(LOG_ERR, "Warning: output file is corrupted, correcting size");
               lseek(fd_out, offs - (offs % sizeof(parserec_t)), SEEK_SET);
            }
         }
      }
   }

   fprintf(stderr, "writing %d\n", cnt_recs);

   bytes = write(fd_out, itm_recs, cnt_recs * sizeof(*itm_recs));
   if (bytes < cnt_recs * sizeof(*itm_recs))
   {  syslog(LOG_ERR, "fwrite(%lld): %m (rc=%ld)", sb.st_size, bytes);
      exit(-1);
   }

   fprintf(stderr, "success\n");

   close(fd_out);

   return 0;
}

int cmprecs(void * one, void * two)
{
//   fprintf(stderr, "%p & %p\n", one, two);

   return ((parserec_t*)(one))->tstamp
              >
          ((parserec_t*)(two))->tstamp;
}

FILE * open_input(const char *path)
{
   return fopen(path, "r");
}

int close_input(FILE * fd)
{
      return fclose(fd);
}

void usage()
{
   fprintf(stderr, 
"BEEPARSE - parse & filter detailed statistics (cnupm(8) collector data)\n\n"
"  USAGE:\n"
"     cat cnupmstat.dump | beeparse [switches] [-o binfile] [-q]\n"
"     beeparse [switches] file [file2 [...]]\n"
"     beeparse [switches] -z file.gz [file2.gz [...]]\n"
"\n"
"  INPUT:\n"
"no args - text dump file from stdin\n"
"args    - binary file list (\"-\" - binary from stdin)\n"
"-z      - filter input files (binaries) through zcat(1)\n"
"\n"
"  FILTER:\n"
"-n CIDR - filter given CIDR range (multi)\n"
"-N CIDR - exclude given CIDR range (multi)\n"
"-F DD:MM:YYYY[:hh:mm[:ss]] - time to filter from\n"
"-T DD:MM:YYYY[:hh:mm[:ss]] - time to filter to\n"
"-a N    - include all gates from account (multi)\n"
"-l list - include all gates from listed accounts (multi)\n"
"-p N    - IP protocol number to filter\n"
"\n"
"  TEXT OUTPUT:\n"
"default - cnupm(8) compatible output\n"
"-q      - no output (for saving binary only)\n"
"-c      - output w/ separate port numbers\n"
"-d      - direction:local:remote output\n"
"-t secs - period average statistics output\n"
"-s      - summary by local output\n"
"-S      - summary by remote output\n"
"-h      - human read output for summary\n"
"-P      - port summary output\n"
"-D      - DNS reverse-lookup summary items\n"
"\n"
"  BINARY SAVING (exclude text parsing):\n"
"default - no binary\n"
"-o file - filename to write binary (\"-\" - binary to stdout)\n"
"-O      - append binary (w/ alignment correction)\n"
"\n"
);
}
