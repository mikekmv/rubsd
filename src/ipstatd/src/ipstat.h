/*
 *		$Id$
 */

char* challenge(int);
char* bin2ascii(unsigned char*, int);
char* ascii2bin(char*, int);

#define		CHAL_SIZE	32

typedef struct {
		int	fd;
		int	state;
		int	rw_fl;	/* wait for read - 1 or write - 0 */
		char	*chal;
		char	*retp;
		time_t	time;
		int	wb;	/* number of bytes we are already write */
		int	rb;	/* number of bytes we are already read */
		char	buf[100];
	} conn_state;

#define	START		1
#define CHAL_SEND	2
#define WAIT_AUTH	2
#define AUTH_RECV	3
#define AUTHTORIZED	4



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

#define		STAT_CMD	"stat"	/* Get traffik statistics */
#define		LOAD_CMD	"load"
#define		MISC_CMD	"misc"
#define		PORT_CMD	"port"
#define		PROTO_CMD	"proto"
#define		AUTH_CMD	"auth"

#define		SERVER_PORT	2000

#define		config_file	ipstatd.conf

static char password[]="authtest";
