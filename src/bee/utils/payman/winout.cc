// File Shell
// Window output class file

#include "global.h"

#define CTRL_TAB	1
#define CTRL_AT		2

WINOUT	Defs;


void WINOUT::at(int l, int c)
{  wlin=(l>=0)?l:wlin;
   wcol=(c>=0)?c:wcol;
   if (wlin+lin>(-1) && wlin+lin<ScreenLines && 
       wcol+col>(-1) && wcol+col<ScreenColumns) 
   {  fAt=0;
      Gotoxy(wlin+lin,wcol+col);
   } else fAt=1;
}

void WINOUT::pc(char c)
{  
   switch (c)
   {   case '\n':
          wcol=cols;
          break;
       case  '\t':
	  do 
	  {  pc(' ');
	  } while(!(wcol&7));
          break;
       default:
          if (c>31 || c<0)
          {  if (wlin+lin>=0 && wlin+lin<ScreenLines && 
	         wcol+col>=0 && wcol+col<ScreenColumns) Putch(c);
             wcol++;
          }
   }
   if (wcol>=cols)
   {   wcol=0;
       wlin++;
       if (wlin>=lins)
       {   wlin=0;
       }
       fAt=1;
   }
   if (fAt) at();

}

char *  WINOUT::ps(char * str, int n, int fField, int fSzz, uchar hattr)
{   char * ptr=str?str:(char *)"";
    char   c;
    int    w=n<1?0:n-1;

    if (fField && n<1) w=n=cols;

    while (((c=*(ptr++)) || fSzz) && w>=0)
    {  if (!c && fSzz) if (!*ptr) break;
       if (c) 
       {  if (n<1 && fSzz) ptr=ps(ptr-1);
          else
	  {  pc(c);
             if (n>0) w--;
	  }
       }
       else
       {  switch (*(ptr++))
          {  case 'a':                       // AT lin, col
                at(*(ptr),*(ptr+1));
	        ptr+=2;
                break;
             case 't':			  // TAB col, char
                if ((*ptr)<cols) 
                {  int  tc=*(ptr++);   
                   char fc=*(ptr++);
                   while (wcol==tc) pc(fc);
                } break;
	     case 'i':			  // INK color
	        Ink(*(ptr++));
	        break;
	     case 'p':			  // PAPER color
	        Paper(*(ptr++));
	        break;
	     case 'c':			  // COLOR ink, paper
	        Attr((int)*(ptr),(int)*(ptr+1));
	        ptr+=2;
	        break;
             case 'b':			 // bAttr byte
	        bAttr((uchar)(*(ptr++)));
		break;
	     case '&':			// Hotkey print
	        {  uchar old=bAttr((uchar)(*(ptr++)));
		   pc(*ptr++);
		   bAttr(old);
                   if (n>0) w--;
		} break;
          }
       }
    }
    w++;
    if (w>0 && fField) while (w--) pc(' ');
    return ptr-1;
}

void fl(char * cc, int x, WINOUT * th)
{
      th->pc(*(cc++));
      while(x--) th->pc(*cc);
      th->pc(*(++cc));
}

void WINOUT::fill(int type)
{   char * cc;
    int	    fframe=cols>2 && lins>2 && type!=FT_NOFRAME;
    int     y=lins;
    cc=Chars+(type ? 9:0);

    if (! fframe) cc="   ";
    if (fframe)
    {  at(0,0);
       fl(cc,cols-2,this);
       cc+=3;
       y=lins-2;
    }
    while (y--) fl(cc,cols-2,this);
    if (fframe)
    {  cc+=3;
       fl(cc,cols-2,this);
    }
    if (fframe) at(1,1);
    else at(0,0);
}

// Return Line size (lines & columnes)
//
//   Procedure traces line with counting printing size in fields cols & lins of
// winTEMPL class object pointed by templ. Columns size also using as return
// value.
//   If out=NULL object will be allocated dinamically.
//   Flag fSzz says, if str pointing to string with control codes, prefixed by
// zerochar, so terminator is double zerochar.
//

int ts(char * str, void * templ, int fSzz)
{  int    fDinamic;
   int    c=0;
   int	  l=0;
   char * ptr=str;

   fDinamic=!templ;
   if (fDinamic) templ=(void *)(new winTEMPL);
#define templ ((winTEMPL*)templ)
   templ->lins=1;
   templ->cols=0;   

   while (*ptr || fSzz)
   {  if (!(*ptr) && fSzz) if (!*(ptr+1)) break;
      switch (*(ptr++))
      {  case '\t':
            c=(c+7)&(~7);
	    if (templ->cols < c) templ->cols=c;
	    break;
	 case '\n':
	    c=0;
	    l++;
	    break;
	 case '\0':
	    switch (*(ptr++))
	    {  case 'a':             // at  lin, col
                  l=*(ptr++);
		  c=*(ptr++);
		  break;
	       case 't':		    // tab col, char 
                  if (c>=*ptr) l++;
                  c=*ptr;
	       case 'c':
	          ptr+=2;
	          break;
               case '&':
	          c++;
	          if (templ->cols < c) templ->cols=c;
                  if (templ->lins < l+1) templ->lins=l+1;
               case 'i':
	       case 'p':
               case 'b':
                  ptr++;
		  break;   
	    }
	    break;
         default:
	    c++;
	    if (templ->cols < c) templ->cols=c;
	    if (templ->lins < l+1) templ->lins=l+1;
      } // (switch)
   } // (while)     
   c=templ->cols;
#undef templ
   if (fDinamic) delete (winTEMPL*)templ;
   return c;
}
