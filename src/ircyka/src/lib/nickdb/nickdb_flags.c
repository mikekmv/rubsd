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

#include <string.h>

#include "ircyka/nickdb.h"


int
nickdb_flags(const char *aflags, u_int32_t *flags)
{
	u_int32_t mask, nflags = *flags;
	size_t len, i;
	int op = 0;

	for (i = 0, len = strlen(aflags); i < len; i++) {
		mask = 0;
		switch (aflags[i]) {
		case '-':
			op = -1;
			continue;
		case '+':
			op = 1;
			continue;
		case 'a':
		case 'A':
			mask |= NF_AUTO;
			break;
		case 'c':
		case 'C':
			mask |= NF_CHANOUT;
			break;
		case 'd':
		case 'D':
			mask |= NF_DISABLED;
			break;
		case 'P':
		case 'p':
			mask |= NF_PRIV;
			break;
		default:
			return ((int)aflags[i]);
		}

		if (op == -1)
			nflags &= ~mask;
		else if (op == 1)
			nflags |= mask;
		else
			return (-1);
	}

	*flags = nflags;
	return (0);
}
