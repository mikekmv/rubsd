// File Shell
// Command line view file
//
// functions:
//
// CmdProc    - Command viewport Area Procedure
// point      - DEBUG checkpoint procedure
//
// CmdUpd     - (CmdProc,CmdDisp) Do update (OA_MINORUPD | OA_MAJORUPD) or
//              check cursor in bounds of viewport (0).
// CmdDisp    - (CmdProc) Do dispatch key or return RET_CONT
// MakePrompt - (CmdUpd) check Prompt need to update. If so - update & return
//              non-zero, otherwise - return 0
// (getprompt) - (MakePrompt) compose prompt in buffer pointed by argument
//

#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/param.h>

#include "global.h"

CMDCLASS	Cmd; // ={"",0,0,0,0,1,0,0};

int		fMores=0;
int		lastfMores=1;
char		Prompt[LIMIT_PROMPT+1]="";
int		lenPrompt;
int		fMultyline=1;

int	CmdDisp		(AREA * th);
int	CmdUpd		(AREA * th, int action);
int 	MakePrompt	();

int	CmdProc(AREA * th, int action, ulong param)
{   switch (action)
    {   case OA_DISPEVENT:
	   return CmdDisp(th);
	case OA_MINORUPD:
	case OA_MAJORUPD:
           return CmdUpd(th,action);
	case OA_PUTCURSOR:
	   CMDCLASS * 	pcmd=(CMDCLASS *)th->inst;
	   Defs.New(th);
	   Defs.at(pcmd->cursl, pcmd->cursc);
	   return 0;
    } 
    return RET_CONT;
}

int	CmdDisp(AREA * th) 
{//  if (eventType!=EV_USER) return RET_CONT;

    CMDCLASS * 	pcmd=(CMDCLASS *)th->inst;       // 

    switch (keyLong & M_KEY)
    {   case	K_RTARROW: 
        case	K_CTRL_S: 
           if(pcmd->pos<pcmd->len) 
           {  pcmd->pos++; 
              CmdUpd(th,0);
           } return RET_DONE; 
        case	K_LTARROW: 
        case	K_CTRL_D: 
           if(pcmd->pos>0) 
           {   pcmd->pos--; 
               CmdUpd(th,0); 
           } return RET_DONE; 
          case	K_HOME: 
//        case	K_CTRL_HOME: 		// no such keystroke
           pcmd->pos=0; 
           CmdUpd(th,0); 
           return RET_DONE; 
        case	K_END: 
//        case	K_CTRL_END: 		// no such keystroke
           pcmd->pos=pcmd->len; 
           CmdUpd(th,0); 
           return RET_DONE; 
        case	K_BS: 
           if (pcmd->pos>0) 
           {   if (pcmd->pos<pcmd->len) 
               {   char * last =&pcmd->Line[pcmd->len]; 
                   char * first=&pcmd->Line[pcmd->pos]; 
                   for (char * i=first;i<last; i++) *(i-1)=*i; 
               } 
               pcmd->pos--; 
               pcmd->len--; 
               pcmd->Line[pcmd->len]='\0'; 
               CmdUpd(th,OF_MINORUPD); 
           } 
           return RET_DONE; 
        case	K_DEL: 
        case	K_CTRL_G: 
      	   if (pcmd->pos<pcmd->len) 
           {   if ((pcmd->pos+1)<pcmd->len) 
               {   char * last =&pcmd->Line[pcmd->len]; 
                   char * first=&pcmd->Line[pcmd->pos+1]; 
                   for (char * i=first;i<last; i++) *(i-1)=*i; 
               } 
               pcmd->len--; 
               pcmd->Line[pcmd->len]='\0'; 
               CmdUpd(th,OF_MINORUPD); 
           } 
           return RET_DONE; 
        case	K_CTRL_Y:	// delete line 
           pcmd->pos=0; 
           pcmd->len=0; 
           pcmd->Line[pcmd->len]='\0'; 
           CmdUpd(th,OF_MINORUPD); 
           return RET_DONE; 
        case	K_CTRL_K: 	// delete line after cursor
           pcmd->len=pcmd->pos; 
           pcmd->Line[pcmd->len]='\0'; 
           CmdUpd(th,OF_MINORUPD); 
           return RET_DONE; 
//        case	K_CTRL_LTARROW: 	// no such keystroke
        case	K_CTRL_A:
//    Set to previous word END 
// 1. Find space 
// 2. Find first non-space symbol 
           {   int i=pcmd->pos; 
               while (i>0 && pcmd->Line[--i]!=' '); 
               while (i>0 && pcmd->Line[--i]==' '); 
               pcmd->pos=i; 
               CmdUpd(th,0); 
               return RET_DONE; 
           } 
//        case	K_CTRL_RTARROW: 	// no such keystroke 
        case	K_CTRL_F: 
//    Set to next word BEGIN 
// 1. Find space 
// 2. Find first non-space symbol 
           {   int i=pcmd->pos; 
               while (i<pcmd->len && pcmd->Line[++i]!=' '); 
               while (i<pcmd->len && pcmd->Line[++i]==' '); 
               pcmd->pos=i; 
               CmdUpd(th,0); 
               return RET_DONE; 
           } 
        case	K_CTRL_BS: 
        case	K_CTRL_W: 
//   Delete left (or current) word 
//  1. Find left letter: 
//		a. findleft nospace (or BOL) 
//		b. findleft space (or BOL) 
//  2. Find right letter 
//		a. findright space (or EOL) 
//  3. Delete [right_letter;left_letter) 
// 
           {   int beg,end,len; 
               int i=pcmd->pos; 
               while (i>0 && pcmd->Line[--i]==' ');	// for BS policy 
               while (i>0 && pcmd->Line[--i]!=' '); 
               beg=i; 
               while (i<pcmd->len && pcmd->Line[++i]!=' '); 
               end=i; 
               len=end-beg; 
               if (end<pcmd->len) 
               {   char * last =&pcmd->Line[pcmd->len]; 
                   char * first=&pcmd->Line[end]; 
                   for(char * ii=first; ii<last; ii++) *(ii-len)=*ii; 
               }
               pcmd->pos=beg; 
               pcmd->len-=len; 
               pcmd->Line[pcmd->len]='\0'; 
               CmdUpd(th,OF_MINORUPD); 
               return RET_DONE; 
           } 
        case	K_CTRL_T: 
//   Delete right (or current) word 
//  1. Find right letter: 
//			a. findrigth space (or EOL) 
//  2. Find left letter 
//			a. findleft space (or BOL) 
//  3. Delete [right_letter;left_letter) 
// 
           {   int beg,end,len; 
               int i=pcmd->pos; 
               while (i<pcmd->len && pcmd->Line[++i]!=' '); 
               end=i; 
               while (i>0 && pcmd->Line[--i]!=' '); 
               beg=i; 
               len=end-beg; 
               if (end<pcmd->len) 
               {   char * last =&pcmd->Line[pcmd->len]; 
                   char * first=&pcmd->Line[end]; 
                   for(char * ii=first; ii<last; ii++) *(ii-len)=*ii; 
               } 
               pcmd->pos=beg; 
               pcmd->len-=len; 
               pcmd->Line[pcmd->len]='\0'; 
               CmdUpd(th,OF_MINORUPD); 
               return RET_DONE; 
            } 
         default: 
            if (keyLong>31 && keyLong<256) 
            {   if (pcmd->len<LIMIT_CMD) 
                {   // Insert/Replace modes
		    if (!pcmd->fInsert && pcmd->pos < pcmd->len)
                    {  char * last =&pcmd->Line[pcmd->len]; 
                       char * first=&pcmd->Line[pcmd->pos]; 
                       for (char * i=last;i>first; i--) *i=*(i-1); 
                    } 
                    pcmd->Line[pcmd->pos]=(char)keyLong; 
                    pcmd->pos++; 
                    pcmd->len++; 
                    pcmd->Line[pcmd->len]='\0'; 
                    CmdUpd(th,OF_MINORUPD); 
                    } 
                    return RET_DONE; 
                }
    }  // switch 
    fMinorUpdate=1;
    return RET_CONT; 
} 

