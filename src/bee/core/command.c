/* $RuOBSD: command.c,v 1.40 2007/10/03 09:31:27 shadow Exp $ */

#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <time.h>
#include <syslog.h>

#include <bee.h>
#include <ipc.h>
#include <db.h>
#include <sql.h>
#include <db2.h>
#include <core.h>
#include <command.h>
#include <res.h>
#include <links.h>
#include <tariffs.h>
#include <version.h>

#include "queries.h"

char   argbuf1[128];
char   argbuf2[128];
char   argbuf3[128];
char   argbuf4[128];
char   argbuf5[128];
char   argbuf6[128];

char   NullWord[]="NULL";
char   NullCmd[]="";

// Command tables
command_t     cmds[];     // main command table
/*
command_t     acccmds[];  // acc  command subtable
command_t     perscmds[]; // person command subtable
command_t     gatecmds[]; // gate command subtable
*/

command_t     cardcmds[]; // card command subtable

command_t  cmds[] =
{  {"exit",	cmdh_exit,	0,	NULL},  // close session
   {"ver",	cmdh_ver,       0,      NULL},  // get version number
   {"user",     cmdh_notimpl,	0,      NULL},  // *** username (human id)
   {"pass",	cmdh_notimpl,	1,      NULL},  // *** password (human id)
   {"authkey",	cmdh_notimpl,	0,      NULL},  // *** public key (machine id)
   {"authsign",	cmdh_notimpl,   1,      NULL},  // *** signature (machine id)
   {"acc",      cmdh_acc,	4,      NULL},  // show account state
   {"look",	cmdh_lookup,	4,      NULL},  // lookup links
   {"log",	cmdh_log,	4,      NULL},  // view transactions log
   {"freeze",	cmdh_freeze,	4,      NULL},  // freeze account
   {"unfreeze",	cmdh_freeze,	4,      NULL},  // unfreeze account
   {"on",	cmdh_freeze,	4,      NULL},  // turn account on
   {"off",	cmdh_freeze,	4,      NULL},  // turn account off
   {"unlimit",	cmdh_freeze,	4,      NULL},  // set account to unlimit
   {"limit",	cmdh_freeze,	4,      NULL},  // turn account off
   {"_break",	cmdh_freeze,	4,      NULL},  // break down account
   {"payman",	cmdh_freeze,	4,      NULL},  // allow payman for account
   {"nopayman",	cmdh_freeze,	4,      NULL},  // deny payman for account
   {"_rstsumm",	cmdh_freeze,	4,      NULL},  // reset account summary info
   {"_fix",	cmdh_fix,	4,      NULL},  // validate account
   {"_dump",	cmdh_notimpl,	4,      NULL},  // *** dump account record
   {"_save",	cmdh_notimpl,	4,      NULL},  // *** store dump to account
   {"new",	cmdh_new,	4,      NULL},  // create new account
   {"add",	cmdh_add,	4,      NULL},  // do add transaction 
   {"res",	cmdh_res,	4,      NULL},  // do resource billing transaction
   {"_hres",	cmdh_hres,	4,      NULL},  // (HACKED) do resource billing transaction 
   {"update",	cmdh_update,	4,      NULL},  // set flag to update filters
   {"human",    cmdh_human,     0,      NULL},  // suppress non-human messages
   {"machine",  cmdh_human,	0,      NULL},  // suppress human comments
   {"date",	cmdh_date,	0,      NULL},  // show time/date
   {"report",	cmdh_report,	4,      NULL},  // show report
   {"del",      cmdh_delete,    4,      NULL},  // (alias) delete account
   {"delete",   cmdh_delete,    4,      NULL},  // delete account
   {"gate",	cmdh_gate,	4,      NULL},  // (alias) add gate
   {"addgate",	cmdh_gate,	4,      NULL},  // add gate
   {"delgate",	cmdh_delgate,	4,      NULL},  // delete gate
   {"allow",	cmdh_notimpl,	4,      NULL},  // allow gate usage
   {"disallow",	cmdh_notimpl,	4,      NULL},  // disallow gate usage
   {"new_contract", cmdh_new_contract, 4,      NULL},  // MACRO create two accounts, return #
   {"new_name", cmdh_new_name,  4,      NULL},  // MACRO create user & return password
   {"new_host", cmdh_notimpl,   4,      NULL},  // MACRO add new host
   {"new_vpn",  cmdh_new_vpn,   4,      NULL},   // MACRO add new host
   {"intraupdate",cmdh_intraupdate, 4,      NULL},	// MACRO call "intra" update script
   {"setstart", cmdh_setstart,  4,      NULL},  // set account start date
   {"setstop",	cmdh_setstart,  4,      NULL},  // set account stop (expire) date
   {"lock",	cmdh_lock,  	4,      NULL},  // lock account & log bases
   {"unlock",	cmdh_lock,  	4,      NULL},  // unlock account & log bases
   {"_tariff",	cmdh_accres,  	4,      NULL},  // set tariff number (old alias)
   {"tariff",	cmdh_accres,  	4,      NULL},  // set tariff number
   {"docharge", cmdh_docharge, 	4,      NULL},  // daily charge tick

   {"card",           NULL,             PERM_NONE,      cardcmds}, // card commands
   {"cards",          NULL,             PERM_NONE,      cardcmds}, // --//-- (alias)
   
// debug commands (none)
   {"_tdump",	cmdh_tdump,  	4,      NULL},  // tariffs dump
   {"debug",          cmdh_debug,       PERM_SUPERUSER, NULL},     // SQL debug on/off

   {NULL, NULL, 0, NULL}       // terminator
};

command_t     cardcmds[]=
{
   {NullCmd,          cmdh_card,        PERM_NONE,      NULL},  // list active cards
   {"gen",            cmdh_card_gen,    PERM_NONE,      NULL},  // generate cards
   {"list",           cmdh_card_list,   PERM_NONE,      NULL},  // list unprinted batches
   {"emit",           cmdh_card_emit,   PERM_NONE,      NULL},  // mark batch as printed
   {"check",          cmdh_card_check,  PERM_NONE,      NULL},  // check card
   {"null",           cmdh_card_null,   PERM_NONE,      NULL},  // annul card
   {"nullbatch",      cmdh_card_nullbatch, PERM_NONE,   NULL},  // annul batch
   {"expire",         cmdh_card_expire, PERM_NONE,      NULL},  // check & annul expired cards
   {"utluser",        cmdh_card_utluser, PERM_NONE,     NULL},  // utilize card by user
   {"useractivate",   cmdh_card_utluser, PERM_NONE,     NULL},  // --//-- (old alias)

   {NULL,NULL,0,NULL}       // terminator
};

char * errmsg[] =
{  "Access Denied",              // 400
   "Unknown command",		 // 401
   "Command not implemented",    // 402
   "Incorrect argument count",   // 403
   "Invalid argument",           // 404
   "I/O Error",                  // 405
   "Account not found",          // 406
   "System error"		 // 407
};
int  errmsgcnt = sizeof(errmsg)/sizeof(char*);

char Buf[128];

char * acc_stat[]=
{  "(EMPTY) ",
   "(BROKEN)",
   "(FROZEN)",
   "(OFF)   ",
   "(UNLIM) ",
   "(VALID) "
};

int cmd_preprocess(char * cmd, char * buf, int size)
{  char          * ptr = cmd;
   char          * str;
   unsigned long   n, m;
   unsigned long   p;

   if (cmd == NULL || buf == NULL || size < 1) return (-1);

   buf[0] = '\0';  // Empty buffer

   while(1)
   {  str = strchr(ptr, '$');
   // no marker - append rest of string & leave
      if (str == NULL)
      {  strlcat(buf, ptr, size);
         break;
      }
   // append string portion before marker (if any)
      n = strlen(buf);                                     // buffer data size
      m = size - n - 1;                                    // free buffer size (excluding space for '\0')
      p = ((unsigned long)str) - ((unsigned long)ptr);     // portion lenght
      p = p <= m ? p : m;
      if (p > 0)
      {  memmove(buf + n, ptr, p);
         buf[n + p] = '\0';         // append terminating '\0'
      }

      n = strlen(buf);   // new buffer data size
      str++;             // skip '$'
      if (strncmp(str, "LASTACC", 7) == 0)
      {  str += 7;       // skip token
         snprintf(buf + n, size - n, "%llu", SessionLastAcc);
      }
      //else if (strncmp(str, "bla-bla", 7) == 0)
      else snprintf(buf + n, size - n, "$"); // append '$' if

      ptr = str;
   }

   cmd_out(RET_COMMENT, "command: \"%s\"", buf);

   return 0;
}

/* * * * * * * * * * * * * *\
 * Execute command string  *
\* * * * * * * * * * * * * */

char     cmdbuf[1024];

int cmd_exec(char * cmd, command_t * table)
{  char * ptr = cmd;
   char * str;
   int    i;

// NULL -> core table (& preprocess command string)
   if (table == NULL)
   {  if (cmd_preprocess(cmd, cmdbuf, sizeof(cmdbuf)) < 0)
         return cmd_out(ERR_SYSTEM, "Preprocessor error");
      ptr = cmdbuf;
      table = cmds;
   }

// Get (sub-)command token
   str = next_token(&ptr, CMD_DELIM);
   if (str == NULL) str = NullCmd;

   for (i=0; table[i].ident != NULL; i++)
   {  if (strcasecmp(str, table[i].ident) == 0)
      {  if ((SessionPerm & PERM_SUPERUSER) != 0 ||
             (SessionPerm & table[i].pl) == table[i].pl )
         {  if (table[i].proc != NULL)
               return table[i].proc(str,ptr);

            if (table[i].subtab != NULL)
               return cmd_exec(ptr, table[i].subtab);  // direct recursion

            return cmd_out(ERR_SYSTEM, "Invalid table entry for %s", str);
         }
         return cmd_out(ERR_ACCESS, "No permissions for %s", str);
      }
   }

   if (str == NullCmd)
   {  if (table == cmds)
         return cmd_out(ERR_INVCMD, "Command expected ('exit' to close session)");
      else
         return cmd_out(ERR_INVCMD, "Sub-command expected");
   }
   return cmd_out(ERR_INVCMD, NULL);
}

/*

int cmd_exec(char * cmd)
{  char * ptr=cmd;
   char * str;
   int    i;

   str = next_token(&ptr, CMD_DELIM);
   if (str == NULL) return cmd_out(ERR_INVCMD, "Type command (or exit)");
   for (i=0; cmds[i].ident != NULL; i++)
   {  if (strcasecmp(str, cmds[i].ident) == 0)
         return cmds[i].proc(str,ptr);
   }
   return cmd_out(ERR_INVCMD, NULL);
}

*/

int cmd_out(int err, char * format, ...)
{  char      buf[256];
   va_list   valist;
   int       headlen;

   if (err == RET_COMMENT && HumanRead == 0) return 0;
   if ((err == RET_STR || err == RET_INT || err == RET_BIN) && MachineRead == 0) 
      return 0;

   headlen = snprintf(buf, sizeof(buf), "%03d ", err);
   if (format != NULL)
   {  va_start(valist, format);
      vsnprintf(buf + headlen, sizeof(buf) - headlen, format, valist);
      va_end(valist);
   }
   else
   {  if (err > 399 && err < (400 + errmsgcnt)) format = errmsg[err - 400];
      if (err == 0) format = "Success";
      snprintf(buf + headlen, sizeof(buf) - headlen, "%s",
           format != NULL ? format : "");
   }
   return link_puts(ld, buf);
}

