// File Shell
// Main file

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <syslog.h> 
#include <unistd.h>

#include <bee.h>
#include <db.h>

#include "global.h"
#include "login.h"
#include "log.h"


char	*FramesFile = NULL;
char	*FramesEntry = NULL;
char 	* Chars = NULL;

 
int	fKeyBar=1;
int	fMenuBar=1;
int	nCmdLines=2;
int	fMinorUpdate;
int	fMajorUpdate;
int	fCharSetLoaded = 0;

char    LoginBuf[17]="login";

int     AccessLevel = 0;

void	KeyDebug();
char   buffer[128]; // debug

int RefreshConsole()
{  
   Attr(7, 0);
   erase();
   RefreshTitle();
   RefreshKeybar();
   Attr(7, 0);
   Gotoxy(2,0); 
   return 0;
}


int	main(int argc, char ** argv)
{
   int	ret = RET_DONE;
   int  ch;

   int  rc;

   int  wflag = 0;


// Initializations

   while ((ch = getopt(argc, argv, "mc:C:t:l:w")) != -1)
   	switch (ch) {

		case 'm':	
			fMono = 1;
			break;

		case 'c':
			FramesEntry = optarg;
			break;

		case 'C':
			FramesFile = optarg;
			break;
                case 't':

for (int i=32;i<255;i++)
{
  printf("%c ",i);
  if (i%16 == 15) printf("\n %x ", (i+1)/16);
}
return 0;             
                case 'l':
			AccListFile = optarg;
                        break;

                case 'w':
			wflag = 1;
                        break;

		default: 
			exit(-1);
	}
   
   argc -= optind;
   argv += optind;

   rc = conf_load(NULL);
   if (rc < 0)
   {  fprintf(stderr, "ERROR: Unable to load configuration");
      exit(-1);
   }

   if (!FramesEntry) FramesEntry = DEFAULT_FRAMES_ENTRY;
   if (!FramesFile)  FramesFile  = conf_paymanframes;
   if (!(Chars = MakeFrames(FramesFile, FramesEntry)))
		exit(-1);

   if (! InIt()) 			// Program init
   {  printf("Can't open screen device");
      return (-1);
   }

// Open transaction log
   rc = log_open();
   if (rc < 0)
   {  syslog(LOG_ERR, "FATAL: no logging, terminating");
      OutIt();
      exit(-1);
   }
   
   log_write("session from \"%s\" user %s logname %s", getenv("SSH_CLIENT"),
             getenv("USER"), getenv("LOGNAME"));

   OpenShell();  // Switch terminal mode             

// Force standard size
   ScreenLines   = ForceLins;
   ScreenColumns = ForceCols; 

   RefreshConsole();

   while(1)
   {  uprintf("Загрузка списка счетов ... ");
      refresh();
      if (AccListFile == NULL) rc =  UserList.load_accs_bee();
      else rc =  UserList.load_accs(AccListFile);
      if (rc >= 0) break;
      sleep(1);
      rc = MessageBox("Ошибка\0", 
                      " Не удается загрузить список счетов, повторить ? \0",
                      MB_YESNO | MB_NEUTRAL);
      if (rc == ID_NO)
      {  CloseShell();
         OutIt();
         return (-1);
      }
      RefreshConsole();
   }
   uprintf("готово (%d счетов)\n", UserList.cnt_accs);

   if (wflag)
   {  uprintf("Апдейт базы (может занять некоторое время) ... ");
      refresh();
      UserList.update_accs_bee();
      uprintf("готово.\n");
   }


   while(1)
   {  uprintf("Загрузка списка пользователей ... ");
      refresh();
      rc =  UserList.load_list();
      if (rc >= 0) break;
      sleep(1);
      rc = MessageBox("Ошибка\0", 
                      " Не удается загрузить список, повторить ? \0",
                      MB_YESNO | MB_NEUTRAL);
      if (rc == ID_NO)
      {  CloseShell();
         OutIt();
         return (-1);
      }
      RefreshConsole();
   }
   uprintf("готово.\n");


   while(1)
   {  uprintf("Загрузка профилей ... ");
      refresh();
      rc =  logins_load();
      if (rc >= 0) break;
      sleep(1);
      rc = MessageBox("Ошибка\0", 
                      " Не удается загрузить список, повторить ? \0",
                      MB_YESNO | MB_NEUTRAL);
      if (rc == ID_NO)
      {  CloseShell();
         OutIt();
         return (-1);
      }
      RefreshConsole();
   }
   uprintf("готово.\n");

// Sort userlist
   UserList.sort_regname();

   UserList.lin  = 4;
   UserList.col  = 14;
   UserList.lins = 15;
   UserList.cols = 50;
   UserList.initview();

   keymode = 1;

   DoRefresh = 1;

   do
   {  
      Update();			             // Update screen

      if (ret != RET_REDO) WaitEvent();	     // Wait Events

      ret = DispEvent();                     // Event dispatcher

   } while (ret == RET_DONE || ret == RET_REDO); 

   CloseShell();    // Switch to original mode

   log_write("end session");

   log_close();

   OutIt();

   return 0;
}


