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

#include <stdlib.h>
#include <time.h>

#include "ircyka/ircyka.h"


struct ircyka_join *
join_create(struct ircyka_channel *ic, struct ircyka_nick *in, u_int32_t flags)
{
	struct ircyka_join *ij;

	if ((ij = malloc(sizeof(*ij))) != NULL) {
		ij->ij_ic = ic;
		ij->ij_in = in;
		ij->ij_time = time(NULL);
		ij->ij_flags = flags;
		LIST_INSERT_HEAD(&in->in_joins, ij, ij_nentry);
		LIST_INSERT_HEAD(&ic->ic_joins, ij, ij_centry);
	}
	return (ij);
}

void
join_destroy(struct ircyka_join *ij)
{
	LIST_REMOVE(ij, ij_nentry);
	LIST_REMOVE(ij, ij_centry);
	if (LIST_EMPTY(&ij->ij_ic->ic_joins) &&
	    !(ij->ij_ic->ic_flags & ICF_PERSIST))
		(void)irc_part(ij->ij_ic, irc_nick, irc_partmsg);
	free(ij);
}