/* * * * * * * * * * * * * * * * * * * * *\
 *  Prepare & Respond w/ complex string  *
\* * * * * * * * * * * * * * * * * * * * */

char    out_buf[512];
int     out_ind = 0;
int     out_err = RET_COMMENT;

// Start new responce
int cmd_out_begin(int err)
{  int       n;

   out_ind = 0;
   out_err = err;

   n = snprintf(out_buf, sizeof(out_buf), "%03d ", err);
   if (n < 0) return (-1);

   out_ind += n;

   return 0;
}

// Add responce portion
int cmd_out_add(char * format, ...)
{  int       n;
   va_list   valist;

   if (format != NULL)
   {  va_start(valist, format);
      n = vsnprintf(out_buf + out_ind, sizeof(out_buf) - out_ind, format, valist);
      out_ind += n;
      va_end(valist);
   }

   return 0;
}

// Commit responce (respond w/ resulting string)
int cmd_out_end()
{
   if (out_ind < 1)
   {  out_ind = 0;
      return (-1);
   }
   out_ind = 0;

   if (out_err == RET_COMMENT && HumanRead == 0) return 0;

   if ((out_err == RET_STR || out_err == RET_INT || out_err == RET_BIN)
        && MachineRead == 0) return 0;

   return link_puts(ld, out_buf);
}

// Rollback responce
int cmd_out_abort()
{
   out_ind = 0;
   return 0;
}

int cmdh_exit(char * cmd, char * args)
{  cmd_out(RET_COMMENT, "Have a nice day");
   return CMD_EXIT;
}

int cmdh_notimpl(char * cmd, char * args)
{   return cmd_out(ERR_NOTIMPL, NULL);   }

int cmdh_ver(char * cmd, char * args)
{
   cmd_out(RET_COMMENT, "version %d.%d.%d.%d", VER_VER, VER_SUBVER, VER_REV, VER_SUBREV);
   cmd_out(RET_INT, "%02d%02d%02d%02d", VER_VER, VER_SUBVER, VER_REV, VER_SUBREV);
   return cmd_out(RET_SUCCESS, NULL);
}

/* Resourse count transaction (machine command) */

int cmdh_res(char * cmd, char * args)
{
/*  Proto description
     0-15 - TCP/UDP port number of remote host (0-any) 
    16-23 - IP protocol id (0-any or tcp/udp if port is non-null)
    24-30   *** reserved ***
    31    - direction (inbound=0)
 */
// res <rid>	<uname>		   <val>	<proto> <host>  [<source port>]
// res  0	192.168.111.37/32  254424	0	192.168.111.37

   is_data_t  data;
   char   * str;
   char   * ptr = args;
   int      ind;
   int      accno;
//   double   sum;
   int      rc;
   int      i;


   memset(&data, 0, sizeof(data));

// get resource ident
   str = next_token(&ptr, CMD_DELIM);
   if (str == NULL) return cmd_out(ERR_ARGCOUNT, NULL);
 
   data.res_id = (-1);
   for (i=0; i < resourcecnt; i++)
   {  if (strcasecmp(resource[i].name, str) == 0)
      {  data.res_id = i;
         break;
      }
   }
   if (data.res_id == (-1)) 
      return cmd_out (ERR_INVARG, "Invalid resource name");

// get gate ident
   str = next_token(&ptr, CMD_DELIM);
   if (str == NULL) return cmd_out(ERR_ARGCOUNT, NULL);

   ind = (-1);
   if (lookup_resname(data.res_id, str, &ind) < 0)
      return cmd_out(ERR_INVARG, "Invalid gate ident");
   data.user_id = linktab[ind].user_id;
   accno = linktab[ind].accno;

// get other fixed int args (val, proto)
   for (i=2; i < 4; i++)
   {  str = next_token(&ptr, CMD_DELIM);
      if (str == NULL) return cmd_out(ERR_ARGCOUNT, NULL);
      ((unsigned int*)(&data))[i] = strtoul(str, NULL, 0);
   }

// get host IP-address
   str = next_token(&ptr, CMD_DELIM);
   if (str == NULL) return cmd_out(ERR_ARGCOUNT, NULL);
   data.host.s_addr = inet_addr(str);    

// get additional proto id (local tcp/udp port) if any
   str = next_token(&ptr, CMD_DELIM);
   if (str != NULL) data.proto2 = strtol(str, NULL, 0);    

   cmd_out(RET_COMMENT, "DATA: rid:%d, uid:%d, val:%d, proto:%d, host:%X\n",
        data.res_id, data.user_id, data.value, data.proto_id, data.host);

// Do account transaction
   rc = acc_transaction(&Accbase, &Logbase, accno, &data, (-1));
// return result (& acc state) to module
   switch (rc)
   {  case NEGATIVE:
         str = "Account is negative"; break;
      case SUCCESS:
         str = "Account is valid"; break;
      case NOT_FOUND:
         str = "Account not found"; break;
      case ACC_DELETED:
         str = "Account is deleted"; break;
      case IO_ERROR:
         return cmd_out(ERR_IOERROR, NULL);
      case ACC_BROKEN:
         str = "Account is broken"; break;
      case ACC_FROZEN:
         str = "Account is frozen"; break;
      case ACC_OFF:
         str ="Account is turned off"; break;
      default:
         str = "Unknown error";
   }      
   cmd_out(RET_COMMENT, str);
   cmd_out(RET_INT, "%d", rc);
   NeedUpdate = 1;
   return cmd_out(RET_SUCCESS, "(success for module)");
}

/* Resourse count transaction (machine command) */

int cmdh_hres(char * cmd, char * args)
{
/*  Proto description
     0-15 - TCP/UDP port number of remote host (0-any) 
    16-23 - IP protocol id (0-any or tcp/udp if port is non-null)
    24-30   *** reserved ***
    31    - direction (inbound=0)
 */
// res <rid>	<uname>		   <val>	<proto> <host>  [<source port>]
// res  0	192.168.111.37/32  254424	0	192.168.111.37

   is_data_t  data;
   char     * str;
   char     * ptr   = args;
   int        ind;
   int        accno;
//   double   sum;
   int        rc;
   int        i;
   int        price = (-1);

   memset(&data, 0, sizeof(data));

// get resource ident
   str = next_token(&ptr, CMD_DELIM);
   if (str == NULL) return cmd_out(ERR_ARGCOUNT, NULL);
 
   data.res_id = (-1);
   for (i=0; i < resourcecnt; i++)
   {  if (strcasecmp(resource[i].name, str) == 0)
      {  data.res_id = i;
         break;
      }
   }
   if (data.res_id == (-1)) 
      return cmd_out (ERR_INVARG, "Invalid resource name");

// get gate ident
   str = next_token(&ptr, CMD_DELIM);
   if (str == NULL) return cmd_out(ERR_ARGCOUNT, NULL);

   ind = -1;
   if (lookup_resname(data.res_id, str, &ind)<0)
      return cmd_out(ERR_INVARG, "Invalid gate ident");
   data.user_id = linktab[ind].user_id;
   accno        = linktab[ind].accno;

// get other fixed int args (val, proto)
   for (i=2; i < 4; i++)
   {  str = next_token(&ptr, CMD_DELIM);
      if (str == NULL) return cmd_out(ERR_ARGCOUNT, NULL);
      ((unsigned int*)(&data))[i] = strtoul(str, NULL, 0);
   }

// get host IP-address
   str = next_token(&ptr, CMD_DELIM);
   if (str == NULL) return cmd_out(ERR_ARGCOUNT, NULL);
   data.host.s_addr = inet_addr(str);    

// hack: get custom inet price
   str = next_token(&ptr, CMD_DELIM);
   if (str != NULL) price = strtol(str, NULL, 0);    

   cmd_out(RET_COMMENT, "DATA: rid:%d, uid:%d, val:%d, proto:%d, host:%X\n",
        data.res_id, data.user_id, data.value, data.proto_id, data.host);

// Do account transaction
   rc = acc_transaction(&Accbase, &Logbase, accno, &data, price);

// return result (& acc state) to module
   switch (rc)
   {  case NEGATIVE:
         str = "Account is negative"; break;
      case SUCCESS:
         str = "Account is valid"; break;
      case NOT_FOUND:
         str = "Account not found"; break;
      case ACC_DELETED:
         str = "Account is deleted"; break;
      case IO_ERROR:
         return cmd_out(ERR_IOERROR, NULL);
      case ACC_BROKEN:
         str = "Account is broken"; break;
      case ACC_FROZEN:
         str = "Account is frozen"; break;
      case ACC_OFF:
         str = "Account is turned off"; break;
      default:
         str = "Unknown error";
   }      
   cmd_out(RET_COMMENT, str);
   cmd_out(RET_INT, "%d", rc);

   NeedUpdate = 1;

   return cmd_out(RET_SUCCESS, "(success for module)");
}

int cmd_getaccno(char ** args, lookup_t * prev)
{  
   char * ptr;
   char * str;
   int    ind = (-1);
   int    rid;
   int    uid;
   int    i;
   int    flag  = 1;
   int    flag2 = 1;
   int    rc;

   if (args != NULL) ptr=*args;

   if (prev != NULL && args == NULL)
   {  switch(prev->what)
      {  case LF_ALL:
            prev->ind++;
            if (prev->ind >= prev->no) return (-1);
	    return prev->ind;
         case LF_NAME:
            rc = lookup_name(prev->str, &prev->ind);
            break;
         case LF_PNAME:
            rc = lookup_pname(prev->str, &prev->ind);
            break;
         case LF_ADDR:
            rc = lookup_addr(prev->str, &prev->ind);
            break;
         default: 
            return (-1);
      }
      if (rc<0) return (-1);
      cmd_out(RET_COMMENT, "%s-%d\t#%04d\t%s", 
              resource[linktab[rc].res_id].name,
              linktab[rc].user_id, linktab[rc].accno,
              linktab[rc].username);
      return linktab[rc].accno;
   }

   if (prev != NULL) prev->what = LF_NONE;

   str = next_token(&ptr, CMD_DELIM);
   if (str == NULL) return (-1);
   if ((flag = strcasecmp(str, "name")) == 0   || 
       (flag2 = strcasecmp(str, "pname")) == 0 || 
       strcasecmp(str, "addr") == 0)
   {  str = next_token(&ptr, CMD_DELIM);
      if (str == NULL) return (-1);
      ind = -1;
      if (flag == 0) rc = lookup_name(str, &ind);
      else
      {  if (flag2 == 0) rc = lookup_pname(str, &ind);
         else rc = lookup_addr(str, &ind);
      } 
      if (rc == (-1))
      {  cmd_out(RET_COMMENT, "Can't lookup name");
         return (-1);
      }
      if (prev != NULL)
      {  prev->what = flag ? (flag2 ? LF_ADDR : LF_PNAME) : LF_NAME;  // zero = true !
         prev->ind  = ind;
         prev->str  = str;
      }
      *args = ptr;
      cmd_out(RET_COMMENT, "%s-%d\t#%04d\t%s", 
              resource[linktab[ind].res_id].name,
              linktab[ind].user_id, linktab[ind].accno,
              linktab[ind].username);
      return linktab[ind].accno;
   }
   for (i=0; i < resourcecnt; i++)
     if (strcasecmp(str, resource[i].name) == 0) break;
   if (i < resourcecnt)
   {  rid = i;
      str = next_token(&ptr, CMD_DELIM);
      if (str == NULL) return (-1);
      if (strcasecmp(str, "name") != 0)
      {  uid = strtol(str, NULL, 0);
         rc = lookup_res(rid, uid, &ind);
      }
      else 
      {  str = next_token(&ptr, CMD_DELIM);
         if (str == NULL) return (-1);
         rc = lookup_resname(rid, str, &ind);
      }
      if (rc == (-1))
      {  cmd_out(RET_COMMENT, "Can't lookup resource");
         return (-1);
      }
      else
      {  *args = ptr;
         cmd_out(RET_COMMENT, "%s-%d\t#%04d\t%s", 
                 resource[linktab[ind].res_id].name,
                 linktab[ind].user_id, linktab[ind].accno,
                 linktab[ind].username);
         return linktab[ind].accno;
      }
   }
   if (strcmp(str, "*") == 0)
   {  *args = ptr;
      if (prev != NULL)
      {  prev->what = LF_ALL;
         prev->ind  = 0;
         prev->no   = acc_reccount(&Accbase);
      }
      return 0; 
   }
   *args = ptr;                   // always
   rc = strtol(str, &ptr, 0);     // ptr is down
   if (ptr != str) return rc;
   cmd_out(RET_COMMENT, "Invalid account");
   return (-1);
}

