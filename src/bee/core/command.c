/* $RuOBSD$ */

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
#include <core.h>
#include <command.h>
#include <res.h>
#include <links.h>

int cmdh_save(char * cmd, char * args)
{
   reslinks_save(LOCK_EX);
   return cmd_out(SUCCESS, NULL);
}


command_t  cmds[]=
{  {"exit",	cmdh_exit,	0},  // close session
   {"ver",	cmdh_ver,       0},  // get version number
   {"user",     cmdh_notimpl,	0},  // *** username (human id)
   {"pass",	cmdh_notimpl,	1},  // *** password (human id)
   {"authkey",	cmdh_notimpl,	0},  // *** public key (machine id)
   {"authsign",	cmdh_notimpl,   1},  // *** signature (machine id)
   {"acc",      cmdh_acc,	4},  // show account state
   {"look",	cmdh_lookup,	4},  // lookup links
   {"log",	cmdh_log,	4},  // view transactions log
   {"freeze",	cmdh_freeze,	4},  // freeze account
   {"unfreeze",	cmdh_freeze,	4},  // unfreeze account
   {"on",	cmdh_freeze,	4},  // turn account on
   {"off",	cmdh_freeze,	4},  // turn account off
   {"unlimit",	cmdh_freeze,	4},  // set account to unlimit
   {"limit",	cmdh_freeze,	4},  // turn account off
   {"_break",	cmdh_freeze,	4},  // break down account
   {"_fix",	cmdh_fix,	4},  // validate account
   {"_dump",	cmdh_notimpl,	4},  // *** dump account record
   {"_save",	cmdh_notimpl,	4},  // *** store dump to account
   {"new",	cmdh_new,	4},  // create new account
   {"add",	cmdh_add,	4},  // do add transaction 
   {"res",	cmdh_res,	4},  // do resource billing transaction
   {"update",	cmdh_update,	4},  // set flag to update filters
   {"human",    cmdh_human,     0},  // suppress non-human messages
   {"machine",  cmdh_human,	0},  // suppress human comments
   {"date",	cmdh_date,	0},  // show time/date
   {"report",	cmdh_report,	4},  // show report
   {"del",      cmdh_delete,    4},  // (alias) delete account
   {"delete",   cmdh_delete,    4},  // delete account
   {"gate",	cmdh_notimpl,	4},  // (alias) add gate
   {"addgate",	cmdh_notimpl,	4},  // add gate
   {"delgate",	cmdh_notimpl,	4},  // delete gate
   {"allow",	cmdh_notimpl,	4},  // allow gate usage
   {"disallow",	cmdh_notimpl,	4},  // disallow gate usage
   {"new_contract", cmdh_new_contract, 4},  // MACRO create two accounts, return #
   {"new_name", cmdh_new_name,  4},  // MACRO create user & return password

// debug commands
   {"save", 	cmdh_save, 	4},

   {NULL,NULL,0}       // terminator
};

char * errmsg[]=
{  "Access Denied",              // 400
   "Unknown command",		 // 401
   "Command not implemented",    // 402
   "Incorrect argument count",   // 403
   "Invalid argument",           // 404
   "I/O Error",                  // 405
   "Account not found",          // 406
   "System error"		 // 407
};
int  errmsgcnt=sizeof(errmsg)/sizeof(char*);

char Buf[128];

char * acc_stat[]=
{  "(EMPTY) ",
   "(BROKEN)",
   "(FROZEN)",
   "(OFF)   ",
   "(UNLIM) ",
   "(VALID) "
};

/////////////////////////////////////////////////////////////////////////////
//				STABLE					   //
/////////////////////////////////////////////////////////////////////////////

int cmd_exec(char * cmd)
{  char * ptr=cmd;
   char * str;
   int    i;

   str=next_token(&ptr, CMD_DELIM);
   if (str==NULL) return cmd_out(ERR_INVCMD, "Type command (or exit)");
   for (i=0; cmds[i].ident; i++)
   {  if (strcmp(str, cmds[i].ident) == 0)
         return cmds[i].proc(str,ptr);
   }
   return cmd_out(ERR_INVCMD, NULL);
}

int cmd_out(int err, char * format, ...)
{  char      buf[256];
   va_list   valist;
   int       headlen;

   if (err==RET_COMMENT && HumanRead==0) return 0;
   if ((err==RET_STR || err==RET_INT || err==RET_BIN) && MachineRead==0) 
      return 0;

   headlen=sprintf(buf,"%03d ", err);
   if (format != NULL)
   {  va_start(valist, format);
      vsnprintf(buf+headlen, sizeof(buf)-headlen, format, valist);
      va_end(valist);
   }
   else
   {  if (err > 399 && err< 400+errmsgcnt) format=errmsg[err-400];
      if (err == 0) format="Success";
      snprintf(buf+headlen, sizeof(buf)-headlen, "%s",
           format!=NULL ? format : "");
   }
   return link_puts(ld, buf);
}

int cmd_intro()
{  return cmd_out(RET_SUCCESS, "Billing daemon 0.0.1.0 ready");   
}

