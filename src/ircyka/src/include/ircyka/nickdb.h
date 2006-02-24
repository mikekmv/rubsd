/*
 * $RuOBSD$
 *
 * Copyright (c) 2005 Oleg Safiullin <form@pdp-11.org.ru>
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

#ifndef __NICKDB_H__
#define __NICKDB_H__

#include <sys/types.h>
#include <md5.h>


#define MAX_NICKLEN	16
#define MAX_NAMELEN	64
#define NICK_DB_MODE	0600

#define N_CURSOR	(void *)0
#define N_FIRST		(void *)(-1)
#define N_LAST		(void *)(-2)
#define N_PREV		(void *)(-3)
#define N_NEXT		(void *)(-4)


struct nick {
	char		n_nick[MAX_NICKLEN];
	char		n_name[MAX_NAMELEN];
	u_int8_t	n_passwd[MD5_DIGEST_LENGTH];
	u_int32_t	n_flags;
#define NF_PRIV		0x00000001U
#define NF_DISABLED	0x00000002U
#define NF_CHANOUT	0x00000004U
#define NF_AUTO		0x00000008U
};


__BEGIN_DECLS
void	*nickdb_open(const char *, int);
void	*nickdb_creat(const char *);
int	nickdb_close(void *);
int	nickdb_sync(void *);
int	nickdb_add(void *, struct nick *);
int	nickdb_del(void *, char *);
int	nickdb_update(void *, struct nick *);
int	nickdb_find(void *, char *, struct nick *);
int	nickdb_auth(void *, char *, char *, struct nick *);
void	nickdb_passwd(const char *, void *);
int	nickdb_flags(const char *, u_int32_t *);
char	*nickdb_aflags(u_int32_t);
__END_DECLS

#endif	/* __NICKDB_H__*/
