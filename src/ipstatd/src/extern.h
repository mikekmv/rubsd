/*	$RuOBSD: extern.h,v 1.3 2002/03/15 11:40:20 tm Exp $	*/

char           *challenge(int);
char           *bin2ascii(unsigned char *, int);
char           *ascii2bin(char *, int);
int             mydaemon(void);
in_addr_t       getaddr(char *);
void		stop(void);

