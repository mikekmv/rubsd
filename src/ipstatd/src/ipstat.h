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

#define		SERVER_PORT	2000
