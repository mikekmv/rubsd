/*	$Id$	*/

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


/* functions from net.c */
int             write_protostat_to_buf(struct conn *);
int             write_loadstat_to_buf(struct conn *);
int             write_portstat_to_buf(u_int8_t, struct conn *);
int             write_stat_to_buf(struct trafstat **, u_int *, struct conn *);
int             init_net(void);
int             get_new_conn(struct conn *, int);
int             serve_conn(struct conn *);
int             print_debug(struct conn *);
int             write_data_to_sock(struct conn *);
int             cmd_help(struct conn *);
int             close_conn(struct conn *, int);
int             get_err(int, struct conn *);
void            stop(void);
char           *getclientaddr(int);
int             chkiplovr(void);


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
