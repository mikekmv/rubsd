/*	$RuOBSD: extern.h,v 1.1 2002/03/13 09:50:50 gluk Exp $	*/

char           *challenge __P((int));
char           *bin2ascii __P((unsigned char *, int));
char           *ascii2bin __P((char *, int));
int             mydaemon __P((void));
in_addr_t       getaddr __P((char *));

