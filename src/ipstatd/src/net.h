/*
 *		$Id$
 */

typedef struct {
                int     fd;
                int     nstate;
                int     state;
		int	err;
                int     rw_fl;  /* wait for read - 1 or write - 0 */
                char    *chal;
                char    *crlfp;
                time_t  timeout;
                char    *wp;    /* pointer to byte which must be writen first */
                int     rb;     /* number of bytes we are already read */
                char    *rbuf;
                char    *buf;
                int     bufload;        /* how many bytes we have in buf */
                int     bufsize;        /* size of mem pointed by buf */
                int     bn;             /* backet number */
                int     bi;             /* index in backet */
        } conn_state;


/* functions from net.c */
int write_protostat_to_buf(conn_state*);
int write_loadstat_to_buf(conn_state*);
int write_portstat_to_buf(u_int8_t,conn_state*);
int write_stat_to_buf(trafstat_t**,u_int*,conn_state*);
int init_net(void);
int get_new_conn(conn_state*,int);
int serve_conn(conn_state*);
int print_debug(conn_state*);
int write_data_to_sock(conn_state*);
int cmd_help(conn_state*);
int close_conn(conn_state*,int);
int get_err(int,conn_state*);
void stop(void);
char* getpeeraddr(int);
int	ckiplovr(void);


#define		AUTH_TMOUT	10
#define		READ_TMOUT	300

#define         CHAL_SIZE       32
#define		PEER_BUF_SIZE   32768
#define		READ_BUF_SIZE   64
#define		MAX_ACT_CONN    3

#define START           1
#define WAIT_AUTH       2
#define WRITE_DATA      5
#define READ_DATA       6
#define AUTHTORIZED     8
#define WRITE_ERROR     9
#define SEND_IP_STAT    10