int cmdh_acc(char * cmd, char * args)
{ 
   char * ptr=args;
//   char * str;
   int    accno;
   int    recs;
   acc_t  acc;
   int    rc;
   int    i;
   int    b,bit;
   char * org    = "PUOFBD";
   char   mask[7];
   char   startbuf[16];
   char   stopbuf[16];
   char   resbuf[16];

   accno = cmd_getaccno(&ptr, NULL);
   if (accno != (-1))
   {  rc = acc_get(&Accbase, accno, &acc);

      if (rc == IO_ERROR) return cmd_out(ERR_IOERROR, NULL);
      if (rc == NOT_FOUND) return cmd_out(ERR_NOACC, NULL);
      bit = 5;
      strlcpy(mask, org, sizeof(mask));
      for (b = 0; b<6; b++)
      {  if ((acc.tag & 1<<b))
         {  if (bit == 5) bit=b;
         }
         else mask[5-b] = '-';
      }
      strcpy(startbuf, "-");
      strcpy(stopbuf, "-");
      strcpy(resbuf, "");

      if (acc.start != 0) cmd_pdate(acc.start, startbuf);
      if (acc.stop != 0)  cmd_pdate(acc.stop,  stopbuf);

      if (acc.tariff != 0) snprintf(resbuf, sizeof(resbuf), "%d", acc.tariff);

      cmd_out(RET_COMMENT, "#%04d  %s %s  %+10.2f  start %-10s stop %-10s%s%s",
          accno, mask, acc_stat[bit], acc.balance, startbuf, stopbuf,
          acc.tariff ? " tariff ":"", resbuf);

      cmd_out(RET_STR, "%d %d %e %d %d %d %g %lld %lld %g %d", accno, acc.tag, acc.balance,
               acc.start, acc.stop, acc.tariff, acc_limit(&acc), acc.inet_summ_in, acc.inet_summ_out, acc.money_summ, acc.summ_rsttime);

      return cmd_out(RET_SUCCESS, "");
   }
   else
   {
      if (next_token(&ptr, CMD_DELIM) != NULL)
         return cmd_out(ERR_NOACC, NULL);

      recs = acc_reccount(&Accbase);
      if (recs < 0) return cmd_out(ERR_IOERROR, NULL);

      for (i=0; i < recs; i++)
      {  rc = acc_get(&Accbase, i, &acc);
         if (rc == IO_ERROR) return cmd_out(ERR_IOERROR, NULL);

         if (acc.tag & ATAG_DELETED) continue;

         bit = 5;
         strlcpy(mask, org, sizeof(mask));
         for (b=0; b < 6; b++)
         {  if ((acc.tag & (1 << b)) != 0)
            {  if (bit == 5) bit = b;
            }
            else mask[5 - b] = '-';
         }
         strcpy(startbuf, "-"); 
         strcpy(stopbuf, "-"); 
         strcpy(resbuf, "");

         if (acc.start != 0) cmd_pdate(acc.start, startbuf);
         if (acc.stop != 0)  cmd_pdate(acc.stop, stopbuf);

         if (acc.tariff != 0) snprintf(resbuf, sizeof(resbuf), "%d", acc.tariff);
         cmd_out(RET_COMMENT, "#%04d  %s %s  %+10.2f  start %-10s stop %-10s%s%s",
            i, mask, acc_stat[bit], acc.balance, startbuf, stopbuf,
            acc.tariff ? " tariff ":"", resbuf);
      }
      return cmd_out(RET_SUCCESS, NULL);
   }
}

int cmdh_freeze(char * cmd, char * args)
{ 
   char * ptr = args;
   int    accno;
   acc_t  acc;
   int    rc;

   NeedUpdate = 1;
   accno = cmd_getaccno(&ptr, NULL);
   if (accno != (-1))
   {  rc = acc_baselock(&Accbase);
      if (rc != SUCCESS) return cmd_out(ERR_IOERROR, NULL);
      rc = acci_get(&Accbase, accno, &acc);
      if (rc == IO_ERROR || rc == NOT_FOUND || rc == ACC_BROKEN ||
          rc == ACC_DELETED) acc_baseunlock(&Accbase);

      if (rc == IO_ERROR)  return cmd_out(ERR_IOERROR, NULL);
      if (rc == NOT_FOUND) return cmd_out(ERR_NOACC, NULL);
      if (acc.tag == ATAG_BROKEN)
         return cmd_out(ERR_ACCESS, "Account is broken");
      if (acc.tag == ATAG_DELETED) return 
                             cmd_out(ERR_ACCESS, "Account is empty");
      if (strcasecmp(cmd, "freeze") == 0)
      {  acc.tag |= ATAG_FROZEN;

         rc = acci_put(&Accbase, accno, &acc);
         acc_baseunlock(&Accbase);
         if (rc <= 0) return cmd_out(RET_SUCCESS, NULL);

         return cmd_out(ERR_IOERROR, NULL);
      }      

      if (strcasecmp(cmd, "unfreeze") == 0)
      {  if ((acc.tag & ATAG_FROZEN) == 0) 
         {  acc_baseunlock(&Accbase);
            return cmd_out(ERR_ACCESS, "Not frozen");
         }
         acc.tag &= ~ATAG_FROZEN;

         rc = acci_put(&Accbase, accno, &acc);
         acc_baseunlock(&Accbase);
         if (rc <= 0) return cmd_out(RET_SUCCESS, NULL);

         return cmd_out(ERR_IOERROR, NULL);
      }

      if (strcasecmp(cmd, "on") == 0)
      {  if ((acc.tag & ATAG_OFF) == 0) 
         {  acc_baseunlock(&Accbase);
            return cmd_out(ERR_ACCESS, "Not OFF");
         }
         acc.tag &= ~ATAG_OFF;

         rc = acci_put(&Accbase, accno, &acc);
         acc_baseunlock(&Accbase);
         if (rc <= 0) return cmd_out(RET_SUCCESS, NULL);

         return cmd_out(ERR_IOERROR, NULL);
      }

      if (strcasecmp(cmd, "off") == 0)
      {  acc.tag |= ATAG_OFF;

         rc = acci_put(&Accbase, accno, &acc);
         acc_baseunlock(&Accbase);
         if (rc <= 0) return cmd_out(RET_SUCCESS, NULL);
 
        return cmd_out(ERR_IOERROR, NULL);
      }

      if (strcasecmp(cmd, "_break") == 0)
      {  acc.tag |= ATAG_BROKEN;

         rc = acci_put(&Accbase, accno, &acc);
         acc_baseunlock(&Accbase);
         if (rc <= 0) return cmd_out(RET_SUCCESS, "Account got broken");

         return cmd_out(ERR_IOERROR, NULL);
      }

      if (strcasecmp(cmd, "unlimit") == 0)
      {  acc.tag |= ATAG_UNLIMIT;

         rc = acci_put(&Accbase, accno, &acc);
         acc_baseunlock(&Accbase);
         if (rc <= 0) return cmd_out(RET_SUCCESS, "Account set unlimit");

         return cmd_out(ERR_IOERROR, NULL);
      }

      if (strcasecmp(cmd, "limit") == 0)
      {  acc.tag &= ~ATAG_UNLIMIT;

         rc = acci_put(&Accbase, accno, &acc);
         acc_baseunlock(&Accbase);
         if (rc <= 0) return cmd_out(RET_SUCCESS, "Account limited");

         return cmd_out(ERR_IOERROR, NULL);
      }

      if (strcasecmp(cmd, "payman") == 0)
      {  acc.tag |= ATAG_PAYMAN;

         rc = acci_put(&Accbase, accno, &acc);
         acc_baseunlock(&Accbase);
         if (rc <= 0) return cmd_out(RET_SUCCESS, "Account allowed from manager");

         return cmd_out(ERR_IOERROR, NULL);
      }

      if (strcasecmp(cmd, "nopayman") == 0)
      {  acc.tag &= ~ATAG_PAYMAN;

         rc = acci_put(&Accbase, accno, &acc);
         acc_baseunlock(&Accbase);
         if (rc <= 0) return cmd_out(RET_SUCCESS, "Account denied from manager");

         return cmd_out(ERR_IOERROR, NULL);
      }

      if (strcasecmp(cmd, "_rstsumm") == 0)
      {  acc.inet_summ_in  = 0;
         acc.inet_summ_out = 0;
         acc.money_summ    = 0;
         acc.summ_rsttime  = 0;

         rc = acci_put(&Accbase, accno, &acc);
         acc_baseunlock(&Accbase);
         if (rc <= 0) return cmd_out(RET_SUCCESS, "Summary info flushed");

         return cmd_out(ERR_IOERROR, NULL);
      }
      return cmd_out(ERR_INVCMD, NULL);


   }
   return cmd_out(ERR_INVARG, NULL);
}

int cmdh_fix(char * cmd, char * args)
{ 
   char * str;
   char * ptr    = args;
   int    accno;
   acc_t  acc;
   int    rc;

   NeedUpdate = 1;
   accno = cmd_getaccno(&ptr, NULL);
   if (accno != (-1))
   {  rc = acc_baselock(&Accbase);
      if (rc != SUCCESS) return cmd_out(ERR_IOERROR, NULL);
      rc = acci_get(&Accbase, accno, &acc);
      if (rc == IO_ERROR || rc == NOT_FOUND) acc_baseunlock(&Accbase);
      if (rc == IO_ERROR) return cmd_out(ERR_IOERROR, NULL);
      if (rc == NOT_FOUND) return cmd_out(ERR_NOACC, NULL);
      acc.accno = accno;
      acc.tag &= ~(ATAG_BROKEN|ATAG_DELETED);
      str = next_token(&ptr, CMD_DELIM);
      if (str != NULL) acc.balance = ((double)strtod(str, NULL));
      rc = acci_put(&Accbase, accno, &acc);
      acc_baseunlock(&Accbase);
      if (rc <= 0) return cmd_out(RET_SUCCESS, NULL);
      return cmd_out(ERR_IOERROR, NULL);
   }
   return cmd_out(ERR_INVARG, NULL);
}

