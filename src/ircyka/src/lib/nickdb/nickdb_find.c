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
nickdb_find(void *db, char *nick, struct nick *n)
{
	DBT key, data;
	int rc;

	if (nick == N_CURSOR)
		rc = ((DB *)db)->seq(db, &key, &data, R_CURSOR);
	else if (nick == N_FIRST)
		rc = ((DB *)db)->seq(db, &key, &data, R_FIRST);
	else if (nick == N_LAST)
		rc = ((DB *)db)->seq(db, &key, &data, R_LAST);
	else if (nick == N_PREV)
		rc = ((DB *)db)->seq(db, &key, &data, R_PREV);
	else if (nick == N_NEXT)
		rc = ((DB *)db)->seq(db, &key, &data, R_NEXT);
	else {
		key.data = nick;
		key.size = strlen(nick);
		rc = ((DB *)db)->get(db, &key, &data, 0);
	}

	switch (rc) {
	case 0:
		if (data.size != sizeof(*n)) {
			errno = EFTYPE;
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
