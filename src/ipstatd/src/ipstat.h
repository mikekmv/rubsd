/*
 *		$Id$
 */

char* challenge(int);
char* bin2ascii(unsigned char*, int);
char* ascii2bin(char*, int);

#define		CHAL_SIZE	32

typedef struct conn_state {
		int	fd;
		int	nstate;
		int	state;
		int	rw_fl;	/* wait for read - 1 or write - 0 */
		char	*chal;
		char	*crlfp;
		time_t	time;
		char	*wp;	/* pointer to byte which must be writen first */
		int	rb;	/* number of bytes we are already read */
		char	*buf;
		int	bufload;	/* how many bytes we have in buf */
		int	bufsize;	/* size of mem pointed by buf */
		int	bn;		/* backet number */
		int	bi;		/* index in backet */
	} conn_state;

#define	START		1
#define WAIT_AUTH	2
#define WRITE_DATA	5
#define READ_DATA	6
#define AUTHTORIZED	8
#define SEND_IP_STAT	10



/*
 * Message exchange protocol.
 * Client send command to server and wait for response
 * Server responses by sending requested data to client
 * Data prepended by header resp_hdr.
 * Client check status and size field of resp_hdr and
 * processes recived data. In the feature client may send
 * confirmation.
 */

typedef struct {
		char	status; 	/* Status or error code */
		u_int	size;
	} resp_hdr;

/* On success status = 0 */

/* Commands */

#define		ERROR_CMD	0
#define		AUTH_CMD	1
#define		STAT_CMD	2	/* Get traffik statistics */
#define		LOAD_CMD	3	
#define		MISC_CMD	4	
#define		PORT_CMD	5
#define		PROTO_CMD	6
#define		HELP_CMD	7
#define		QUIT_CMD	8
#ifdef	DEBUG
#define		DEBUG_CMD	9
#endif


struct cmd
{
        char    *cmdname;
	char	*cmdhelp;
        int     cmdcode;
};

static struct cmd cmdtab[] =
{
        { "auth", "- MD5 sum for authtorization", AUTH_CMD },
        { "stat", "- generic IP statistic", STAT_CMD },
        { "load", "- Not implemented", LOAD_CMD },
        { "misc", "- Not implemented", MISC_CMD },
        { "port", "- Not implemented", PORT_CMD },
        { "proto", "- Not implemented", PROTO_CMD },
        { "help", "- this help", HELP_CMD },
        { "quit", "- close connection", QUIT_CMD },
#ifdef	DEBUG
        { "debug", "- Print internal vars", DEBUG_CMD },
#endif
        { NULL, "", ERROR_CMD }
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

static struct err errtab[] =
{
        { OK_ERR , "- OK" },
        { AUTH_ERR , "- fake authtorization data" },
        { NAUTH_ERR , "- You are not authtorized" },
        { UNKNOWN_ERR , "- Command unknown" },
        { INVL_ERR , "- Invalid command" },
        { AUTHTMOUT_ERR , "- Authtorization timeout" },
        { LOCK_ERR , "- Other client uses this resource" },
        { NULL, "- Unknown error" }
};

#define		config_file	ipstatd.conf
#define               SERVER_PORT     2000

static char password[]="authtest";
