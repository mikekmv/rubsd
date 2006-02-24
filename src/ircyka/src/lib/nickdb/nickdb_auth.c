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

#include <sys/types.h>
#include <db.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "ircyka/nickdb.h"


int
nickdb_auth(void *db, char *nick, char *passwd, struct nick *n)
{
	u_int8_t digest[MD5_DIGEST_LENGTH];
	DBT key, data;
	MD5_CTX ctx;

	key.data = nick;
	key.size = strlen(nick);
	switch (((DB *)db)->get(db, &key, &data, 0)) {
	case 0:
		if (data.size != sizeof(*n)) {
			errno = EFTYPE;
			break;
		}

		MD5Init(&ctx);
		MD5Update(&ctx, (u_int8_t *)passwd, strlen(passwd));
		MD5Final(digest, &ctx);
		if (bcmp(digest, ((struct nick *)data.data)->n_passwd,
		    sizeof(digest)) != 0) {
			errno = EACCES;
			break;
		}
		bcopy(data.data, n, sizeof(*n));
		return (0);
	case 1:
		errno = ENOENT;
		break;
	}

	return (-1);
}