int cmdh_exit(char * cmd, char * args)
{  cmd_out(RET_COMMENT, "Have a nice day");
   return CMD_EXIT;
}

int cmdh_notimpl(char * cmd, char * args)
{   return cmd_out(ERR_NOTIMPL, NULL);   }

int cmdh_ver(char * cmd, char * args)
{  cmd_out(RET_COMMENT, "Billing project ver. 0.0.1.0");
   cmd_out(RET_INT, "00000100");
   return cmd_out(RET_SUCCESS, NULL);
}

///////////////////////////////////////////////////////////////////////////
//				UNSTABLE				 //
///////////////////////////////////////////////////////////////////////////

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
   char   * ptr=args;
   int      ind;
   int      accno;
   money_t  sum;
   int      rc;
   int      i;


   memset(&data, 0, sizeof(data));

// get resource ident
   str=next_token(&ptr, CMD_DELIM);
   if (str==NULL) return cmd_out(ERR_ARGCOUNT, NULL);
 
   data.res_id=(-1);
   for (i=0; i<resourcecnt; i++)
   {  if (strcmp(resource[i].name, str) == 0)
      {  data.res_id=i;
         break;
      }
   }
   if (data.res_id == (-1)) 
      return cmd_out (ERR_INVARG, "Invalid resource name");
// get gate ident
   str=next_token(&ptr, CMD_DELIM);
   if (str==NULL) return cmd_out(ERR_ARGCOUNT, NULL);

   ind=-1;
   if (lookup_resname(data.res_id, str, &ind)<0)
      return cmd_out(ERR_INVARG, "Invalid gate ident");
   data.user_id=linktab[ind].user_id;
   accno=linktab[ind].accno;
// get other fixed int args (val, proto)
   for (i=2; i<4; i++)
   {  str=next_token(&ptr, CMD_DELIM);
      if (str==NULL) return cmd_out(ERR_ARGCOUNT, NULL);
      ((unsigned int*)(&data))[i]=strtoul(str, NULL, 0);
   }
// get host IP-address
   str=next_token(&ptr, CMD_DELIM);
   if (str==NULL) return cmd_out(ERR_ARGCOUNT, NULL);
   data.host.s_addr=inet_addr(str);    
// get additional proto id (local tcp/udp port) if any
   str=next_token(&ptr, CMD_DELIM);
   if (str != NULL) data.proto2=strtol(str, NULL, 0);    

   cmd_out(RET_COMMENT, "DATA: rid:%d, uid:%d, val:%d, proto:%d, host:%X\n",
        data.res_id, data.user_id, data.value, data.proto_id, data.host);

// Count sum
   sum=resource[data.res_id].count(&data);
// Do account transaction
   rc=acc_trans(&Accbase, accno, sum, &data, &Logbase);
// return result (& acc state) to module
   switch (rc)
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
   cmd_out(RET_COMMENT, str);
   cmd_out(RET_INT, "%d", rc);
   NeedUpdate=1;
   return cmd_out(RET_SUCCESS, "(success for module)");
}