int cmdh_new(char * cmd, char * args)
{  int      rc;
   acc_t    acc;
   char   * ptr  = args;
   char   * str;
   double   sum;

   memset(&acc, 0, sizeof(acc));
   acc.balance = 0;
   rc = acc_add(&Accbase, &acc);
   if (rc < 0) return cmd_out(ERR_IOERROR, NULL);

   cmd_out(RET_COMMENT, "Account %d created", rc);

   str = next_token(&ptr, CMD_DELIM);   // ??? :)
   if (str != NULL)
   {  sum = strtod(str, NULL);
   }

   NeedUpdate = 1;
   return cmd_out(RET_SUCCESS, Buf);
}

int cmdh_add(char * cmd, char * args)
{  char *    ptr    = args;
   char *    str;
   int       accno;
   double    sum;
   int       rc; 

   accno = cmd_getaccno(&ptr, NULL);
   if (accno >= 0)
   {  str = next_token(&ptr, CMD_DELIM);
      if (str != NULL)
      {  sum = strtod(str, NULL);
         rc  = cmd_add(accno, sum);
         if (rc >= 0) cmd_out(RET_COMMENT, "Transaction successful");
         else 
         {  cmd_accerr(rc);
            return cmd_out(ERR_ACCESS, "Unable to access account");
         }
      }
      else return cmd_out(ERR_ARGCOUNT, NULL);
   }
   else return cmd_out(ERR_INVARG, NULL);

   NeedUpdate = 1;

   return cmd_out(RET_SUCCESS, NULL);
}

int cmd_accerr(int rc)
{  char * str;

   switch (rc)
   {  case NEGATIVE:
         str = "Account is negative"; break;
      case SUCCESS:
         str = "Account is valid"; break;
      case NOT_FOUND:
         str = "Account not found"; break;
      case ACC_DELETED:
         str = "Account is deleted"; break;
      case IO_ERROR:
         return cmd_out(ERR_IOERROR, NULL);
      case ACC_BROKEN:
         str = "Account is broken"; break;
      case ACC_FROZEN:
         str = "Account is frozen"; break;
      case ACC_OFF:
         str = "Account is turned off"; break;
      default:
         str = "Unknown error";
   }      
   return cmd_out(RET_COMMENT, "%s", str);
}

int cmd_add(int accno, double sum)
{  is_data_t data;
   struct sockaddr_in  addr;
   socklen_t           len  = sizeof(addr);

   memset(&data, 0, sizeof(data));
   data.res_id  = RES_ADDER;
   data.user_id = accno;
   if (getpeername(ld->fd, (struct sockaddr*)&addr, &len) == 0)
      data.host = addr.sin_addr;
   
   return acc_trans(&Accbase, accno, sum, &data, &Logbase);
}

int cmdh_log(char * cmd, char * args)
{
// errno time accno sum balance rid uid val proto host proto2  
// S  05-05 20:50 #0000 (  +100.00)   -3001.32   inet(000) 
//   tcp/udp    80  local    0 from 192.168.111.37   

   int       rc;
   logrec_t  logrec;
   int       i;
   int       recs;
   char   *  result;
   char      tbuf[20];
   char      pbuf[64];
   char      bbuf[16];
   int       proto;
   int       rport;
   int       lport;
   int       accno    = (-1);
   char    * ptr      = args;
   char    * str;
   int       line     = (-1);
   int       part     = 0;
   int       all      = 0;

   accno = cmd_getaccno(&ptr, NULL);
   str   = next_token(&ptr, CMD_DELIM);
   if (str != NULL)
   {  if (strcasecmp(str, "all") == 0) all=1;
      else part = strtol(str, NULL, 0);
   }
   recs = log_reccount(&Logbase);
   if (recs < 0) return cmd_out(ERR_IOERROR, NULL);

   for (i = recs - 1; i >= 0; i--)
   {  
      rc = log_get(&Logbase, i, &logrec);

      if (rc == IO_ERROR) return cmd_out(ERR_IOERROR, NULL);

      if (accno >= 0 && accno != logrec.accno) continue;
      if (all == 0)
      {  line += 1;
         if (line/20 > part) break;
         if (line/20 != part) continue;
      }

      switch(logrec.serrno)
      {  case NEGATIVE:
           result = "S-";
           break;
         case SUCCESS:
           result = "S+";
           break;
         case NOT_FOUND:
           result = "NF";
           break;
         case ACC_DELETED:
           result = "EM";
           break;
         case IO_ERROR:
           result = "IO";
           break;
         case ACC_BROKEN:
           result = "BR";
           break;
         case ACC_FROZEN:
           result = "FR";
           break;
         case ACC_OFF:
           result = "OF";
           break;
         case ACC_UNLIMIT:
           result = "UN";
           break;
         default:
           result = "??";
      }
      cmd_ptime(logrec.time, tbuf);
// out t/u 00000 (00000)
      proto = (logrec.isdata.proto_id & 0x00ff0000) >> 16;
      rport = logrec.isdata.proto_id & 0xffff;
      lport = logrec.isdata.proto2   & 0xffff;
      switch(proto)
      {  case 1:
            snprintf(pbuf, sizeof(pbuf), "icmp            ");
            break;
         case 6:
            snprintf(pbuf, sizeof(pbuf), "tcp ");
         case 17:
            if (proto == 17) snprintf(pbuf, sizeof(pbuf), "udp ");
            break;
         case 0:
            if ((logrec.isdata.proto_id & PROTO_CHARGE) == 0)
            {  if (proto == 0)
               {  if (rport == 0 && lport == 0)
                  {  snprintf(pbuf, sizeof(pbuf), "any             ");
                     break;
                  }
                  else snprintf(pbuf, sizeof(pbuf), "t/u ");
               }
               if (rport != 0) snprintf(pbuf+4, sizeof(pbuf)-4, "%5d ", rport);
               else snprintf(pbuf + 4, sizeof(pbuf) - 4, "any   ");
               if (lport != 0) snprintf(pbuf + 10, sizeof(pbuf) - 10, "%5d ", lport);
               else snprintf(pbuf + 10, sizeof(pbuf) - 10, "      ");
            }
            else snprintf(pbuf, sizeof(pbuf), "charge          ");
            break;
         default:
            snprintf(pbuf, sizeof(pbuf), "ip %3d          ", proto);
      }
      if (logrec.balance == BALANCE_NA) 
         snprintf(bbuf, sizeof(bbuf), "    N/A   ");
      else
         snprintf(bbuf, sizeof(bbuf), "%+10.2f", logrec.balance);

      cmd_out(RET_COMMENT, "%s %s #%-4d %+8.2f (was %s) %6s %s %10d %s",
                result, 
                tbuf,
                logrec.accno, 
                logrec.sum,
                bbuf,
                resource[logrec.isdata.res_id].name,
                ((logrec.isdata.proto_id & PROTO_CHARGE) == 0) ? (((logrec.isdata.proto_id & 0x80000000) == 0) ?  " in":"out") : "   ",
                logrec.isdata.value,
                pbuf);
//      rc = cmd_out(RET_COMMENT,"       (host %s)",
//                inet_ntoa(logrec.isdata.host));
      if (rc < 0) break;
   }
   return cmd_out(RET_SUCCESS, NULL);
}

int cmd_ptime(time_t utc, char * buf)
{  struct tm stm;
   
   localtime_r(&utc, &stm);

// 00-00-00 00:00.00
// 01234567890123456

   snprintf(buf, 12, "%02d-%02d %02d:%02d",
           stm.tm_mday,
           stm.tm_mon+1,
           stm.tm_hour,
           stm.tm_min);

   return 0;
}


int cmdh_update (char * cmd, char * args)
{  int rc;

   cmd_out(RET_COMMENT, "Updating access restrictions ...");
   access_update();
   rc = cmd_out(RET_SUCCESS, "Done.");
   if (rc < 0) return rc; 
   return 0;
}

int cmdh_human (char * cmd, char * args)
{  int rc;

   if (strcasecmp(cmd, "human") == 0)  
   {  HumanRead   = 1;
      MachineRead = 0;
   }
   else 
   {  MachineRead = 1;
      HumanRead   = 0;
   }
   rc = cmd_out(RET_SUCCESS, "Mode set");
   if (rc < 0) return rc; 
   return 0;
}

// lookup acc       by resource
// lookup accs      by name/addr
// lookup resources by acc

int cmdh_lookup(char * cmd, char * args)
{  char *   ptr   = args;
//   char *   str;
   int      accno;
   lookup_t prev;
   int      ind;

   accno = cmd_getaccno(&ptr, &prev);
   if (accno == (-1)) return cmd_out(ERR_ARGCOUNT, NULL);
   do
   {  ind = (-1);
      while(lookup_accno(accno, &ind) >= 0)
      {  cmd_out(RET_TEXT, "%d\t%s\t%s%s", accno,
            resource[linktab[ind].res_id].name,
            linktab[ind].username, linktab[ind].allow ? "":"\t(disabled)");
      }
      accno = cmd_getaccno(NULL, &prev);
   } while (accno >= 0);   
   return cmd_out(RET_SUCCESS, "");
}

int cmdh_date(char * cmd, char * args)
{  char   buf[16];
   time_t tim     = time(NULL);
   
   cmd_out(RET_INT, "%d", tim);
   cmd_ptime(tim, buf);
   return cmd_out(RET_SUCCESS, "%s", buf);
}

int cmdh_report(char * cmd, char * args)
{
   logrec_t * rep     = NULL;        // report array
   int        repc    = 0;           // report records count
   time_t     tfrom   = 0;           // time from (default - origin)
   time_t     tto     = time(NULL);  // time to (default - current)
   int        accno   = -1;	     // account number
   int        tstep   = 0x7fffffff;  // default - summary
   int        logrecs;		     // no of log records
   char     * ptr     = args;
   char     * str;
   int        i,j;
   int        wait4   = 0;
/*
   0 - token
   1 - step value
*/		
   logrec_t   logrec;
   void     * tmp;
   int        rc;   
   int        flag;

//report <acc> [from <time>] [to <time>] [step <hours>] [<flags>]

   accno = cmd_getaccno(&ptr, NULL);
   if (accno == -1) return cmd_out(ERR_ARGCOUNT, NULL);
   
   while((str = next_token(&ptr, CMD_DELIM)) != NULL)
   {  switch(wait4)
      {  case 0:
            if (strcasecmp(str, "from") == 0)
            {  tfrom = cmd_gettime(&ptr, 0);
               if (tfrom == -1) return cmd_out(ERR_INVARG, NULL);   
               wait4 = 0;
               break;
            }
            if (strcasecmp(str, "to") == 0)
            {  tto = cmd_gettime(&ptr, 0);
               if (tto == -1) return cmd_out(ERR_INVARG, NULL);   
               wait4 = 0;
               break;
            }
            if (strcasecmp(str, "step") == 0)
            {  wait4 = 1;
               break;
            }
            break;
         case 1:
            tstep = strtol(str, NULL, 0) * 3600;
            wait4 = 0;
            break;
         default:
            wait4 = 0;
            break;
      }
   }
   logrecs = log_reccount(&Logbase);   


   for (i=0; i < logrecs; i++)
   {  rc = log_get(&Logbase, i, &logrec);
      if (rc < 0)
      {  if (rep != NULL) free(rep);
         return cmd_out(ERR_IOERROR, NULL);
      }
      if (logrec.accno != accno) continue;
      if (logrec.time < tfrom || logrec.time > tto) continue;
      flag = 1;
      for (j = repc - 1; j >= 0; j--)
      {  if (logrec.time - rep[j].time > tstep) break;
         if (rep[j].accno             == logrec.accno &&
             rep[j].serrno            == logrec.serrno &&
             rep[j].isdata.res_id     == logrec.isdata.res_id &&
             rep[j].isdata.user_id    == logrec.isdata.user_id &&
             rep[j].isdata.proto_id   == logrec.isdata.proto_id &&
             rep[j].isdata.proto2     == logrec.isdata.proto2 &&
             rep[j].isdata.host.s_addr== logrec.isdata.host.s_addr)
         {  
            rep[j].isdata.value += logrec.isdata.value;
            rep[j].sum          += logrec.sum;
            rep[j].balance       = BALANCE_NA;
            flag                 = 0;
            break; 
         }
      }
      if (flag)
      {  tmp = realloc(rep, sizeof(logrec_t)*(repc + 1));
         if (tmp == NULL)
         {  syslog(LOG_ERR, "report(realloc()): %m");
            if (rep != NULL) free(rep);
            return cmd_out(ERR_SYSTEM, NULL);
         }
         rep         = (logrec_t*)tmp;
         rep[repc++] = logrec;
      }
   }
   for (i=0; i < repc; i++)
   {  if ((rc = cmd_plogrec(rep+i)) < 0) return rc; 
   }
   if (rep != NULL) free(rep);
   return cmd_out(RET_SUCCESS, NULL);
}


