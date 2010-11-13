/* $RuOBSD: beeipf.c,v 1.6 2005/03/26 06:41:03 shadow Exp $ */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <syslog.h>
#include <unistd.h>

#include <bee.h>
#include <db.h>

typedef struct
{  u_int bhost;
   u_int bmask;
} hostspec_t;


char * head_add = "/usr/bin/rsh -l bee 192.168.213.201 \"access-template 100 BEE ";
char * tail_add = " any\"";

char * head_rm  = "/usr/bin/rsh -l bee 192.168.213.201 \"clear access-template 100 BEE ";
char * tail_rm  = " any\"";

char * rshead  = "";
char * rsepilog= "";

char * srchosts="/var/bee/allowed.inet";

char * srcrules="-";
char * dstrules="-";

char   namebuf[32]; 
char   addrbuf1[64]; 
char   addrbuf2[64]; 

char   buf[256];
char   hostname[80];

int          cnt_current = 0;
hostspec_t * itm_current = NULL;

int          cnt_allow   = 0;
hostspec_t * itm_allow   = NULL;

extern char * optarg;
void usage(int rc);

int ip_compare(void * one, void * two);

int main (int argc, char ** argv)
{
   char * ptr;
   char * str;
   FILE * f    = NULL;
   char   c;
   int    i, i2;

   hostspec_t  hostspec;


/*
    TODO:
 s   1. Redefine default filter source file (/etc/ipf.rules)
 t   2. Redefine default filter target file (/var/bee/ipf.rules.effecive)
 d   3. Redefine destination host (any)
 i   4. Redefine target interface (tun0)
 r   5. Redefine resource name (inet)
 f   6. Redefine allowed filename (/var/bee/allowed.inet)
 P   7. Redefine dest_s (NULL)
 p   8. Redefine host_s (NULL)
 m   9. Redefine filter file rsmark (#<beerules>)
 S  10. Redefine rule_s (NULL)
 R  11. Reverse in & out
 l  12. Redefine log mark(NULL)
 o  13. One rule only (to destination)
*/   

#define OPTS "s:t:d:i:r:f:P:p:m:S:Rl:oT:"

   while ((c = getopt(argc, argv, OPTS)) != -1)
   {  switch (c)
      {
      case 's':
         srcrules = optarg;
         break;
      case 't':
         dstrules = optarg;
         break;
      case 'r':
         snprintf(namebuf, sizeof(namebuf),"/var/bee/allowed.%s", optarg); 
         srchosts = namebuf;    
         break;	 
      case 'f':
         srchosts = optarg;
         break;	 
      default:
         usage(-1);
      }
   }

// 1. Load current acl

   if (strcmp(srcrules, "-") != 0)
   {  f=fopen(srcrules, "r");
      if (f==NULL)
      {  syslog(LOG_ERR, "fopen(%s): %m", srcrules);
         exit (-1);
      }
   }
   else f = stdin;

   while(fgets(buf, sizeof(buf), f) != NULL)
   {
      ptr = buf;

      memset(&hostspec, 0, sizeof(hostspec));

//      printf("%s", ptr);

      // rule check
      str = next_token(&ptr, " \t\n");   
      if (str == NULL) continue;
      if (strcmp(str, "permit") != 0) continue;

//      printf("got '%s' (permit)\n", str);

      str = next_token(&ptr, " \t\n");
      if (str == NULL) continue;
      if (strcmp(str, "ip") != 0) continue;
      
//      printf("got '%s' (ip)\n", str);

      // host/net specification
      str = next_token(&ptr, " \t\n");
      if (str == NULL) continue;
      if (strcmp(str, "host") == 0)
      {

//         printf("got '%s' (host)\n", str);

         // set host mask /32
         hostspec.bmask = 0xffffffff;
         str = next_token(&ptr, " \t\n");
         if (str == NULL) continue;
      }
      
      if (inet_pton(AF_INET, str, &(hostspec.bhost)) == 0) continue;

//      printf("got '%s' (ipaddr)\n", str);
    
      if (hostspec.bmask != 0xffffffff)
      {  
         // parse net wildcard
         str = next_token(&ptr, " \t\n");
         if (str == NULL) continue;
         if (inet_pton(AF_INET, str, &(hostspec.bmask)) == 0) continue;
         hostspec.bmask = ~hostspec.bmask;

//         printf("got '%s' (ipmask)\n", str);
      }

// attach host spec to list
      da_ins(&cnt_current, &itm_current, sizeof(*itm_current), (-1), &hostspec);
   }

   if (f != stdin) fclose(f);

// 2. Load allowed list

   if (strcmp(srchosts, "-") != 0)
   {  f=fopen(srchosts, "r");
      if (f==NULL)
      {  syslog(LOG_ERR, "open(%s): %m", srchosts);
         exit (-1);
      }
   }
   else f = stdin;

   while(fgets(buf, sizeof(buf), f) != NULL)
   {
      ptr = buf;

      memset(&hostspec, 0, sizeof(hostspec));

      str = next_token(&ptr, " \t\n");
      if (str == NULL) continue;

      if (make_addrandmask(str, &hostspec.bhost, &hostspec.bmask) < 0) continue;

// attach host spec to list
      da_ins(&cnt_allow, &itm_allow, sizeof(*itm_allow), (-1), &hostspec);
   }

   if (f != stdin) fclose(f);

// 3. Sort current acl

   da_bsort(&cnt_current, &itm_current, sizeof(*itm_current), ip_compare);

// 4. Sort allowed list

   da_bsort(&cnt_allow, &itm_allow, sizeof(*itm_allow), ip_compare);

// 5. Remove equal items


   i  = 0;
   i2 = 0;

   while(i < cnt_current && i2 < cnt_allow)
   {
      if (itm_current[i].bhost == itm_allow[i2].bhost &&
          itm_current[i].bmask == itm_allow[i2].bmask)
      {
          da_rm(&cnt_current, &itm_current, sizeof(*itm_current), i,  NULL);
          da_rm(&cnt_allow,   &itm_allow,   sizeof(*itm_allow),   i2, NULL);
          continue;   
      }

      if (ip_compare(itm_current+i, itm_allow+i2)) i2++;
      else i++;
   }


// 6. Generate additon commands from allow list remains
   
   if (strcmp(dstrules, "-") != 0)
   {  f = fopen(dstrules, "w");
      if (f == NULL)
      {  syslog(LOG_ERR, "fopen(%s): %m", dstrules);
         exit (-1);
      }
   }
   else f = stdout;

   for (i=0; i < cnt_allow; i++)
   {
      hostspec = itm_allow[i];
      hostspec.bmask = ~hostspec.bmask;

      fprintf(f, " %s %s %s%s\n",
	head_add,
        inet_ntop(AF_INET, &hostspec.bhost, addrbuf1, sizeof(addrbuf1)),
        inet_ntop(AF_INET, &hostspec.bmask, addrbuf2, sizeof(addrbuf2)),
        tail_add ? tail_add : "" );
   }

// 7. Generate removal commands from current list remains

   for (i=0; i < cnt_current; i++)
   {
      hostspec = itm_current[i];
      hostspec.bmask = ~hostspec.bmask;

      fprintf(f, " %s %s %s%s\n",
	head_rm,
        inet_ntop(AF_INET, &hostspec.bhost, addrbuf1, sizeof(addrbuf1)),
        inet_ntop(AF_INET, &hostspec.bmask, addrbuf2, sizeof(addrbuf2)),
        tail_rm ? tail_rm : "" );
   }

   if (f != stdout) fclose(f);

   return 0; 
}