int	CmdUpd(AREA * th, int action)
{ 
    CMDCLASS * pcmd=(CMDCLASS *)th->inst;                  //
    if (MakePrompt()) action=OA_MINORUPD;
    lenPrompt = strlen(Prompt);
    pcmd->widthView=(ScreenColumns*nCmdLines)-lenPrompt-(fMores?2:0);
    
    if (lastfMores!=fMores)
    {	action=OA_MINORUPD;
	lastfMores=fMores;
    }
    if (! fMultyline)
    {	if (pcmd->startView > pcmd->pos)
	{   action=OA_MINORUPD;
	    pcmd->startView = pcmd->pos;
	} else 
	  if (pcmd->startView + pcmd->widthView <= pcmd->pos)
	{   action=OA_MINORUPD;
	    pcmd->startView = pcmd->pos - pcmd->widthView + 1;
	}
    } else
    {	if (pcmd->widthView - pcmd->len - 1 >= ScreenColumns || 
            pcmd->len+1 > pcmd->widthView)
	{
	    register int temp=lenPrompt+pcmd->len+1+(fMores?2:0);
	    nCmdLines=(temp/ScreenColumns)+(temp%ScreenColumns>0);
	    Arrange();
	    return CmdUpd(th,OA_MAJORUPD);
	}
    }
    if (action)
    {   
        Defs.New(th);
        Attr(7,0);
	Defs.ps(Prompt);
	if (fMores) Defs.pc(pcmd->startView?'+':' ');
	Defs.psfd((char*)(pcmd->Line + pcmd->startView), pcmd->widthView);
	if (fMores)
	   Defs.pc((pcmd->len - pcmd->startView > pcmd->widthView)?'+':' ');
    }
    register int temp= (pcmd->pos - pcmd->startView + lenPrompt + (fMores?1:0));
    pcmd->cursl = temp / ScreenColumns;
    pcmd->cursc = temp % ScreenColumns;
    return 0;
}    
////////////////////////////////////////////////////////////////////////
// int MakePrompt()
//
//  This procedure _checks_ if shell prompt (char Prompt[]) needs to be
//  updated (i.e.: prompt formula or values of prompt parts was changed)
//  If so (and only if so), procedure have to replace string Prompt &
//  return non-zero.
//    Prompt need to have lenght less or equal LIMIT_PROMPT.
////////////////////////////////////////////////////////////////////////
int MakePrompt()
{   char  newprompt[LIMIT_PROMPT+1];
    int   rets;

    getprompt(newprompt);
    if ((rets=strcmp(Prompt, newprompt))) 
	strcpy(Prompt,newprompt);
    return rets;
} 

void	point(char * str)
{ Gotoxy(0,0); Puts(str); refresh();}

void	getprompt(char* p)
{  char    host[MAXHOSTNAMELEN];
   char    dir[MAXPATHLEN];
   char   *user;

   if (gethostname(host, MAXHOSTNAMELEN) == -1) 
      (void)strcpy(host, DEFAULT_HOST);

   if (!(user = getlogin())) user = DEFAULT_USER;

   if (!(getwd(dir))) (void)strcpy(dir, DEFAULT_DIR);

   snprintf(p, LIMIT_PROMPT - 3 , DEFAULT_PROMPT_FMT,
       user, host, dir);
   strcat(p, getuid() ? "]$ " : "]# ");
}