time_t cmd_gettime(char ** arg, time_t def)
{  struct tm stm;
   time_t    ctimet = def ? def : time(NULL);
   char *    ptr    = *arg;
   char *    ptr2;
   char *    str;
   int       sign;
   time_t    disp;
   int       val1;

// {{+}|{-} <H>:<M>}|{{[<D>/<M>[/<YY>]]} {<H>:<M>}} 
   str = next_token(&ptr, " \t");
   if (str == NULL) return -1;
   if (strcmp(str, "+") == 0 || strcmp(str, "-") == 0)
   {  sign = *str == '+' ? 1 : -1;
      str = next_token(&ptr, " \t");
      if (str == NULL) return (-1);
      ptr2 = str;
      str = next_token(&ptr2, ":");
      if (str == NULL) return -1;
      disp = strtol(str, NULL, 0) * 3600;
      str  = next_token(&ptr2, ":");
      if (str == NULL) return (-1);
      disp += strtol(str, NULL, 0) * 60;
      disp *= sign;
      *arg  = ptr;
      return ctimet + disp;
   }      
   localtime_r(&ctimet, &stm);
   val1 = strtol(str, &ptr2, 0);
   str  = ptr2+1;
   if (*ptr2 == ':')
   {  stm.tm_min  = strtol(str, NULL, 0);
      stm.tm_sec  = 0;
      stm.tm_hour = val1;
      *arg        =ptr;
      return mktime(&stm);
   }
   stm.tm_mon  = strtol(str, &ptr2, 0) - 1;
   stm.tm_mday = val1;
   if (*ptr2 != '\0')
   {  val1 = strtol(ptr2 + 1, NULL, 0);
      if (val1 > 1900) val1 -= 1900;
      stm.tm_year = val1;
   }
   str = next_token(&ptr, " \t");
   if (str == NULL) return (-1);
   stm.tm_hour = strtol(str, &ptr2, 0);
   stm.tm_min  = strtol(ptr2+1, NULL, 0);
   stm.tm_sec  = 0;
   *arg        = ptr;
   return mktime(&stm);
}


int cmd_plogrec(logrec_t * logrec)
{
   char    * result;
   char      tbuf[20];
   char      pbuf[64];
   char      bbuf[16];
   int       rport, lport, proto;
   int       rc;

   switch(logrec->serrno)
   {  case NEGATIVE:
         result = "S-";
         break;
      case SUCCESS:
         result = "S+";
         break;
      case NOT_FOUND:
         result = "NF";
         break;
      case ACC_DELETED:
         result = "EM";
         break;
      case IO_ERROR:
         result = "IO";
         break;
      case ACC_BROKEN:
         result = "BR";
         break;
      case ACC_FROZEN:
         result = "FR";
         break;
      case ACC_OFF:
         result = "OF";
         break;
      case ACC_UNLIMIT:
         result = "UN";
         break;
      default:
         result = "??";
   }
   cmd_ptime(logrec->time, tbuf);
   proto = (logrec->isdata.proto_id & 0x00ff0000) >> 16;
   rport = logrec->isdata.proto_id & 0xffff;
   lport = logrec->isdata.proto2   & 0xffff;
   switch(proto)
   {  case 1:
         snprintf(pbuf, sizeof(pbuf), "icmp            ");
         break;
      case 6:
         snprintf(pbuf, sizeof(pbuf), "tcp ");
      case 17:
         if (proto == 17) snprintf(pbuf, sizeof(pbuf), "udp ");
         break;
      case 0:
         if (proto == 0)
         {  if (rport == 0 && lport == 0)
            {  snprintf(pbuf, sizeof(pbuf), "any             ");
               break;
            }
            else snprintf(pbuf, sizeof(pbuf), "t/u ");
         }
         if (rport != 0) snprintf(pbuf+4, sizeof(pbuf)-4, "%5d ", rport);
         else snprintf(pbuf+4, sizeof(pbuf)-4, "any   ");
         if (lport != 0) snprintf(pbuf+10, sizeof(pbuf)-10, "%5d ", lport);
         else snprintf(pbuf+10, sizeof(pbuf)-10, "      ");
         break;
      default:
         snprintf(pbuf, sizeof(pbuf), "ip %3d          ", proto);
   }
   if (logrec->balance == BALANCE_NA) 
      snprintf(bbuf, sizeof(bbuf), "    N/A   ");
   else
      snprintf(bbuf, sizeof(bbuf), "%+10.2f", logrec->balance);
   rc = cmd_out(RET_COMMENT, "%s %s #%-4d %+8.2f %6s %s %10d %s",
                result, 
                tbuf,
                logrec->accno, 
                logrec->sum,
                resource[logrec->isdata.res_id].name,
                ((logrec->isdata.proto_id & 0x80000000) == 0) ?  " in":"out",
                logrec->isdata.value,
                pbuf);
   return rc;
}

int cmdh_delete(char * cmd, char * args)
{
   char * ptr    = args;
   int    accno;
   acc_t  acc;
   int    rc;

   accno = cmd_getaccno(&ptr, NULL);
   if (accno != (-1))
   {  rc = acc_baselock(&Accbase);
      if (rc != SUCCESS) return cmd_out(ERR_IOERROR, NULL);
      rc = acci_get(&Accbase, accno, &acc);
      if (rc == IO_ERROR || rc == NOT_FOUND) acc_baseunlock(&Accbase);
      if (rc == IO_ERROR)  return cmd_out(ERR_IOERROR, NULL);
      if (rc == NOT_FOUND) return cmd_out(ERR_NOACC, NULL);
      acc.tag |= ATAG_DELETED;
      rc = acci_put(&Accbase, accno, &acc);
      acc_baseunlock(&Accbase);
      if (rc <= 0) return cmd_out(RET_SUCCESS, NULL);
      return cmd_out(ERR_IOERROR, NULL);
   }
   return cmd_out(ERR_INVARG, NULL);
}

int cmdh_new_contract(char * cmd, char * args)
{  char   * ptr  = args;
   char   * str;  
   int      rc;
   acc_t    acc;
   int      acc_inet, acc_intra;
   char   * name;
   int      len;
   int      lockfd;

   str = next_token(&ptr, CMD_DELIM);
   if (str == NULL) return cmd_out(ERR_ARGCOUNT, "Initial username expected");
   name = str;
// Check name (len <= 8, letters, digits)
   len = strlen(name);
   if (len < 2 || len > 8) return cmd_out(ERR_INVARG, "Invalid name lenght");

   memset(&acc, 0, sizeof(acc));
   acc.balance = 0;
   rc = acc_add(&Accbase, &acc);
   if (rc < 0) return cmd_out(ERR_IOERROR, NULL);
   acc_inet = rc;  
   rc = acc_add(&Accbase, &acc);
   if (rc < 0) return cmd_out(ERR_IOERROR, NULL);
   acc_intra = rc;
   cmd_out(RET_COMMENT, "Inet account: %d", acc_inet);
   cmd_out(RET_INT, "%d", acc_inet);
   cmd_out(RET_COMMENT, "Intranet account: %d", acc_intra);
   cmd_out(RET_INT, "%d", acc_intra);

   if ((lockfd = reslinks_lock(LOCK_EX)) != (-1))
   {  reslinks_load(LOCK_UN);
      reslink_new(RES_ADDER, acc_inet, name);
      reslink_new(RES_ADDER, acc_intra, name);      
      reslinks_save(LOCK_UN);
      reslinks_unlock(lockfd);
   }
   else syslog(LOG_ERR, "cmdh_new_name(): Unable to lock reslinks");

   return cmd_out(RET_SUCCESS, NULL);
}

int cmdh_new_name(char * cmd, char * args)
{  char * str;
   char * ptr  = args;
   int    acc_inet, acc_intra;
   char * name;
   char * host;
   int    accs;
   acc_t  test;
   int    rc;
   int    len;
   unsigned char c;
   FILE * fd;
   int    i;
   char   buf[128];
   int    lockfd;

// new_name <name>@host <#inet> <#intra>

// Get name
   str = next_token(&ptr, CMD_DELIM);
   if (str == NULL) return cmd_out(ERR_ARGCOUNT, "Arguments needed");
   name = str;
// Get account numbers
   str = next_token(&ptr, CMD_DELIM);
   if (str == NULL) return cmd_out(ERR_ARGCOUNT, "Account number needed");
   acc_inet = strtol(str, NULL, 0);
   str = next_token(&ptr, CMD_DELIM);
   if (str == NULL) return cmd_out(ERR_ARGCOUNT, "Intranet account needed");
   acc_intra = strtol(str, NULL, 0);
// Check account exists
   accs = acc_reccount(&Accbase);
   if (accs < 0) return cmd_out(ERR_IOERROR, NULL);
   if (acc_inet < 0 || acc_intra < 0 || acc_inet >= accs || acc_intra >= accs)
      return cmd_out(ERR_INVARG, "Invalid account given");
   rc = acc_get(&Accbase, acc_inet, &test);
   if (rc == IO_ERROR) return cmd_out(ERR_IOERROR, NULL);
   if (rc == ACC_DELETED || rc == NOT_FOUND) 
      return cmd_out(ERR_INVARG, "Given account (inet) is deleted"); 
   rc = acc_get(&Accbase, acc_intra, &test);
   if (rc == IO_ERROR) return cmd_out(ERR_IOERROR, NULL);
   if (rc == ACC_DELETED || rc == NOT_FOUND) 
      return cmd_out(ERR_INVARG, "Given account (intra) is deleted"); 
// Strip username
   ptr = name;
   name = next_token(&ptr, "@");
   if (name == NULL) return cmd_out(ERR_INVARG, "No username given");
// Check name (len <= 8, letters, digits)
   len = strlen(name);
   if (len < 2 || len > 8) return cmd_out(ERR_INVARG, "Invalid name lenght");
   for (i=0; i < len; i++)
   {  c = (unsigned char)name[i];  
      if ((c<'0' || c>'z') || (c>'9' && c<'a'))
         return cmd_out(ERR_INVARG, "Invalid chars in name");
   }
// Check hostname
   if (ptr == NULL) return cmd_out(ERR_INVARG, "No hostname given");
   if (strcasecmp(ptr, "home.oganer.net") != 0)
      return cmd_out(ERR_INVARG, "Inallowed hostname");
   host = ptr;
// Add login
   snprintf(buf, sizeof(buf), "%s %s %s", conf_intrascript,
               name, ptr);
   fd = popen(buf, "r");
   if (fd == NULL) return cmd_out(ERR_IOERROR, NULL); 
   if (fgets(buf, sizeof(buf), fd) == NULL)
      return cmd_out(ERR_IOERROR, NULL);
   ptr = buf;
   str = next_token(&ptr, " \t");
   if (str == NULL) return cmd_out(ERR_IOERROR, NULL);
   if (strcmp(str, "000") != 0)
      return cmd_out(ERR_IOERROR, "%s", ptr);
   str = next_token(&ptr, " \t\n");
   if (str == NULL) return cmd_out(ERR_IOERROR, "Unexpected script error");
   cmd_out(RET_COMMENT, "User password: %s", str);
   cmd_out(RET_STR, "%s", str);
// Add adder resources
   if ((lockfd = reslinks_lock(LOCK_EX)) != (-1))
   {  reslinks_load(LOCK_UN);
// Add mail resources
      snprintf(buf, sizeof(buf), "%s@%s", name, host);
      reslink_new(RES_MAIL, acc_inet, buf);
      reslinks_save(LOCK_UN);
      reslinks_unlock(lockfd);
   }
   else syslog(LOG_ERR, "cmdh_new_name(): Unable to lock reslinks");

   return cmd_out(SUCCESS, NULL);  
} 