int	Arrange()
{  int	firstlin=0;
   int	lastlin=ScreenLines-1;
   int  firstcol=0;
   int	lastcol=ScreenColumns-1;

//   Rules
// 1. KeyBar gets bottom line (if enabled)
// 2. Menu gets top line (if enabled)
// 3. Cmd View gets specificied number of lines on bottom
// 4. Other screen splits between panels

   register AREA * ptr;

   ptr=&Area[AI_KEYBAR];
   if (fKeyBar)
   {  ptr->lin=lastlin;
      ptr->col=0;
      ptr->lins=1;
      ptr->cols=ScreenColumns;
      lastlin--;
   } else ptr->lins=0;

   ptr=&Area[AI_MENUBAR];
   if (fMenuBar)
   {  ptr->lin=firstlin;
      ptr->col=0;
      ptr->lins=1;
      ptr->cols=ScreenColumns;
      firstlin++;
   } else ptr->lins=0;

   ptr=&Area[AI_CMDVIEW];
   ptr->lin=lastlin-nCmdLines+1;
   ptr->col=0;
   ptr->lins=nCmdLines;
   ptr->cols=ScreenColumns;
   lastlin-=nCmdLines;

   ptr=&Area[AI_LPANEL];
   ptr->lin=firstlin;
   ptr->col=0;
   ptr->lins=lastlin-firstlin+1;
   ptr->cols=ScreenColumns/2;
   firstcol+=ptr->cols;

   ptr=&Area[AI_RPANEL];
   ptr->lin=firstlin;
   ptr->col=firstcol;
   ptr->lins=lastlin-firstlin+1;
   ptr->cols=lastcol-firstcol+1;

   fMajorUpdate=1;
   return 0;
}

SCHEME  ColorPullDnScheme=
{  0x60,
   0x60,
   {0x60,0x60},
   {0x60,0x60},
   {0x68,0x60,0x47},
   {0x60,0x60,0x60}
};
SCHEME ColorMenusScheme=
{  0x70,
   0x70,
   {0x70,0x70},
   {0x70,0x70},
   {0x78,0x70,0x47},
   {0x70,0x70,0x70}
};
SCHEME ColorNeutralScheme=
{  0x70,
   0x70,
   {0x78,0x70},
   {0x07,0x07},
   {0x78,0x70,0x47},
   {0x78,0x70,0x47}
};

SCHEME * ColorSchemes[]=
{  0,                         //(SCHEME *)(&ColorShellScheme),
   &ColorPullDnScheme,
   &ColorMenusScheme,
   &ColorNeutralScheme
};

SCHEME  **Schemes=ColorSchemes;
