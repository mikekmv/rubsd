// File Shell
// Main file

#include <unistd.h>
#include "global.h"

char	*FramesFile = NULL;
char	*FramesEntry = NULL;
char 	* Chars = NULL;

 
int	fKeyBar=1;
int	fMenuBar=1;
int	nCmdLines=2;
int	fMinorUpdate;
int	fMajorUpdate;
int	fCharSetLoaded = 0;

void	KeyDebug();

int	main(int argc, char ** argv)
{
   int	ret=RET_DONE;
   int  ch;

// Initializations

   while ((ch = getopt(argc, argv, "mc:C:")) != -1)
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

		default: 
			exit(-1);
	}
   
   argc -= optind;
   argv += optind;

   if (!FramesEntry) FramesEntry = DEFAULT_FRAMES_ENTRY;
   if (!FramesFile)  FramesFile  = DEFAULT_FRAMES_FILE;
   if (!(Chars = MakeFrames(FramesFile, FramesEntry)))
		exit(-1);

   if (! InIt()) 			// Program init
   {  printf("Can't open screen device");
      return (-1);
   }

   do
   {  WINOUT ww;
      OpenShell();			// Switch to Shell (store term)
      Gotoxy(0, 0); Putch('*');
      Gotoxy(ScreenLines-1, 0); Putch('*');
      Gotoxy(0, ScreenColumns-1); Putch('*');
      Gotoxy(ScreenLines-1, ScreenColumns-1); Putch('*');
      ww.lin  = 1;
      ww.col  = 1;
      ww.lins = ScreenLines-2;
      ww.cols = ScreenColumns-2;
      ww.at(0,0);
      Paper(0);
      Ink(7);
      ww.fill(FT_DOUBLE);

      MessageBox("Title", "Message box text\ntext\0", MB_NEUTRAL);       

      Paper(0);
      Ink(7);
      ww.fill(FT_DOUBLE);
           
      GetKey();   

       do
       {   
           break;
           Arrange();			// Divide screen beetwing areas
           do 	// Internal Event cycle
           {   
	       Update();			     // Update screen
	       if (ret!=RET_REDO) WaitEvent();	     // Wait Events
	       ret=DispEvent();                      // Event dispatcher
// DispEvent return value (ret):
//  RET_DONE   - Continue Cycle
//  RET_REDO   - Repeat Dispatch (new event)
//  RET_REDRAW - (not used)
//  RET_EXIT   - Exit programm requested
//  RET_EXEC   - Execute command line (Cmd.Line)
//  RET_TERM   - Rearrange screen (term sizes changed)
           } while (ret==RET_DONE || ret==RET_REDO); 
       } while (ret == RET_TERM);
       CloseShell();				// Switch to Term (restore)
       if (ret == RET_EXEC)
//     {  system(Cmd.Line);    // primitive version
          getch();
//     }
   } while (ret == RET_EXEC);
    OutIt();
    delete Chars; 	
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
   {0x68,0x60},
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
