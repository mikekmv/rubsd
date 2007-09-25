/* $oganer: netio.h,v 1.2 2003/08/05 07:30:00 tm Exp $ */

/*
 * Copyright (c) 2003 Ilya Kovalenko <shadow@oganer.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _NETIO_H
#define _NETIO_H

#include <netinet/in.h>

// init flags
#define SF_ZOMBIES    1

// socket flags
#define SF_NONBLOCK   2

#define SUCCESS     0    
#define NO_CLIENTS  (-2)  /* if EWOULDBLOCK returned */

__BEGIN_DECLS

int tcp_init       (int flags);
int tcp_socket     (int flags);
int tcp_server     (int sd, int port, int clients, struct sockaddr_in *name);
int tcp_accept     (int sd);
int tcp_forkclient (int sd, int clsd);
int tcp_client     (int sd, char * host, int port);

__END_DECLS

#endif /* _NETIO_H */
