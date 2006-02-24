/*
 * $RuOBSD$
 *
 * Copyright (c) 2005-2006 Oleg Safiullin <form@pdp-11.org.ru>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ircyka/ircyka.h"


const char *
ircyka_sockaddr(const void *sa)
{
	const struct sockaddr_in6 *sin6 = sa;
	const struct sockaddr_in *sin = sa;
	static char addr[INET6_ADDRSTRLEN + 6];
	size_t len;

	switch (sin->sin_family) {
	case AF_INET:
		(void)inet_ntop(AF_INET, &sin->sin_addr, addr, sizeof(addr));
		break;
	case AF_INET6:
		(void)inet_ntop(AF_INET, &sin6->sin6_addr, addr, sizeof(addr));
		break;
	default:
		return ("?:?");
	}

	len = strlen(addr);
	(void)snprintf(addr + len, sizeof(addr) - len, ":%u",
	    ntohs(sin->sin_port));
	return (addr);
}

int
ircyka_connect(const char **errstr)
{
	struct addrinfo hints, *res, *res0;
	const char *cause = NULL;
	int error, s = -1;

	if (ircyka_debug)
		(void)fprintf(stderr, "Connecting to %s:%s...\n",
		    irc_host, irc_port);

	bzero(&hints, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if ((error = getaddrinfo(irc_host, irc_port, &hints, &res0)) != 0) {
		*errstr = gai_strerror(error);
		return (-2);
	}

	for (res = res0; res != NULL; res = res->ai_next) {
		if (ircyka_debug)
			(void)fprintf(stderr, "Trying %s...\n",
			    ircyka_sockaddr(res->ai_addr));
		if ((s = socket(res->ai_family, res->ai_socktype,
		    res->ai_protocol)) < 0) {
			cause = "socket";
			continue;
		}
		if (connect(s, res->ai_addr, res->ai_addrlen) < 0) {
			error = errno;
			cause = "connect";
			(void)close(s);
			errno = error;
			continue;
		}
		if (ircyka_debug)
			(void)fprintf(stderr, "Connected to %s...\n",
			    ircyka_sockaddr(res->ai_addr));
		cause = NULL;
		break;
	}

	error = errno;
	freeaddrinfo(res0);
	errno = error;

	if (cause != NULL) {
		*errstr = cause;
		return (-1);
	}

	*errstr = NULL;
	return (s);
}
