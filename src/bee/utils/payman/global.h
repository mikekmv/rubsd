// File shell
// Global include file

///////////////
#define UNIX
///////////////
//  Defines  //
///////////////

#include "keys.h"

#define ForceLins  24
#define ForceCols  80


 // Area Indexes
#define	AI_CMDVIEW	0
#define AI_LPANEL	1
#define AI_RPANEL	2
#define AI_KEYBAR	3
#define	AI_MENUBAR	4

 // Limits
#define LIMIT_CMD	512
#define LIMIT_HISTORY	10
#define LIMIT_PROMPT	40

// Defaults
#define DEFAULT_HOST		"localhost"
#define DEFAULT_USER		"nologin"
#define DEFAULT_DIR		"..."
#define DEFAULT_PROMPT_FMT	"[%s@%s %s"
#define DEFAULT_FRAMES_ENTRY	"bsd"
#define DEFAULT_FRAMES_FILE	".framesrc"

 // AREA flagbits
#define	OF_MINORUPD	1
#define OF_MAJORUPD     2
#define OF_EVENTS       4
#define OF_MOUSE        8

 // AREA Actions
#define OA_MAJORUPD	1
#define OA_MINORUPD	2
#define OA_DISPEVENT	3
#define OA_DISPMOUSE	4
#define OA_PUTCURSOR	5

 // DispEvent return values
#define	RET_DONE	0
#define	RET_TERM	1
#define	RET_EXIT	2
#define	RET_EXEC	3
#define	RET_CONT	4
#define RET_REDO	5
#define RET_DEFEXIT	6
#define RET_REDRAW	7
 // Frame (fill) types 
#define FT_DOUBLE	0
#define FT_SINGLE	1
#define FT_NOFRAME	2

 // General Even types
#define EV_USER		1
#define EV_TERM		2   /* ???? */

 // Menu dialog params
#define MP_DRAWONLY	0xffff

 // Color Scheme indexes
#define SI_SHELL	0
#define SI_PULLDN	1
#define SI_MENUS	2
#define SI_NEUTRAL	3
#define SI_WARNING	4
#define SI_ERROR	5
#define SI_INFO		6

 // Message Box types
#define MB_OK		0
#define MB_OKCANCEL	1
#define MB_YESNO	2
#define MB_YESNOCANCEL	3
#define MB_DEF1		0
#define MB_DEF2		4
#define MB_DEF3		8
#define MB_NEUTRAL	0x30
#define MB_WARNING	0x40
#define MB_ERROR	0x50
#define MB_INFO		0x60
 // MB masks
#define MBM_TYPE	3
#define MBM_DEF		12
#define MBM_SCHEME	0x70
 // MB shift
#define MBS_TYPE	0
#define MBS_DEF		2
#define MBS_SCHEME	4

///////////////////////////////////
//           Macro               //
///////////////////////////////////
#define keyShort (*((*unsigned short)&keyLong))
////////////////
//  Includes  //
////////////////
#include "unicon.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef UNIX 
#include <ncurses.h>
#endif

#include "list.h"

////////////////
//    Types   //
////////////////

typedef unsigned long ulong;
typedef	unsigned char uchar;

class winTEMPL   // fictive class (for WINOUT New();)
{  public:
   int	lin;
   int  col;
   int  lins;
   int  cols;
};

class	AREA
{  public:
   int	lin;
   int	col;
   int	lins;
   int	cols;

   ulong flags;
   void * inst;

   int	(*proc)(AREA * th, int action, ulong param);
};

class	WINOUT   
{  public:
   int	lin;
   int	col;
   int	lins;
   int	cols;
   int	wlin;
   int	wcol;
   int  fAt;

   void   New	(void * templ)
       {  *((winTEMPL*)this)=*((winTEMPL*)templ);  
          at(0,0);
       }
   void   RelNew(void * templ)
       {  this->lin+=((winTEMPL*)templ)->lin;
          this->col+=((winTEMPL*)templ)->col;
	  this->lins=((winTEMPL*)templ)->lins;
	  this->cols=((winTEMPL*)templ)->cols;
          at(0,0);
       }
   void	  at	(int l=(-1), int c=(-1));
   void	  pc	(char c);
   char  *ps	(char * str, int n=0, int fField=0, int fSzz=0, uchar hattr=0);
   char  *pszz	(char * str, int n=0, int fField=0, uchar hattr=0)
   { return ps(str,n,fField,1,hattr); }
   char  *psfd	(char * str, int n)
   { return ps(str,n,1); }
   void	  fill (int type=FT_DOUBLE);
};

class	CMDCLASS
{  public:
   char	Line[LIMIT_CMD];
   int	len;
   int	pos;
   int	startView;
   int	widthView;
   int	fInsert;
   int	cursl;
   int	cursc;
};

#include "dialog.h"

////////////////////////////////////////
//            Static data             //
////////////////////////////////////////

extern  AREA			Area[];

extern  int			ScreenLines;
extern  int			ScreenColumns;

extern	CMDCLASS		Cmd;
extern	char			Prompt[];
extern	int			lenPrompt;

extern	WINOUT			Defs;

 // Shell environment variables
extern	int			fKeyBar;
extern	int			fMenuBar;
extern	int			nCmdLines;
extern  int			fMinorUpdate;
extern	int			fMajorUpdate;
extern	int			fMores;
extern	int			lastfMores;
extern  char		      *	Chars;

extern  int			fMono;

extern  SCHEME		     ** Schemes;
extern  SCHEME		      * ClassicSchemes[];
extern  SCHEME		      * MonoSchemes[];

 // Event structure
extern	int			eventType;
extern	ulong			keyLong;

 // Menu data
extern CONTROL			LeftMenuControls[];
extern CONTROL			RightMenuControls[];
extern CONTROL			OptionsMenuControls[];
extern CONTROL			FileMenuControls[];

 // (Payman related)
extern int                      DoRefresh;
extern int                      keymode;
extern int                      StageScr;

extern char *                   months_cased[];
extern char *                   months[];

// Inline 
int inline Dialog(DIALOG * th, ulong param)
{  return th->Dialog(param); }

// Prototypes

int	PanelProc	(AREA * th, int action, ulong param);
int	CmdProc		(AREA * th, int action, ulong param);
int	KeyProc		(AREA * th, int action, ulong param);
int	MenuProc	(AREA * th, int action, ulong param);

int 	InIt		();
void	OutIt		();
void	OpenShell	();
int	Arrange		();
void	CloseShell	();
int	ShPass		(char * str);

int	Update		();
int	WaitEvent	();
int	DispEvent	();

void	point		(char * str);
void	getprompt       (char * p);
char	*MakeFrames	(char *, char *);
int	ts		(char * str, void * templ=0, int fSzz=0);
int	MessageBox	(char * title, char * text, int type);

// New
int     RefreshTitle    ();
int     RefreshKeybar   ();
