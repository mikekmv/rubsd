// (console output & keyboard input)

#include <stdio.h>

typedef unsigned char uchar;

extern	int		ScreenLines;	// Screen sizs
extern	int		ScreenColumns;	// Screen size

int		ProbeUnicon	();

int		CreateUnicon	();
// Do init curses, return result

void		KillUnicon	();
// Do close

int		Color		();
// Do initialize color

void		GetSizes	();
void		Gotoxy		(int lin, int col);
int		Cursor		(int f);
int		Putch		(char	c);
int		Puts		(char * str);

int		Kbhit		();
unsigned int	GetKey 		();

int		Ink		(int c);               // set ink color 0-15
int		Paper		(int c);               // set paper color 0-7
int		Attr		(int ink, int paper);  // set attribute
uchar		bAttr		(uchar byte);       // set attribute from byte

int uprintf(const char * format, ...);
