/*	$RuOBSD: extern.h,v 1.2 2002/03/14 06:53:34 tm Exp $	*/

char           *challenge(int);
char           *bin2ascii(unsigned char *, int);
char           *ascii2bin(char *, int);
int             mydaemon(void);
in_addr_t       getaddr(char *);

