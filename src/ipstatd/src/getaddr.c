/*
 *		$Id$
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <sys/socket.h>


u_long getaddr(char *host_name)
{

	struct in_addr host_address;
	struct hostent *host_ent;

	if((host_address.s_addr = inet_addr(host_name)) == -1){
	  if(!(host_ent = gethostbyname(host_name))){
	    perror("gethostbyname");
	    exit(1);
	  }else{
	    memcpy((char *)&host_address.s_addr, *(host_ent->h_addr_list), host_ent->h_length);
	  }
	}
	return(host_address.s_addr);
}
