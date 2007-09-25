/////////////////////////////////////////////////////
// Uniconsole library (Unix-version)
// (console output & keyboard input)
/////////////////////////////////////////////////////

#include <curses.h>
#include <stdlib.h>

#include "unicon.h"

#define K_ESC	27
#define K_ALT	0x200


int		ScreenLines;	// Screen size
int		ScreenColumns;	// Screen size
int		Pair;
int		bHigh;
int		fMono=0;
int		Keydump[8];

int ProbeUnicon()
{
    if (initscr() == 0) return 0;
    endwin();

    return 1;
}

int CreateUnicon()
{  WINDOW * th;

   th = initscr();
   noecho();
   raw();
   keypad(stdscr, TRUE);
   GetSizes();

   return (th != 0);
}

void KillUnicon()
{
   keypad(stdscr, FALSE);
   noraw();
   noecho();
   endwin();
}

int Color()
{   
   short  f, b;

   if (! has_colors()) 
   {  fMono = 1;
      return 0;
   }

   start_color();
   if (COLOR_PAIRS < 64 || COLORS < 8) 
   {  fMono = 1;
      return 0;
   }

   for (f=0; f<8; f++)
      for (b=0; b<8; b++) 
         init_pair(f+b*8, f, b);

   Pair  = 0;
   bHigh = 0;    
   return 1;
}

void		GetSizes()
{
   ScreenLines   = LINES;     //stdscr->_maxy+1;
   ScreenColumns = COLS;      //stdscr->_maxx+1;
}

void Gotoxy(int lin, int col)
{  
   move(lin, col);
}

int Cursor(int f)
{  
   return curs_set(f);  
}

int Putch (char c)
{  
   return addch((unsigned int)((unsigned char)c));
}

int Puts (char * str)
{
   return addstr(str);
}

int Kbhit()
{ 
   return 0; 
}


unsigned int GetKey()
{  
   return (unsigned int) getch();
}


int Ink (int c)
{  int attr;

   
   Pair = Pair & 0x38 | (c & 7);
   if (c & 8) bHigh = A_BOLD;
   else bHigh = 0;

   if (fMono)
   {  if ((c & 7) > ((Pair&38)>>3)) attr = A_NORMAL;
      else attr = A_REVERSE;
      return attrset(attr | bHigh);
   }

    return attrset(COLOR_PAIR(Pair) | bHigh);
}

int Paper (int c)
{  int attr;

   Pair = Pair & 7 | (c<<3 & 0x38);

   if (fMono)
   {  if ((Pair & 7) > c) attr = A_NORMAL;
      else attr = A_REVERSE;
      return attrset(attr | bHigh);
   }

    return attrset(COLOR_PAIR(Pair) | bHigh);
}

int	Attr (int ink, int paper)
{  int attr; 

    Pair = (ink & 7) | (paper<<3 & 0x38);
    if (ink & 8) bHigh = A_BOLD;
    else bHigh = 0;

    if (fMono)
    {  if ((ink & 7) > paper) attr = A_NORMAL;
       else attr = A_REVERSE;
       return attrset(attr | bHigh);
    }

    return attrset(COLOR_PAIR(Pair) | bHigh);
}
    
uchar 	bAttr (uchar byte)
{  
   int rets = (Pair&7) | ((Pair&0x48)<<1) | (bHigh ? 8:0); // restore attribute
   int attr; 

   Pair = (byte & 7) | ((byte & 0x70) >> 1);
   if (byte & 8) bHigh = A_BOLD;
   else bHigh = 0;

   if (fMono)
   {  if ((Pair & 7) > ((Pair&38)>>3)) attr = A_NORMAL;
      else attr = A_REVERSE;
      attrset(attr | bHigh);
   }
   else attrset(COLOR_PAIR(Pair) | bHigh);
   
   return rets;
}

int uprintf(const char * format, ...)
{  char    * p = NULL;
   va_list   ap;
   int       n;

   va_start(ap, format);
   n = vasprintf(&p, format, ap);
   va_end(ap);

   if (p != NULL)
   {  Puts(p);
      free(p);
   }
   
   return n;
}