int cmdh_gate(char * cmd, char * args)
{  char * ptr   = args;
   char * str;
   int    i;
   int    rid;
   int    accno; 
   int    lockfd;
// addgate <res> <acc_id> <name>
   
   str = next_token(&ptr, CMD_DELIM);
   if (str == NULL) return cmd_out(ERR_ARGCOUNT, "Arguments expected");
   for (i=0; i < resourcecnt; i++)
      if (strcasecmp(str, resource[i].name) == 0) break;
   if (i >= resourcecnt) return cmd_out(ERR_INVARG, "Invalid resource name");
   rid = i;
   accno = cmd_getaccno(&ptr, NULL);
   if (accno == (-1)) return cmd_out(ERR_INVARG, "Invalid account id");
   str = strtrim(ptr, CMD_DELIM);
   if (str == NULL || *str == '\0') return cmd_out(ERR_ARGCOUNT, "Gate name expected");
   if ((lockfd = reslinks_lock(LOCK_EX)) != (-1))
   {  reslinks_load(LOCK_UN);
      reslink_new(rid, accno, str);
      reslinks_save(LOCK_UN);
      reslinks_unlock(lockfd);
   }
   else 
   {  syslog(LOG_ERR, "cmdh_gate(): Unable to lock reslinks");
      return cmd_out(ERR_IOERROR, "Can't lock gate file");
   }
   return cmd_out(SUCCESS, NULL);
}

int cmdh_delgate(char * cmd, char * args)
{  char * ptr   = args;
   char * str;
   int    accno;
   int    rid;
   int    i;
   int    lockfd;
// delgate <res> <accid> [<name>] 

   str = next_token(&ptr, CMD_DELIM);
   if (str == NULL) return cmd_out(ERR_ARGCOUNT, "Arguments expected");
   for (i=0; i < resourcecnt; i++)
      if (strcasecmp(str, resource[i].name) == 0) break;
   if (i >= resourcecnt) return cmd_out(ERR_INVARG, "Invalid resource name");
   rid = i;
   accno = cmd_getaccno(&ptr, NULL);
   if (accno == (-1)) return cmd_out(ERR_INVARG, "Invalid account id");
   str = strtrim(ptr, CMD_DELIM);
   if ((lockfd = reslinks_lock(LOCK_EX)) != (-1))
   {  reslinks_load(LOCK_UN);
      i = -1;
      while (lookup_accno(accno, &i) != -1)
      {  if (linktab[i].res_id == rid)
         {  if (str == NULL) break;
            if (strcasecmp(str, linktab[i].username) == 0) break;
         }
      }
      if (i >= linktabsz) 
      {  reslinks_unlock(lockfd);
         return cmd_out(ERR_INVARG, "No matching gate found");
      }
      reslink_del(i);
      reslinks_save(LOCK_UN);
      reslinks_unlock(lockfd);
   }
   else 
   {  syslog(LOG_ERR, "cmdh_delgate(): Unable to lock reslinks");
      return cmd_out(ERR_IOERROR, "Can't lock gate file");
   }

   return cmd_out(SUCCESS, NULL);
}

int cmdh_intraupdate(char * cmd, char * args)
{
   system(conf_intrascript);
   return cmd_out(SUCCESS, NULL);
}

int cmd_pdate(time_t tim, char * buf)
{  struct tm  stm;

   if (buf == NULL) return (-1);
   if (localtime_r(&tim, &stm) == NULL)
   {  strcpy(buf, "(invalid)");
      return 0;
   }
   snprintf(buf, 11, "%02d:%02d:%04d", stm.tm_mday, stm.tm_mon+1, 
            stm.tm_year+1900);
   return 0;
}

char  * datedelim = ":/\\.,'`-=";

int cmd_getdate(char ** pptr)
{  char      * ptr;
   char      * str;
   struct tm   stm;

   str = next_token(pptr, " \t\n\r");
   if (str == NULL) return (-1);
   if (strcasecmp(str, "null") == 0) return 0; // special value
   ptr = str;

// zero structure
   bzero(&stm, sizeof(stm));

// get day of month
   str = next_token(&ptr, datedelim);
   if (str == NULL) return (-1);
   stm.tm_mday = strtol(str, NULL, 10);
   if (stm.tm_mday < 1) return (-1);

// get month
   str = next_token(&ptr, datedelim);
   if (str == NULL) return (-1);
   stm.tm_mon = strtol(str, NULL, 10) - 1;
   if (stm.tm_mon < 0) return (-1);

// get year
   str = next_token(&ptr, datedelim);
   if (str == NULL) return (-1);
   stm.tm_year = strtol(str, NULL, 10);
   if (stm.tm_year < 0) return (-1);
   if (stm.tm_year < 100) stm.tm_year+=100;  // 20yy - default
   else
   {  if (stm.tm_year > 1900) stm.tm_year -= 1900;  // full year format
      else return (-1);
   }

// initialize other fields
   stm.tm_isdst=-1;   

   return mktime(&stm);
}

int cmdh_setstart(char * cmd, char * arg)
{  char * ptr=arg;
   int    accno;
   time_t tim;
   int    rc;
   acc_t  acc;

   accno = cmd_getaccno(&ptr, NULL);
   if (accno < 0) return cmd_out(ERR_INVARG, "Invalid account id");
   tim = cmd_getdate(&ptr);
   if (tim < 0) return cmd_out(ERR_INVARG, "Invalid date");
   rc = acc_baselock(&Accbase);
   if (rc != SUCCESS) return cmd_out(ERR_IOERROR, NULL);
   rc = acci_get(&Accbase, accno, &acc);
   if (rc == IO_ERROR || rc == NOT_FOUND || 
       rc == ACC_BROKEN || rc == ACC_DELETED) acc_baseunlock(&Accbase);
   if (rc == IO_ERROR) return cmd_out(ERR_IOERROR, NULL);
   if (rc == NOT_FOUND) return cmd_out(ERR_NOACC, NULL);
   if (rc == ACC_BROKEN) return cmd_out(ERR_NOACC, "Account is broken");
   if (rc == ACC_DELETED) return cmd_out(ERR_NOACC, "Account is deleted");
   if (strcasecmp(cmd, "setstart") == 0) acc.start = tim;
   else acc.stop = tim;
   rc = acci_put(&Accbase, accno, &acc);
   acc_baseunlock(&Accbase);
   if (rc <= 0) return cmd_out(RET_SUCCESS, NULL);
      return cmd_out(ERR_IOERROR, NULL);
}

int cmdh_new_vpn (char * cmd, char * args)
{  char   * ptr  = args;
   char   * str;  
   int      rc;
   acc_t    acc;
   int      acc_inet;

   char   * name = NULL;
   char   * addr = NULL;
   int      sum  = 0;

   int      len;
   int      lockfd;

// new_name <name> <ip-addr> <sum>

   str = next_token(&ptr, CMD_DELIM);
   if (str == NULL) return cmd_out(ERR_ARGCOUNT, " Username expected");
   name = str;

   str = next_token(&ptr, CMD_DELIM);
   if (str == NULL) return cmd_out(ERR_ARGCOUNT, "Address expected");
   addr = str;

   str = next_token(&ptr, CMD_DELIM);
   if (str != NULL) sum = strtol(str, NULL, 10);

// Check name (len <= 16, letters, digits)
   len = strlen(name);
   if (len < 2 || len > 16) return cmd_out(ERR_INVARG, "Invalid name lenght");

   memset(&acc, 0, sizeof(acc));
   acc.balance = sum;
   acc.tag |= ATAG_PAYMAN;
   rc=acc_add(&Accbase, &acc);
   if (rc < 0) return cmd_out(ERR_IOERROR, NULL);
   acc_inet = rc;  

   cmd_out(RET_COMMENT, "account: %d", acc_inet);
   cmd_out(RET_INT, "%d", acc_inet);

   if ((lockfd = reslinks_lock(LOCK_EX)) != (-1))
   {  reslinks_load(LOCK_UN);
      reslink_new(RES_ADDER, acc_inet, name);
      reslink_new(RES_INET, acc_inet, addr);
      reslinks_save(LOCK_UN);
      reslinks_unlock(lockfd);
   }
   else syslog(LOG_ERR, "cmdh_new_vpn(): Unable to lock reslinks");

   return cmd_out(RET_SUCCESS, NULL);
}

int cmdh_lock (char * cmd, char * args)
{
   if (strcasecmp(cmd, "lock") == 0)
   {  acc_async_on(&Accbase);
      log_async_on(&Logbase);
   }
   else
   {  acc_async_off(&Accbase);
      log_async_off(&Logbase);
   }

   return cmd_out(RET_SUCCESS, NULL);
}

// Set account tariff number

int cmdh_accres(char * cmd, char * args)
{
   char * ptr = args;
   char * str;
   int    accno;
   acc_t  acc;
   int    rc;
   int    value;

   NeedUpdate = 1;
   accno = cmd_getaccno(&ptr, NULL);
   if (accno < 0)
      return cmd_out(ERR_INVARG, NULL);
    
   str = next_token(&ptr, CMD_DELIM);
   if (str == NULL)
      return cmd_out(ERR_ARGCOUNT, "Argument expected");
   value = strtol(str, NULL, 10);   


   rc = acc_baselock(&Accbase);
   if (rc != SUCCESS) return cmd_out(ERR_IOERROR, NULL);

   rc = acci_get(&Accbase, accno, &acc);
   if (rc == IO_ERROR || rc == NOT_FOUND || rc == ACC_BROKEN ||
          rc == ACC_DELETED) acc_baseunlock(&Accbase);
   if (rc == IO_ERROR)  return cmd_out(ERR_IOERROR, NULL);
   if (rc == NOT_FOUND) return cmd_out(ERR_NOACC, NULL);
   if (acc.tag == ATAG_BROKEN) return cmd_out(ERR_ACCESS, "Account is broken");
   if (acc.tag == ATAG_DELETED)  return cmd_out(ERR_ACCESS, "Account is empty");

   acc.tariff = value;

   rc = acci_put(&Accbase, accno, &acc);

   acc_baseunlock(&Accbase);
   if (rc <= 0) 
      return cmd_out(RET_SUCCESS, NULL);
   else
      return cmd_out(ERR_IOERROR, NULL);
}

