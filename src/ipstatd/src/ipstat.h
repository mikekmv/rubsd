/*	$RuOBSD: ipstat.h,v 1.20 2002/03/13 02:18:47 tm Exp $	*/

char           *challenge(int);
char           *bin2ascii(unsigned char *, int);
char           *ascii2bin(char *, int);
int             mydaemon(void);
in_addr_t	getaddr(char *);

#define	MAXCMDLEN	16

#define	config_file	ipstatd.conf
#define	SERVER_PORT     2000

static u_char   password[] = "b449e73b5fc33d66107ee2ecd8489cd2e499d523";
