/*	$Id$	*/

#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

in_addr_t
getaddr(char *host_name)
{

	struct in_addr  host_address;
	struct hostent *host_ent;

	if ((host_address.s_addr = inet_addr(host_name)) == INADDR_NONE) {
		if (!(host_ent = gethostbyname(host_name))) {
			perror("gethostbyname");
			exit(1);
		} else {
			memcpy((char *)&host_address.s_addr,
			    *host_ent->h_addr_list,
			    sizeof(host_address.s_addr));
		}
	}
	return (host_address.s_addr);
}
