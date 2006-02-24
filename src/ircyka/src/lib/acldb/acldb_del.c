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
#include <db.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "ircyka/acldb.h"


int
acldb_del(void *db, const char *channel, const char *nick)
{
	struct acl *a;
	DBT key, data;
	size_t i, n;
	int rc, save_errno;

	bcopy(&channel, &key.data, sizeof(key.data));
	key.size = strlen(channel);
	if ((rc = ((DB *)db)->get(db, &key, &data, 0)) == 0) {
		if (((n = data.size) % sizeof(*a)) != 0) {
			errno = EFTYPE;
			return (-1);
		}
		for (i = 0, n /= sizeof(*a), a = data.data; i < n; i++) {
			if (strcmp(nick, a[i].a_nick) == 0)
				break;
			if (i == n) {
				errno = ENOENT;
				return (-1);
			}
		}
		if (n == 1)
			rc = ((DB *)db)->del(db, &key, 0);
		else {
			if ((a = malloc(data.size - sizeof(*a))) == NULL)
				return (-1);
			bcopy(data.data, a, sizeof(*a) * i);
			bcopy((struct acl *)data.data + i + 1, a + i,
			    sizeof(*a) * (n - i - 1));
			data.data = a;
			data.size -= sizeof(*a);
			rc = ((DB *)db)->put(db, &key, &data, 0);
			save_errno = errno;
			free(a);
			errno = save_errno;
		}
	} else if (rc == 1) {
		errno = ENOENT;
		rc = -1;
	}

	return (rc);
}
