/*	$RuOBSD: net.h,v 1.14 2002/03/13 09:36:55 gluk Exp $	*/

struct conn {
	int             fd;
	int             nstate;
	int             state;
	int             err;
	int             rw_fl;	/* wait for read - 1 or write - 0 */
	char           *chal;
	char           *crlfp;
	time_t          timeout;
	char           *wp;	/* pointer to byte which must be writen first */
	int             rb;	/* number of bytes we are already read */
	char           *rbuf;
	char           *buf;
	int             bufload;/* how many bytes we have in buf */
	int             bufsize;/* size of mem pointed by buf */
	int             bn;	/* backet number */
	int             bi;	/* index in backet */
} conn;

#define		AUTH_TMOUT	10
#define		READ_TMOUT	300

#define         CHAL_SIZE       32
#define		PEER_BUF_SIZE   32768
#define		READ_BUF_SIZE   100
#define		MAX_ACT_CONN    3

#define START           1
#define WAIT_AUTH       2
#define WRITE_DATA      5
#define READ_DATA       6
#define AUTHTORIZED     8
#define WRITE_ERROR     9
#define SEND_IP_STAT    10
#define CLOSE_CONN	11

struct cmd {
	char           *cmdname;
	char           *cmdhelp;
	int             cmdcode;
};

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

/* functions from net.c */
int	write_protostat_to_buf __P((struct conn *));
int	write_loadstat_to_buf __P((struct conn *));
int	write_portstat_to_buf __P((u_int8_t, struct conn *));
int	write_stat_to_buf __P((struct trafstat **, u_int *, struct conn *));
int	init_net __P((void));
int	get_new_conn __P((struct conn *, int));
int	serve_conn __P((struct conn *));
int	print_debug __P((struct conn *));
int	write_data_to_sock __P((struct conn *));
int	cmd_help __P((struct conn *));
int	close_conn __P((struct conn *, int));
int	get_err __P((int, struct conn *));
void	stop __P((void));
char	*getclientaddr __P((int));
int	chkiplovr __P((void));