// perform charge transaction for each account w/ charged inet tariff

int cmdh_docharge(char * cmd, char * args)
{  int        i;
   is_data_t  data;

   char     * str;
   int        rc;
   int        recs;


   memset(&data, 0, sizeof(data));
   data.res_id   = 0;
   data.user_id  = 0;
   data.proto_id = PROTO_CHARGE;

   recs = acc_reccount(&Accbase);
   if (recs < 0) return cmd_out(ERR_IOERROR, NULL);

   for (i=0; i < recs; i++)
   {  
      rc = acc_charge_trans(&Accbase, &Logbase, i, &data);
         
//         cmd_out(RET_COMMENT, "#%d DATA: rid:%d, uid:%d, val:%d, proto:%d, host:%X", i,
//                  data.res_id, data.user_id, data.value, data.proto_id, data.host);

// return result (& acc state) to module
      if (rc != ACC_NOCHARGE)
      {  switch (rc)
         {  case NEGATIVE:
               str="Account is negative"; break;
            case SUCCESS:
               str="Account is valid"; break;
            case NOT_FOUND:
               str="Account not found"; break;
            case ACC_DELETED:
               str="Account is deleted"; break;
            case IO_ERROR:
               return cmd_out(ERR_IOERROR, NULL);
            case ACC_BROKEN:
               str="Account is broken"; break;
            case ACC_FROZEN:
               str="Account is frozen"; break;
            case ACC_OFF:
               str="Account is turned off"; break;
            default:
               str="Unknown error";
         }      
         cmd_out(RET_COMMENT, "#%d - %s", i, str);
      }
   }

   NeedUpdate = 1;

   return cmd_out(RET_SUCCESS, "(success for module)");
}

int cmdh_tdump(char * cmd, char * args)
{  int i;

   for (i=0; tariffs_info[i].tariff >= 0; i++)
   {  cmd_out(RET_COMMENT, "%d\t%s", tariffs_info[i].tariff, tariffs_info[i].name);
   }

   for (i=0; tariffs_limit[i].tariff >= 0; i++)
   {  cmd_out(RET_COMMENT, "%d\t%g", tariffs_limit[i].tariff, tariffs_limit[i].min);
   }

   for (i=0; tariffs_inet[i].tariff >= 0; i++)
   {  cmd_out(RET_COMMENT, "%5d %5d %5d %5d %5g %5g %5g %5d %5g %10lld %5u", 
         tariffs_inet[i].tariff, 
         tariffs_inet[i].weekday,
         tariffs_inet[i].hour_from,
         tariffs_inet[i].hour_to,
         tariffs_inet[i].price_in,
         tariffs_inet[i].price_out,
         tariffs_inet[i].month_charge,
         tariffs_inet[i].sw_tariff,
         tariffs_inet[i].sw_summ,
         tariffs_inet[i].sw_inetsumm,
         tariffs_inet[i].flags);
   }

   return cmd_out(RET_SUCCESS, NULL);
}

//--------------------

/* * * * * * * * * * * * * * * * * * * * *\
 *  Prepare & Respond w/ aligned table   *
\* * * * * * * * * * * * * * * * * * * * */

outtab_t outtab = {0, NULL, 0, NULL};


/* Start new table */

int cmd_tab_begin ()
{
// Ensure that table is empty
   cmd_tab_abort();

   return 0;
}


/* Add value to table (add first row if none) */

int cmd_tab_value (char * format, ...)
{  va_list    ap;
   char     * newval;
   int        n;
   tabrow_t * row;
   int        rc;
   void     * tmp;

   if (format == NULL) return (-1);

   if (outtab.cnt_rows < 1) cmd_tab_newrow(); 

// Make value string
   va_start(ap, format);
   n = vasprintf(&newval, format, ap);
   va_end(ap);
  
// Insert value item   
   row = outtab.itm_rows + (outtab.cnt_rows - 1); // ptr to last row
   rc = da_ins(&(row->cnt_vals), &(row->itm_vals), sizeof(char*), (-1), &newval);
   if (rc < 0)
   {  free(newval);
      return (-1); 
   }
   newval = NULL;

// Update/Insert width value
   while(outtab.cnt_cols < row->cnt_vals)
   {  tmp = da_new(&(outtab.cnt_cols), &(outtab.itm_widths), sizeof(int), (-1));
      if (tmp < 0) return (-1);
   }
   if (outtab.itm_widths[row->cnt_vals - 1] < n) outtab.itm_widths[row->cnt_vals - 1] = n;

   return 0;
}


/* Switch to new row */

int cmd_tab_newrow()
{  void * tmp;

   tmp = da_new(&(outtab.cnt_rows), &(outtab.itm_rows), sizeof(tabrow_t), (-1));
   if (tmp == NULL) return (-1);

   return 0;
}


char fmtstr[16];
/* Print & destroy table */

int cmd_tab_end()
{  int         i, n;
   tabrow_t  * row;

   for(i=0; i < outtab.cnt_rows; i++)
   {  cmd_out_begin(RET_ROW);
      row = outtab.itm_rows + i;
      for (n=0; n < row->cnt_vals; n++)
      {  snprintf(fmtstr, sizeof(fmtstr), "%%%ds  ", outtab.itm_widths[n]);
         cmd_out_add(fmtstr, row->itm_vals[n]);
      }
      cmd_out_end();
   }

   cmd_tab_abort();

   return 0;
}


/* Abort table (destroys table data) */

int cmd_tab_abort()
{  int         i, n;
   tabrow_t  * row;

   da_empty(&(outtab.cnt_cols), &(outtab.itm_widths), sizeof(int));

   for(i=0; i < outtab.cnt_rows; i++)
   {  row = outtab.itm_rows + i;
      for (n=0; n < row->cnt_vals; n++)
      {  if (row->itm_vals[n] != NULL) free(row->itm_vals[n]);
      }
      da_empty(&(row->cnt_vals), &(row->itm_vals), sizeof(char*));
   }
   da_empty(&(outtab.cnt_rows), &(outtab.itm_rows), sizeof(tabrow_t));

   return 0;
}

//--------------------

/* * * * * * * * * * * * * * * *\
 * CARD[S] - pay cards support *
\* * * * * * * * * * * * * * * */

int cmdh_card  (char * cmd, char * args)
{
   char    ** row;
   int        resrows;
   int        i, c;

   row = db2_search(DBdata, 0, "SELECT id, val, "
                              "(SELECT name FROM resources WHERE id = res_id), " 
                              "to_char(gen_time,'DD.MM.YYYY'), "
                              "to_char(expr_time,'DD.MM.YYYY'), batchno FROM paycards "
                              "WHERE printed ORDER BY id;");
   if (row == NULL) return cmd_out(ERR_SYSTEM, "Database error");

   resrows = db2_howmany(DBdata);
   if (resrows > 0)
   {  cmd_tab_begin();
      for (i=0; i < resrows; i++)
      {  if (i > 0) db2_next(DBdata);
         cmd_tab_newrow();
         for (c=0; row[c] != NULL; c++)
         {  cmd_tab_value("%s", row[c]);
         }
      }
      cmd_tab_end();
      return cmd_out(RET_SUCCESS, "%d active cards", resrows);
   } 
   else return cmd_out(RET_SUCCESS, "no rows");
}

int cmdh_card_gen (char * cmd, char * args)
{
   char     * ptr = args;
   char     * str;
   int        no, days, sum, e, i;
   long long  res;
   char    ** row;
   unsigned long long  pin;
   unsigned long long  code;
    
// gen <no_of_cards> <no_of_days> <value> [<res_name>]

// parse number of cards 
   str = next_token(&ptr, CMD_DELIM);
   if (str == NULL) return cmd_out(ERR_ARGCOUNT, "Arguments expected: <no> <days> <value> [<res>]");
   no = strtol(str, NULL, 10);
   if (no < 1) return cmd_out(ERR_INVARG, "Invalid number of cards");

// parse card lifetime
   str  = next_token(&ptr, CMD_DELIM);
   if (str == NULL) return cmd_out(ERR_ARGCOUNT, "Card lifetime (days) expected");
   days = strtol(str, NULL, 10);
   if (days < 1) return cmd_out(ERR_INVARG, "Invalid card lifetime");

// parse card value
   str = next_token(&ptr, CMD_DELIM);
   if (str == NULL) return cmd_out(ERR_ARGCOUNT, "Card value expected");
   sum = strtol(str, NULL, 10);
   if (sum < 1) return cmd_out(ERR_INVARG, "Invalid card value");

// parse optional resource ident & get res_id (for resource cards)
   str = next_token(&ptr, CMD_DELIM);
   if (str != NULL)
   {  row = db2_search(DBdata, 0, "SELECT id FROM resources WHERE name = '%s' AND NOT deleted;",
                      ESCARG1(str));
      if (row == NULL) return cmd_out(ERR_SYSTEM, "Database error");
      if (row[0] == NULL)return cmd_out(ERR_INVARG, "Invalid resource name");  
      res = strtoll(row[0], NULL, 10);
   }
   else res = (-1);
   snprintf(argbuf2, sizeof(argbuf2), "%lld", res);

// create new cards batch
   row = db2_search(DBdata, 0, "SELECT batch_new('comment stub');");
   if (row == NULL) return cmd_out(ERR_SYSTEM, "Database error");
   snprintf(argbuf1, sizeof(argbuf1), "%s", row[0]);

   cmd_out(RET_COMMENT, "Generating batch #%s (%d cards %d, %d days)", *row, no, sum, days); 

// machine output: batch number
   cmd_out(RET_INT, "%s", row[0]);

   e = 0;
   for (i=0; i < no; i++)
   { 
   // generate random PIN
      ((long*)(&pin))[0] = arc4random();
      ((long*)(&pin))[1] = arc4random();
      pin = pin % 10000000000000000LL;

   // generate random barcode 
      ((long*)(&code))[0] = 0;
      ((long*)(&code))[1] = arc4random();
      code = code % 100000000LL;

   // (try to) add new card (can fail on non-unique PIN)
      row = db2_search(DBdata, 0, "SELECT pcard_new(%d, %lld, %lld, %d, %s, %s)", 
                       sum, pin, code, days, argbuf1, res < 0 ? "NULL" : argbuf2);
        
   // on fail: loop to generate another PIN, fail on permanent errors
      if (row == NULL || row[0] == NULL || strcasecmp(row[0], "t") != 0)
      {  if (e > 2) return cmd_out(ERR_SYSTEM, "SQL error on cards generation");
         i--;
         e++;
         continue;  // loop again
      }
      e = 0;

      cmd_out(RET_COMMENT, "Card %d done", i);
   } 

   return cmd_out(RET_SUCCESS, NULL);
}