void usage(int rc)
{
    fprintf(stderr,"%s",
 "beeipf - IPF rules generator\n"
 "Usage: beeipf [<switches>]\n" 
 "s - filter source file (default - /etc/ipf.rules)\n"
 "t - filter target file (default - /var/bee/ipf.rules.effecive)\n"
 "d - destination host   (default - \"any\")\n"
 "i - target interface   (default - none)\n"
 "r - resource name      (default - inet)\n"
 "f - hostlist filename  (default - /var/bee/allowed.inet)\n"
 "P - destination suffix (default - \"\")\n"
 "p - source suffix      (default - \"\")\n"
 "m - filter file mark   (default - \"#<beerules>\")\n"
 "S - rule suffix        (default - \"\")\n"
 "R - swap in & out      (default - no swap)\n"
 "l - word before \"on\"   (default - \"\")\n"
 "o - only one rule      (default - two rules)\n"
 "T - rule tail          (default - none)\n");
    exit(rc);
}

int ip_compare(void * one, void * two)
{
   if (((hostspec_t*)one)->bmask == ((hostspec_t*)two)->bmask)
   {  
      return ntohl(((hostspec_t*)one)->bhost) > ntohl(((hostspec_t*)two)->bhost);
   }
   
   return ((hostspec_t*)one)->bmask > ((hostspec_t*)two)->bmask;
}