int cmd_getaccno(char ** args, lookup_t * prev)
{  
   char * ptr;
   char * str;
   int    ind=-1;
   int    rid;
   int    uid;
   int    i;
   int    flag;
   int    rc;

   if (args != NULL) ptr=*args;

   if (prev != NULL && args == NULL)
   {  switch(prev->what)
      {  case LF_ALL:
            prev->ind++;
            if (prev->ind >= prev->no) return (-1);
	    return prev->ind;
         case LF_NAME:
            rc=lookup_name(prev->str, &prev->ind);
            break;
         case LF_ADDR:
            rc=lookup_addr(prev->str, &prev->ind);
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

   if (prev != NULL) prev->what=LF_NONE;

   str=next_token(&ptr, CMD_DELIM);
   if (str==NULL) return (-1);
   if ((flag=strcmp(str, "name"))==0 || strcmp(str, "addr")==0)
   {  str=next_token(&ptr, CMD_DELIM);
      if (str==NULL) return (-1);
      ind=-1;
      if (flag != 0) rc=lookup_addr(str, &ind);
      else rc=lookup_name(str, &ind);
      if (rc==(-1))
      {  cmd_out(RET_COMMENT, "Can't lookup name");
         return (-1);
      }
      if (prev != NULL)
      {  prev->what= flag ? LF_ADDR : LF_NAME;
         prev->ind=ind;
         prev->str=str;
      }
      *args=ptr;
      cmd_out(RET_COMMENT, "%s-%d\t#%04d\t%s", 
              resource[linktab[ind].res_id].name,
              linktab[ind].user_id, linktab[ind].accno,
              linktab[ind].username);
      return linktab[ind].accno;
   }
   for (i=0; i<resourcecnt; i++)
     if (strcmp(str, resource[i].name)==0) break;
   if (i<resourcecnt)
   {  rid=i;
      str=next_token(&ptr, CMD_DELIM);
      if (str==NULL) return (-1);
      if (strcmp(str, "name") != 0)
      {  uid=strtol(str, NULL, 0);
         rc=lookup_res(rid, uid, &ind);
      }
      else 
      {  str=next_token(&ptr, CMD_DELIM);
         if (str==NULL) return (-1);
         rc=lookup_resname(rid, str, &ind);
      }
      if (rc==(-1))
      {  cmd_out(RET_COMMENT, "Can't lookup resource");
         return (-1);
      }
      else
      {  *args=ptr;
         cmd_out(RET_COMMENT, "%s-%d\t#%04d\t%s", 
                 resource[linktab[ind].res_id].name,
                 linktab[ind].user_id, linktab[ind].accno,
                 linktab[ind].username);
         return linktab[ind].accno;
      }
   }
   if (strcmp(str, "*") == 0)
   {  *args=ptr;
      if (prev != NULL)
      {  prev->what=LF_ALL;
         prev->ind=0;
         prev->no=acc_reccount(&Accbase);
      }
      return 0; 
   }
   *args=ptr;                // always
   rc=strtol(str, &ptr, 0);  // ptr is down
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
   char * org="UOFBD";
   char   mask[6];

   accno=cmd_getaccno(&ptr, NULL);
   if (accno != (-1))
   {  rc=acc_get(&Accbase, accno, &acc);
      if (rc == IO_ERROR) return cmd_out(ERR_IOERROR, NULL);
      if (rc == NOT_FOUND) return cmd_out(ERR_NOACC, NULL);
      bit=5;
      strlcpy(mask, org, sizeof(mask));
      for (b=0; b<5; b++)
      {  if ((acc.tag & 1<<b))
         {  if (bit == 5) bit=b;
         }
         else mask[4-b]='-';
      }
      cmd_out(RET_COMMENT, "#%04d  %s %s  %+10.2f",
          accno, mask, acc_stat[bit], acc.balance);
      cmd_out(RET_STR, "%d %d %e", accno, acc.tag, acc.balance);
      return cmd_out(RET_SUCCESS, "");
   }
   else
   {
      if (next_token(&ptr, CMD_DELIM)!=NULL)
         return cmd_out(ERR_NOACC, NULL);
      recs=acc_reccount(&Accbase);
      if (recs<0) return cmd_out(ERR_IOERROR, NULL);

      for (i=0; i<recs; i++)
      {  rc=acc_get(&Accbase, i, &acc);
         if (rc == IO_ERROR) return cmd_out(ERR_IOERROR, NULL);
         if (acc.tag & ATAG_DELETED) continue;
         bit=5;
         strlcpy(mask, org, sizeof(mask));
         for (b=0; b<5; b++)
         {  if ((acc.tag & 1<<b))
            {  if (bit == 5) bit=b;
            }
            else mask[4-b]='-';
         }
         cmd_out(RET_COMMENT, "#%04d  %s %s  %+10.2f",
            i, mask, acc_stat[bit], acc.balance);
//         cmd_out(RET_STR, "%d %d %e", i, acc.tag, acc.balance);
//         return cmd_out(RET_SUCCESS, "");
      }
/*
      {  rc=acc_get(&Accbase, i, &acc);
         if (rc == IO_ERROR) return cmd_out(ERR_IOERROR, NULL);
         rc=cmd_out(RET_COMMENT, "#%04d  %s  %+10.2f",
            i, acc_stat[acc.tag+1], acc.balance);
         if (rc<0) return rc;
      }
*/
   return cmd_out(RET_SUCCESS, NULL);
   }
}

///////////////////////////////////////////////////////////////////////////
//				DEBUGING				 //
///////////////////////////////////////////////////////////////////////////

int cmdh_freeze(char * cmd, char * args)
{ 
   char * ptr=args;
   int    accno;
   acc_t  acc;
   int    rc;

   NeedUpdate=1;
   accno=cmd_getaccno(&ptr, NULL);
   if (accno != (-1))
   {  rc=acc_baselock(&Accbase);
      if (rc != SUCCESS) return cmd_out(ERR_IOERROR, NULL);
      rc=acci_get(&Accbase, accno, &acc);
      if (rc==IO_ERROR || rc==NOT_FOUND || rc==ACC_BROKEN ||
          rc==ACC_DELETED) acc_baseunlock(&Accbase);
      if (rc == IO_ERROR) return cmd_out(ERR_IOERROR, NULL);
      if (rc == NOT_FOUND) return cmd_out(ERR_NOACC, NULL);
      if (acc.tag==ATAG_BROKEN)  return 
                             cmd_out(ERR_ACCESS, "Account is broken");
      if (acc.tag==ATAG_DELETED) return 
                             cmd_out(ERR_ACCESS, "Account is empty");
      if (strcmp(cmd, "freeze")==0)
      {  acc.tag |= ATAG_FROZEN;
         rc=acci_put(&Accbase, accno, &acc);
         acc_baseunlock(&Accbase);
         if (rc <= 0) return cmd_out(RET_SUCCESS, NULL);
         return cmd_out(ERR_IOERROR, NULL);
      }      
      if (strcmp(cmd, "unfreeze")==0)
      {  if ((acc.tag & ATAG_FROZEN)==0) 
         {  acc_baseunlock(&Accbase);
            return cmd_out(ERR_ACCESS, "Not frozen");
         }
         acc.tag &= ~ATAG_FROZEN;
         rc=acci_put(&Accbase, accno, &acc);
         acc_baseunlock(&Accbase);
         if (rc <= 0) return cmd_out(RET_SUCCESS, NULL);
         return cmd_out(ERR_IOERROR, NULL);
      }
      if (strcmp(cmd, "on")==0)
      {  if ((acc.tag & ATAG_OFF)==0) 
         {  acc_baseunlock(&Accbase);
            return cmd_out(ERR_ACCESS, "Not OFF");
         }
         acc.tag &= ~ATAG_OFF;
         rc=acci_put(&Accbase, accno, &acc);
         acc_baseunlock(&Accbase);
         if (rc <= 0) return cmd_out(RET_SUCCESS, NULL);
         return cmd_out(ERR_IOERROR, NULL);
      }
      if (strcmp(cmd, "off")==0)
      {  acc.tag |= ATAG_OFF;
         rc=acci_put(&Accbase, accno, &acc);
         acc_baseunlock(&Accbase);
         if (rc <= 0) return cmd_out(RET_SUCCESS, NULL);
         return cmd_out(ERR_IOERROR, NULL);
      }
      if (strcmp(cmd, "_break")==0)
      {  acc.tag |= ATAG_BROKEN;
         rc=acci_put(&Accbase, accno, &acc);
         acc_baseunlock(&Accbase);
         if (rc <= 0) return cmd_out(RET_SUCCESS, "Account got broken");
         return cmd_out(ERR_IOERROR, NULL);
      }
      if (strcmp(cmd, "unlimit")==0)
      {  acc.tag |= ATAG_UNLIMIT;
         rc=acci_put(&Accbase, accno, &acc);
         acc_baseunlock(&Accbase);
         if (rc <= 0) return cmd_out(RET_SUCCESS, "Account set unlimit");
         return cmd_out(ERR_IOERROR, NULL);
      }
      if (strcmp(cmd, "limit")==0)
      {  acc.tag &= ~ATAG_UNLIMIT;
         rc=acci_put(&Accbase, accno, &acc);
         acc_baseunlock(&Accbase);
         if (rc <= 0) return cmd_out(RET_SUCCESS, "Account limited");
         return cmd_out(ERR_IOERROR, NULL);
      }
      return cmd_out(ERR_INVCMD, NULL);
   }
   return cmd_out(ERR_INVARG, NULL);
}

int cmdh_fix(char * cmd, char * args)
{ 
   char * str;
   char * ptr=args;
   int    accno;
   acc_t  acc;
   int    rc;

   NeedUpdate=1;
   accno=cmd_getaccno(&ptr, NULL);
   if (accno != (-1))
   {  rc=acc_baselock(&Accbase);
      if (rc != SUCCESS) return cmd_out(ERR_IOERROR, NULL);
      rc=acci_get(&Accbase, accno, &acc);
      if (rc==IO_ERROR || rc==NOT_FOUND) acc_baseunlock(&Accbase);
      if (rc == IO_ERROR) return cmd_out(ERR_IOERROR, NULL);
      if (rc == NOT_FOUND) return cmd_out(ERR_NOACC, NULL);
      acc.tag &= ~(ATAG_BROKEN|ATAG_DELETED);
      str=next_token(&ptr, CMD_DELIM);
      if (str != NULL) acc.balance=((money_t)strtod(str, NULL));
      rc=acci_put(&Accbase, accno, &acc);
      acc_baseunlock(&Accbase);
      if (rc <= 0) return cmd_out(RET_SUCCESS, NULL);
      return cmd_out(ERR_IOERROR, NULL);
   }
   return cmd_out(ERR_INVARG, NULL);
}

int cmdh_new(char * cmd, char * args)
{  int      rc;
   acc_t    acc;
   char   * ptr=args;
   char   * str;
   double   sum;

   memset(&acc, 0, sizeof(acc));
   acc.balance=0;
   rc=acc_add(&Accbase, &acc);
   if (rc < 0) return cmd_out(ERR_IOERROR, NULL);
   cmd_out(RET_COMMENT, "Account %d created", rc);
   str=next_token(&ptr, CMD_DELIM);
   if (str != NULL)
   {  sum=strtod(str, NULL);
   }
   NeedUpdate=1;
   return cmd_out(RET_SUCCESS, Buf);
}

int cmdh_add(char * cmd, char * args)
{  char *    ptr=args;
   char *    str;
   int       accno;
   money_t   sum;
   int rc; 
   accno=cmd_getaccno(&ptr, NULL);
   if (accno>=0)
   {  str=next_token(&ptr, CMD_DELIM);
      if (str != NULL)
      {  sum=strtod(str, NULL);
         rc=cmd_add(accno, sum);
         if (rc>=0) cmd_out(RET_COMMENT, "Transaction successful");
         else 
         {  cmd_accerr(rc);
            return cmd_out(ERR_ACCESS, "Unable to access account");
         }
      }
      else return cmd_out(ERR_ARGCOUNT, NULL);
   }
   else return cmd_out(ERR_INVARG, NULL);
   NeedUpdate=1;
   return cmd_out(RET_SUCCESS, NULL);
}

int cmd_accerr(int rc)
{  char * str;

   switch (rc)
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
   return cmd_out(RET_COMMENT, "%s", str);
}

int cmd_add(int accno, money_t sum)
{  is_data_t data;
   struct sockaddr_in  addr;
   socklen_t           len=sizeof(addr);

   memset(&data, 0, sizeof(data));
   data.res_id =RES_ADDER;
   data.user_id=accno;
   if (getpeername(ld->fd, (struct sockaddr*)&addr, &len)==0)
      data.host=addr.sin_addr;
   
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
   int       accno=-1;
   char    * ptr=args;
   char    * str;
   int       line=-1;
   int       part=0;
   int       all=0;

   accno=cmd_getaccno(&ptr, NULL);
   str=next_token(&ptr, CMD_DELIM);
   if (str != NULL)
   {  if (strcmp(str, "all") == 0) all=1;
      else part=strtol(str, NULL, 0);
   }
   recs=log_reccount(&Logbase);
   if (recs<0) return cmd_out(ERR_IOERROR, NULL);

   for (i=recs-1; i>=0; i--)
   {  rc=log_get(&Logbase, i, &logrec);
      if (rc == IO_ERROR) return cmd_out(ERR_IOERROR, NULL);
      if (accno >= 0 && accno != logrec.accno) continue;
      if (all == 0)
      {  line+=1;
         if (line/20 != part) continue;
      }

      switch(logrec.errno)
      {  case NEGATIVE:
           result="S-";
           break;
         case SUCCESS:
           result="S+";
           break;
         case NOT_FOUND:
           result="NF";
           break;
         case ACC_DELETED:
           result="EM";
           break;
         case IO_ERROR:
           result="IO";
           break;
         case ACC_BROKEN:
           result="BR";
           break;
         case ACC_FROZEN:
           result="FR";
           break;
         case ACC_OFF:
           result="OF";
           break;
         case ACC_UNLIMIT:
           result="UN";
           break;
         default:
           result="??";
      }
      cmd_ptime(logrec.time, tbuf);
// out t/u 00000 (00000)
      proto =(logrec.isdata.proto_id & 0x00ff0000) >> 16;
      rport =logrec.isdata.proto_id & 0xffff;
      lport =logrec.isdata.proto2   & 0xffff;
      switch(proto)
      {  case 1:
           sprintf(pbuf, "icmp            ");
           break;
         case 6:
           sprintf(pbuf, "tcp ");
         case 17:
           if (proto == 17) sprintf(pbuf, "udp ");
           break;
         case 0:
           if (proto == 0)
           {  if (rport == 0 && lport == 0)
              {  sprintf(pbuf, "any             ");
                 break;
              }
              else sprintf(pbuf, "t/u ");
           }
           if (rport != 0) sprintf(pbuf+4, "%5d ", rport);
           else sprintf(pbuf+4, "any   ");
           if (lport != 0) sprintf(pbuf+10, "%5d ", lport);
           else sprintf(pbuf+10, "      ");
           break;
         default:
           sprintf(pbuf,"ip %3d          ", proto);
      }
      if (logrec.balance == BALANCE_NA) 
         snprintf(bbuf, sizeof(bbuf), "    N/A   ");
      else
         snprintf(bbuf, sizeof(bbuf), "%+10.2f", logrec.balance);

      cmd_out(RET_COMMENT, "%s %s #%-4d %+8.2f %6s %s %10d %s",
                result, 
                tbuf,
                logrec.accno, 
//                bbuf,
                logrec.sum,
                resource[logrec.isdata.res_id].name,
                ((logrec.isdata.proto_id & 0x80000000)==0) ?  " in":"out",
                logrec.isdata.value,
                pbuf);
//      rc=cmd_out(RET_COMMENT,"       (host %s)",
//                inet_ntoa(logrec.isdata.host));
      if (rc<0) return rc;
   }
   return cmd_out(RET_SUCCESS, NULL);
}

int cmd_ptime(time_t utc, char * buf)
{  struct tm stm;
   
   localtime_r(&utc, &stm);
// 00-00-00 00:00.00
// 01234567890123456
   sprintf(buf, "%02d-%02d %02d:%02d",
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
   rc=cmd_out(RET_SUCCESS, "Done.");
   if (rc < 0) return rc; 
   return 0;
}

int cmdh_human (char * cmd, char * args)
{  int rc;

   if (strcmp(cmd, "human") == 0)  
   {  HumanRead=1;
      MachineRead=0;
   }
   else 
   {  MachineRead=1;
      HumanRead=0;
   }
   rc=cmd_out(RET_SUCCESS, "Mode set");
   if (rc < 0) return rc; 
   return 0;
}


// lookup acc       by resource
// lookup accs      by name/addr
// lookup resources by acc

int cmdh_lookup(char * cmd, char * args)
{  char *   ptr=args;
//   char *   str;
   int      accno;
   lookup_t prev;
   int      ind;

   accno=cmd_getaccno(&ptr, &prev);
   if (accno == (-1)) return cmd_out(ERR_ARGCOUNT, NULL);
   do
   {  ind=-1;
      while(lookup_accno(accno, &ind) >= 0)
      {  cmd_out(RET_TEXT, "%d\t%s\t%s", accno,
            resource[linktab[ind].res_id].name,
            linktab[ind].username);
      }
      accno=cmd_getaccno(NULL, &prev);
   } while (accno >= 0);   
   return cmd_out(RET_SUCCESS, "");
}

int cmdh_date(char * cmd, char * args)
{  char   buf[16];
   time_t tim=time(NULL);
   
   cmd_out(RET_INT, "%d", tim);
   cmd_ptime(tim, buf);
   return cmd_out(RET_SUCCESS, "%s", buf);
}


int cmdh_report(char * cmd, char * args)
{
   logrec_t * rep=NULL;         // report array
   int        repc=0;           // report records count
   time_t     tfrom=0;          // time from (default - origin)
   time_t     tto=time(NULL);   // time to (default - current)
   int        accno=-1;		// account number
   int        tstep=0x7fffffff; // default - summary
   int        logrecs;		// no of log records
   char     * ptr=args;
   char     * str;
   int        i,j;
   int        wait4=0;
/*
   0 - token
   1 - step value
*/		
   logrec_t   logrec;
   void     * tmp;
   int        rc;   
   int        flag;

//report <acc> [from <time>] [to <time>] [step <hours>] [<flags>]

   accno=cmd_getaccno(&ptr, NULL);
   if (accno==-1) return cmd_out(ERR_ARGCOUNT, NULL);
   
   while((str=next_token(&ptr, CMD_DELIM)) != NULL)
   {  switch(wait4)
      {  case 0:
           if (!strcmp(str, "from"))
           {  tfrom=cmd_gettime(&ptr, 0);
              if (tfrom==-1) return cmd_out(ERR_INVARG, NULL);   
              wait4=0;
              break;
           }
           if (!strcmp(str, "to"))
           {  tto=cmd_gettime(&ptr, 0);
              if (tto==-1) return cmd_out(ERR_INVARG, NULL);   
              wait4=0;
              break;
           }
           if (!strcmp(str, "step"))
           {  wait4=1;
              break;
           }
           break;
         case 1:
           tstep=strtol(str, NULL, 0) * 3600;
           wait4=0;
           break;
         default:
          wait4=0;
           break;
      }
   }
   logrecs=log_reccount(&Logbase);   


   for (i=0; i<logrecs; i++)
   {  rc=log_get(&Logbase, i, &logrec);
      if (rc < 0)
      {  if (rep != NULL) free(rep);
         return cmd_out(ERR_IOERROR, NULL);
      }
      if (logrec.accno != accno) continue;
      if (logrec.time<tfrom || logrec.time>tto) continue;
      flag=1;
      for (j=repc-1; j>=0; j--)
      {  if (logrec.time - rep[j].time > tstep) break;
         if (rep[j].accno==logrec.accno &&
             rep[j].errno==logrec.errno &&
             rep[j].isdata.res_id==logrec.isdata.res_id &&
             rep[j].isdata.user_id==logrec.isdata.user_id &&
             rep[j].isdata.proto_id==logrec.isdata.proto_id &&
             rep[j].isdata.proto2==logrec.isdata.proto2 &&
             rep[j].isdata.host.s_addr==logrec.isdata.host.s_addr)
         {  rep[j].isdata.value += logrec.isdata.value;
            rep[j].sum += logrec.sum;
            rep[j].balance = BALANCE_NA;
            flag=0;
            break; 
         }
      }
      if (flag)
      {  tmp=realloc(rep, sizeof(logrec_t)*(repc+1));
         if (tmp == NULL)
         {  syslog(LOG_ERR, "report(realloc()): %m");
            if (rep!=NULL) free(rep);
            return cmd_out(ERR_SYSTEM, NULL);
         }
         rep=(logrec_t*)tmp;
         rep[repc++]=logrec;
      }
   }
   for (i=0; i<repc; i++)
   {  if ((rc=cmd_plogrec(rep+i))<0) return rc; 
   }
   if (rep != NULL) free(rep);
   return cmd_out(RET_SUCCESS, NULL);
}


time_t cmd_gettime(char ** arg, time_t def)
{  struct tm stm;
   time_t    ctimet=def ? def : time(NULL);
   char *    ptr=*arg;
   char *    ptr2;
   char *    str;
   int       sign;
   time_t    disp;
   int       val1;

// {{+}|{-} <H>:<M>}|{{[<D>/<M>[/<YY>]]} {<H>:<M>}} 
   str=next_token(&ptr, " \t");
   if (str==NULL) return -1;
   if (!strcmp(str, "+") || !strcmp(str, "-"))
   {  sign=*str=='+' ? 1 : -1;
      str=next_token(&ptr, " \t");
      if (str==NULL) return -1;
      ptr2=str;
      str=next_token(&ptr2, ":");
      if (str==NULL) return -1;
      disp=strtol(str, NULL, 0) * 3600;
      str=next_token(&ptr2, ":");
      if (str==NULL) return -1;
      disp+=strtol(str, NULL, 0) * 60;
      disp *= sign;
      *arg=ptr;
      return ctimet+disp;
   }      
   localtime_r(&ctimet, &stm);
   val1=strtol(str, &ptr2, 0);
   str=ptr2+1;
   if (*ptr2 == ':')
   {  stm.tm_min=strtol(str, NULL, 0);
      stm.tm_sec=0;
      stm.tm_hour=val1;
      *arg=ptr;
      return mktime(&stm);
   }
   stm.tm_mon=strtol(str, &ptr2, 0)-1;
   stm.tm_mday=val1;
   if (*ptr2 != '\0')
   {  val1=strtol(ptr2+1, NULL, 0);
      if (val1 > 1900) val1 -= 1900;
      stm.tm_year=val1;
   }
   str=next_token(&ptr, " \t");
   if (str==NULL) return (-1);
   stm.tm_hour=strtol(str, &ptr2, 0);
   stm.tm_min=strtol(ptr2+1, NULL, 0);
   stm.tm_sec=0;
   *arg=ptr;
   return mktime(&stm);
}


int cmd_plogrec(logrec_t * logrec)
{
   char * result;
   char      tbuf[20];
   char      pbuf[64];
   char      bbuf[16];
   int       rport, lport, proto;
   int       rc;

   switch(logrec->errno)
   {  case NEGATIVE:
        result="S-";
        break;
      case SUCCESS:
        result="S+";
        break;
      case NOT_FOUND:
        result="NF";
        break;
      case ACC_DELETED:
        result="EM";
        break;
      case IO_ERROR:
        result="IO";
        break;
      case ACC_BROKEN:
        result="BR";
        break;
      case ACC_FROZEN:
        result="FR";
        break;
      case ACC_OFF:
        result="OF";
        break;
      case ACC_UNLIMIT:
        result="UN";
        break;
      default:
        result="??";
   }
   cmd_ptime(logrec->time, tbuf);
   proto =(logrec->isdata.proto_id & 0x00ff0000) >> 16;
   rport =logrec->isdata.proto_id & 0xffff;
   lport =logrec->isdata.proto2   & 0xffff;
   switch(proto)
   {  case 1:
        sprintf(pbuf, "icmp            ");
        break;
      case 6:
        sprintf(pbuf, "tcp ");
      case 17:
        if (proto == 17) sprintf(pbuf, "udp ");
        break;
      case 0:
        if (proto == 0)
        {  if (rport == 0 && lport == 0)
           {  sprintf(pbuf, "any             ");
              break;
           }
           else sprintf(pbuf, "t/u ");
        }
        if (rport != 0) sprintf(pbuf+4, "%5d ", rport);
        else sprintf(pbuf+4, "any   ");
        if (lport != 0) sprintf(pbuf+10, "%5d ", lport);
        else sprintf(pbuf+10, "      ");
        break;
      default:
        sprintf(pbuf,"ip %3d          ", proto);
   }
   if (logrec->balance == BALANCE_NA) 
      snprintf(bbuf, sizeof(bbuf), "    N/A   ");
   else
      snprintf(bbuf, sizeof(bbuf), "%+10.2f", logrec->balance);
   rc=cmd_out(RET_COMMENT, "%s %s #%-4d %+8.2f %6s %s %10d %s",
                result, 
                tbuf,
                logrec->accno, 
                logrec->sum,
                resource[logrec->isdata.res_id].name,
                ((logrec->isdata.proto_id & 0x80000000)==0) ?  " in":"out",
                logrec->isdata.value,
                pbuf);
   return rc;
}

int cmdh_delete(char * cmd, char * args)
{
   char * ptr=args;
   int    accno;
   acc_t  acc;
   int    rc;

   accno=cmd_getaccno(&ptr, NULL);
   if (accno != (-1))
   {  rc=acc_baselock(&Accbase);
      if (rc != SUCCESS) return cmd_out(ERR_IOERROR, NULL);
      rc=acci_get(&Accbase, accno, &acc);
      if (rc==IO_ERROR || rc==NOT_FOUND) acc_baseunlock(&Accbase);
      if (rc == IO_ERROR) return cmd_out(ERR_IOERROR, NULL);
      if (rc == NOT_FOUND) return cmd_out(ERR_NOACC, NULL);
      acc.tag |= ATAG_DELETED;
      rc=acci_put(&Accbase, accno, &acc);
      acc_baseunlock(&Accbase);
      if (rc <= 0) return cmd_out(RET_SUCCESS, NULL);
      return cmd_out(ERR_IOERROR, NULL);
   }
   return cmd_out(ERR_INVARG, NULL);
}

int cmdh_new_contract(char * cmd, char * args)
{  int      rc;
   acc_t    acc;
   int      acc_inet, acc_intra;

   memset(&acc, 0, sizeof(acc));
   acc.balance=0;
   rc=acc_add(&Accbase, &acc);
   if (rc < 0) return cmd_out(ERR_IOERROR, NULL);
   acc_inet=rc;  
   acc.balance=1;
   rc=acc_add(&Accbase, &acc);
   if (rc < 0) return cmd_out(ERR_IOERROR, NULL);
   acc_intra=rc;
   cmd_out(RET_COMMENT, "Inet account: %d", acc_inet);
   cmd_out(RET_INT, "%d", acc_inet);
   cmd_out(RET_COMMENT, "Intranet account: %d", acc_intra);
   cmd_out(RET_INT, "%d", acc_intra);

   return cmd_out(RET_SUCCESS, NULL);
}

int cmdh_new_name(char * cmd, char * args)
{  char * str;
   char * ptr=args;
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
   str=next_token(&ptr, CMD_DELIM);
   if (str == NULL) return cmd_out(ERR_ARGCOUNT, "Arguments needed");
   name=str;
// Get account numbers
   str=next_token(&ptr, CMD_DELIM);
   if (str == NULL) return cmd_out(ERR_ARGCOUNT, "Account number needed");
   acc_inet=strtol(str, NULL, 0);
   str=next_token(&ptr, CMD_DELIM);
   if (str == NULL) return cmd_out(ERR_ARGCOUNT, "Intranet account needed");
   acc_intra=strtol(str, NULL, 0);
// Check account exists
   accs=acc_reccount(&Accbase);
   if (accs < 0) return cmd_out(ERR_IOERROR, NULL);
   if (acc_inet<0 || acc_intra<0 || acc_inet>=accs || acc_intra>=accs)
      return cmd_out(ERR_INVARG, "Invalid account given");
   rc=acc_get(&Accbase, acc_inet, &test);
   if (rc == IO_ERROR) return cmd_out(ERR_IOERROR, NULL);
   if (rc == ACC_DELETED || rc == NOT_FOUND) 
      return cmd_out(ERR_INVARG, "Given account (inet) is deleted"); 
   rc=acc_get(&Accbase, acc_intra, &test);
   if (rc == IO_ERROR) return cmd_out(ERR_IOERROR, NULL);
   if (rc == ACC_DELETED || rc == NOT_FOUND) 
      return cmd_out(ERR_INVARG, "Given account (intra) is deleted"); 
// Strip username
   ptr=name;
   name=next_token(&ptr, "@");
   if (name == NULL) return cmd_out(ERR_INVARG, "No username given");
// Check name (len <= 8, letters, digits)
   len=strlen(name);
   if (len<2 || len>8) return cmd_out(ERR_INVARG, "Invalid name lenght");
   for (i=0; i<len; i++)
   {  c=(unsigned char)name[i];  
      if ((c<'0' || c>'z') || (c>'9' && c<'a'))
         return cmd_out(ERR_INVARG, "Invalid chars in name");
   }
// Check hostname
   if (ptr==NULL) return cmd_out(ERR_INVARG, "No hostname given");
   if (strcmp(ptr, "home.oganer.net") != 0)
      return cmd_out(ERR_INVARG, "Inallowed hostname");
   host=ptr;
// Add login
   snprintf(buf, sizeof(buf), "/usr/local/bin/newlogin.sh %s %s", 
               name, ptr);
   fd=popen(buf, "r");
   if (fd==NULL) return cmd_out(ERR_IOERROR, NULL); 
   if (fgets(buf, sizeof(buf), fd)==NULL)
      return cmd_out(ERR_IOERROR, NULL);
   ptr=buf;
   str=next_token(&ptr, " \t");
   if (str==NULL) return cmd_out(ERR_IOERROR, NULL);
   if (strcmp(str, "000")!=0)
     return cmd_out(ERR_IOERROR, "%s", ptr);
   str=next_token(&ptr, " \t\n");
   if (str==NULL) return cmd_out(ERR_IOERROR, "Unexpected script error");
   cmd_out(RET_COMMENT, "User password: %s", str);
   cmd_out(RET_STR, "%s", str);
// Add adder resources
   if ((lockfd=reslinks_lock(LOCK_EX))!=-1)
   {  reslinks_load(LOCK_UN);
      reslink_new(RES_ADDER, acc_inet, name);
      reslink_new(RES_ADDER, acc_intra, name);      
// Add mail resources
      snprintf(buf, sizeof(buf), "%s@%s", name, host);
      reslink_new(RES_MAIL, acc_inet, buf);
      reslinks_save(LOCK_UN);
      reslinks_unlock(lockfd);
   }
   else syslog(LOG_ERR, "cmdh_new_name(): Unable to lock reslinks");

   return cmd_out(SUCCESS, NULL);  
} 

/*
int cmdh_gate(char * cmd, char * args)
{  char * ptr=args;
   char * str; 
// addgate <res> <acc_id> <name>
   
   str=next_token(&ptr, CMD_DELIM);
   if (str==NULL) return cmd_out()
}
*/
