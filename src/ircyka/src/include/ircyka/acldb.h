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

#ifndef __ACLDB_H__
#define __ACLDB_H__

#define ACLDB_MAX_NICKLEN	16
#define ACLDB_DB_MODE		0600

#define A_CURSOR		(void *)0
#define A_FIRST			(void *)(-1)
#define A_LAST			(void *)(-2)
#define A_PREV			(void *)(-3)
#define A_NEXT			(void *)(-4)


struct acl {
	char			a_nick[ACLDB_MAX_NICKLEN];
	u_int32_t		a_flags;
#define AF_OPER			0x00000001U
#define AF_HALFOP		0x00000002U
#define AF_VOICE		0x00000004U
#define AF_INVITE		0x00000008U
};


__BEGIN_DECLS
void	*acldb_open(const char *, int);
void	*acldb_creat(const char *);
int	acldb_close(void *);
int	acldb_sync(void *);
int	acldb_add(void *, const char *, const char *, u_int32_t);
int	acldb_update(void *, const char *, const char *, u_int32_t);
int	acldb_get(void *, const char *, const char *, u_int32_t *);
int	acldb_del(void *, const char *, const char *);
int	acldb_find(void *, const char *, char **, struct acl **);
int	acldb_flags(const char *, u_int32_t *);
char	*acldb_aflags(u_int32_t);
__END_DECLS

#endif	/* __ACLDB_H__ */
