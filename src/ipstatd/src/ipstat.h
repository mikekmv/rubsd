/*
 *		$Id$
 */

char* challenge(int);
char* bin2ascii(unsigned char*, int);
char* ascii2bin(char*, int);
void mydaemon(void);
void read_ipl(int);

/* Commands */

#define		ERROR_CMD	0
#define		AUTH_CMD	1
#define		STAT_CMD	2	/* Get traffik statistics */
#define		LOAD_CMD	3	
#define		PORT_CMD	5
#define		PROTO_CMD	6
#define		HELP_CMD	7
#define		QUIT_CMD	8

#ifdef	DEBUG
#define		DEBUG_CMD	20
#define		VERSION_CMD	21
#endif

struct cmd
{
        char    *cmdname;
	char	*cmdhelp;
        int     cmdcode;
};

struct err
{
        int	errnum;
	char	*errdesc;
};

#define		OK_ERR		200
#define		AUTH_ERR	201
#define		NAUTH_ERR	202
#define		UNKNOWN_ERR	203
#define		INVL_ERR	204
#define		AUTHTMOUT_ERR	205
#define		LOCK_ERR	206
#define		INVLPAR_ERR	207

#define		config_file	ipstatd.conf
#define         SERVER_PORT     2000

static char password[]="authtest";
