#include "global.h"


char * TitleText = "Менeджер оплат (вер. 0.0)";

int RefreshTitle()
{  int i;
   int n;

   Attr(8, 7);
   Gotoxy(0,0);

// Output centered text
   n = (ForceCols - strlen(TitleText)) / 2;
   for(i = 0; i < n; i++) Putch(' ');

   Puts(TitleText);   

   n = (ForceCols - strlen(TitleText)) / 2 + 
       (ForceCols - strlen(TitleText)) % 2;
   for(i = 0; i < n; i++) Putch(' ');
   
   return 0;
}
