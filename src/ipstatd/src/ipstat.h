/*	$RuOBSD$	*/
/*	$Id$	*/

char           *challenge(int);
char           *bin2ascii(unsigned char *, int);
char           *ascii2bin(char *, int);
int             mydaemon(void);

/* Commands */

#define		ERROR_CMD	0
#define		AUTH_CMD	1
#define		STAT_CMD	2	/* Get traffic statistic */
#define		LOAD_CMD	3
#define		PORT_CMD	5
#define		PROTO_CMD	6
#define		HELP_CMD	7
#define		QUIT_CMD	8
#define		STOP_CMD	9
#define		NOP_CMD		10
#define		VERSION_CMD	21

#ifdef	DEBUG
#define		DEBUG_CMD	20
#endif

#define	MAXCMDLEN	16

struct cmd {
	char           *cmdname;
	char           *cmdhelp;
	int             cmdcode;
};

static struct cmd cmdtab[] =
{
	{"auth", "- MD5 sum for authtorization", AUTH_CMD},
	{"stat", "- generic IP statistic", STAT_CMD},
	{"load", "- get load statistic", LOAD_CMD},
	{"port", " [udp|tcp] - get port traffik statistic", PORT_CMD},
	{"proto", "- get protocol statistic", PROTO_CMD},
	{"help", "- this help", HELP_CMD},
	{"?", "- this help", HELP_CMD},
	{"nop", "- no operation", NOP_CMD},
	{"quit", "- close connection", QUIT_CMD},
	{"stop", "- close all connections and exit daemon", STOP_CMD},
	{"version", "- Get version info", VERSION_CMD},
#ifdef  DEBUG
	{"debug", "- Print internal vars", DEBUG_CMD},
#endif
	{NULL, "", ERROR_CMD}
};

struct err {
	int             errnum;
	char           *errdesc;
};

#define		OK_ERR		200
#define		AUTH_ERR	201
#define		NAUTH_ERR	202
#define		UNKNOWN_ERR	203
#define		INVL_ERR	204
#define		AUTHTMOUT_ERR	205
#define		LOCK_ERR	206
#define		INVLPAR_ERR	207
#define		TMOUT_ERR	208
#define		STOP_ERR	220

#define		config_file	ipstatd.conf
#define         SERVER_PORT     2000

static u_char   password[] = "b449e73b5fc33d66107ee2ecd8489cd2e499d523";