int cmdh_card_list (char * cmd, char * args)
{
   char     * ptr = args;
   char     * str;
   char    ** row;
   int        resrows;
   int        i, no, c;   

// list [batchno]

   str = next_token(&ptr, CMD_DELIM);
   if (str == NULL)
   {  row = db2_search(DBdata, 0, "SELECT DISTINCT batchno FROM paycards WHERE NOT printed;");
      if (row == NULL) return cmd_out(ERR_SYSTEM, "Database error");

      resrows = db2_howmany(DBdata);
      if (resrows > 0)
      {  cmd_out_begin(RET_COMMENT);
         cmd_out_add("Available batch numers: ");
         for(i=0; i < resrows; i++)
         {  if (i > 0) db2_next(DBdata);
            cmd_out_add("%s%s", *row, i < (resrows-1) ? " ":"");
         } 
         cmd_out_end();
      }
      else cmd_out(RET_COMMENT, "No available batches");
      return cmd_out(RET_SUCCESS, NULL);
   } 

   no = strtol(str, NULL, 10);
   if (no < 1) cmd_out(ERR_INVARG, "Invalid batch number");

   row = db2_search(DBdata, 0, "SELECT id, pin, val, to_char(gen_time,'DD.MM.YYYY'), "
                              "to_char(expr_time,'DD.MM.YYYY') FROM paycards "
                              "WHERE NOT printed AND batchno = %d;", no);
   if (row == NULL) return cmd_out(ERR_SYSTEM, "Database error");

   resrows = db2_howmany(DBdata);
   if (resrows > 0)
   {  cmd_tab_begin();
      for (i=0; i < resrows; i++)
      {  if (i > 0) db2_next(DBdata);
         cmd_tab_newrow();
         for (c=0; row[c] != NULL; c++)
         {  if (c != 1) cmd_tab_value("%s", row[c]);
            else cmd_tab_value("%016s", row[c]);
         }
      }
      cmd_tab_end();
      return cmd_out(RET_SUCCESS, NULL);
   } 
   else return cmd_out(RET_SUCCESS, "No rows");
}

int cmdh_card_emit (char * cmd, char * args)
{
   char     * ptr = args;
   char     * str;
   int        no;
   char    ** row;

// emit <batchno>

   str = next_token(&ptr, CMD_DELIM);
   if (str == NULL) return cmd_out(ERR_ARGCOUNT, "Batch number expected");
   no = strtol(str, NULL, 10);
   if (no < 1) cmd_out(ERR_INVARG, "Invalid batch number");

   row = db2_search(DBdata, 0, "SELECT batch_emit(%d)", no);

   if (row == NULL || row[0] == NULL || strcasecmp(row[0], "t") != 0) return cmd_out(ERR_SYSTEM, "Database error");

   return cmd_out(RET_SUCCESS, NULL);
}

int cmdh_card_check (char * cmd, char * args)
{
   char     * ptr = args;
   char     * sno;
   char     * spin;
   char    ** row;
   int        resrows;
   int        rescols;
   int        i, c;

// check <no> [<pin>]

   sno = next_token(&ptr, CMD_DELIM);
   if (sno == NULL) return cmd_out(ERR_ARGCOUNT, "Card number expected");

   spin = next_token(&ptr, CMD_DELIM);

   // Search for active card
   if (spin == NULL)
      row = dbex_search(DBdata, 0, "chkcard1", ESCARG1(sno));
   else
      row = dbex_search(DBdata, 0, "chkcardpin1", ESCARG1(sno), ESCARG2(spin));

   if (row == NULL) return cmd_out(ERR_SYSTEM, "Database error");

   resrows = db2_howmany(DBdata);
   if (resrows > 0)
   {  for (i=0; i < resrows; i++)
      {  if (i > 0) db2_next(DBdata);
         cmd_out_begin(RET_ROW);
         for (c=0; row[c] != NULL; c++)
         {  cmd_out_add("%s\t", row[c]);
         }
         cmd_out_end();
      }
      return cmd_out(RET_SUCCESS, NULL);
   } 

   // Search for deleted card
   if (spin == NULL)
      row = dbex_search(DBdata, 0, "chkcard2", ESCARG1(sno));
   else
      row = dbex_search(DBdata, 0, "chkcardpin2", ESCARG1(sno), ESCARG2(spin));

   if (row == NULL) return cmd_out(ERR_SYSTEM, "Database error");

   resrows = db2_howmany(DBdata);
   if (resrows > 0)
   {  rescols = 8;
      for (i=0; i < resrows; i++)
      {  if (i > 0) db2_next(DBdata);
         cmd_out_begin(RET_ROW);
         for (c=0; c < rescols; c++)
         {  cmd_out_add("%s\t", row[c]);
         }
         cmd_out_end();
      }
      return cmd_out(RET_SUCCESS, NULL);
   } 

   return cmd_out(ERR_NOACC, "Card not found");
}

int cmdh_card_null (char * cmd, char * args)
{
   char     * ptr = args;
   char     * sno;
   char    ** row;

// null <no> // [<pin>]

   sno = next_token(&ptr, CMD_DELIM);
   if (sno == NULL) return cmd_out(ERR_ARGCOUNT, "Card number expected");

   row = db2_search(DBdata, 0, "SELECT pcard_null('%s')", ESCARG1(sno));
   if (row == NULL || row[0] == NULL || strcasecmp(row[0], "t") != 0) return cmd_out(ERR_SYSTEM, "Database error");
 
   return cmd_out(RET_SUCCESS, NULL);
}

int cmdh_card_nullbatch (char * cmd, char * args)
{
   char     * ptr = args;
   char     * sno;
   char    ** row;

// nullbatch <no>

   sno = next_token(&ptr, CMD_DELIM);
   if (sno == NULL) return cmd_out(ERR_ARGCOUNT, "Card number expected");

   row = db2_search(DBdata, 0, "SELECT batch_null('%s')", ESCARG1(sno));
   if (row == NULL || row[0] == NULL || strcasecmp(row[0], "t") != 0) return cmd_out(ERR_SYSTEM, "Database error");

   return cmd_out(RET_SUCCESS, NULL);
}

int cmdh_card_expire (char * cmd, char * args)
{
   char    ** row;

// expire

   row = db2_search(DBdata, 0, "SELECT pcards_expire()");
   if (row == NULL || row[0] == NULL || strcasecmp(row[0], "t") != 0) return cmd_out(ERR_SYSTEM, "Database error");

   return cmd_out(RET_SUCCESS, NULL);
}

// v0 card assistance hack variables
link_t   v0lnk;
char     linbuf[128];

int cmdh_card_utluser (char * cmd, char * args)
{
   char     * ptr = args;
   char     * sno;
   char     * spin;
   char     * shost;
   int        sum;
   char    ** row;
   int        accno;
   int        ind;
   int        rc;
   acc_t      acc;

// utluser <no> <pin> <host>
//   RET_INT values (for web interface)
//    >=0  - success, sum
//    (-1) - insufficient arguments
//    (-2) - card/PIN incorrect (log fault)
//    (-3) - unable to lookup account (or deleted)
//    (-4) - temporary failure (try later)
//    (-5) - host is banned
//    (-6) - account is broken
//    (-7) - account is frozen
//    (-8) - account is unlimited
//    (-9) - partial transaction
//    (-10)- unsupported card type

   sno   = next_token(&ptr, CMD_DELIM);
   if (sno == NULL)
   {  cmd_out(RET_INT, "%d", (-1));  
      return cmd_out(ERR_ARGCOUNT, "<card_number> <pin_code> <client_host>");
   }

   spin  = next_token(&ptr, CMD_DELIM);
   if (spin == NULL)
   {  cmd_out(RET_INT, "%d", (-1));  
      return cmd_out(ERR_ARGCOUNT, "Card PIN expected");
   } 

   shost = next_token(&ptr, CMD_DELIM);
   if (shost == NULL)
   {  cmd_out(RET_INT, "%d", (-1));  
      return cmd_out(ERR_ARGCOUNT, "Client host expected");
   }

   ind = -1;
   rc = lookup_addr(shost, &ind);
   if (rc == (-1))
   {  cmd_out(RET_INT, "%d", (-3));
      return cmd_out(ERR_INVARG, NULL);
   }
   accno = linktab[ind].accno;

   rc = acc_baselock(&Accbase);
   if (rc != SUCCESS) 
   {  cmd_out(RET_INT, "%d", (-3));
      return cmd_out(ERR_IOERROR, NULL);
   }
   rc = acci_get(&Accbase, accno, &acc);
   acc_baseunlock(&Accbase);

   if (rc == IO_ERROR)
   {  cmd_out(RET_INT, "%d", (-3));
      return cmd_out(ERR_IOERROR, NULL);
   }
   if (rc == NOT_FOUND)
   {  cmd_out(RET_INT, "%d", (-3));  
      return cmd_out(ERR_NOACC, NULL);
   }
   if ((acc.tag & ATAG_BROKEN) != 0)
   {  cmd_out(RET_INT, "%d", (-6));
      return cmd_out(ERR_ACCESS, "Account is broken");
   }
   if ((acc.tag & ATAG_DELETED) != 0)
   {  cmd_out(RET_INT, "%d", (-3));
      return cmd_out(ERR_ACCESS, "Account is deleted");
   }
   if ((acc.tag & ATAG_FROZEN) != 0)
   {  cmd_out(RET_INT, "%d", (-7));
      return cmd_out(ERR_ACCESS, "Account is frozen");
   }
   if ((acc.tag & ATAG_UNLIMIT) != 0)
   {  cmd_out(RET_INT, "%d", (-8));
      return cmd_out(ERR_ACCESS, "Account is unlimited");
   }

// (try to) do utilize card (by user)
   row = db2_search(DBdata, 0, "SELECT pcard_utluser_v0('%s', '%s', '%s');", 
                   ESCARG1(sno), ESCARG2(spin), ESCARG3(shost));
   if (row == NULL) 
   {  cmd_out(RET_INT, "%d", (-4));
      return cmd_out(ERR_SYSTEM, "SQL error");
   }

   sum = strtol(row[0], NULL, 10);
   if (sum < 0)
   {  cmd_out(RET_INT, "%d", sum);
      return cmd_out(ERR_SYSTEM, "Utilize error %d", sum);
   }

   rc = cmd_add(accno, sum);
   if (rc < 0)
   {  cmd_out(RET_INT, "%d", (-9));
      cmd_accerr(rc);
      syslog(LOG_ERR, "INCOMPLETE TRANSACTION (cannot add utilized card #%s value for #%d)", sno, accno);
      return cmd_out(ERR_ACCESS, "Unable to access account");
   }

   NeedUpdate = 1;

   cmd_out(RET_INT, "%d", sum);  
   return cmd_out(RET_SUCCESS, "Card activated (%+d)", sum);
}

int cmdh_debug   (char * cmd, char * args)
{  char   * ptr = args;
   char   * str;

   str = next_token(&ptr, CMD_DELIM);
   if (str == NULL)
   {  return cmd_out(RET_SUCCESS, "Debug is %s", DumpQuery ? "on":"off");
   }

   if (strcasecmp(str, "on") == 0)        DumpQuery = 1;
   else if (strcasecmp(str, "off") == 0)  DumpQuery = 0;
   else return cmd_out(ERR_INVARG, "[ {on | off} ]");

   return cmd_out(RET_SUCCESS, "Debug is turned %s", DumpQuery ? "on":"off");
}

