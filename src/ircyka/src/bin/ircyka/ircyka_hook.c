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

#include "ircyka/ircyka.h"

struct ircyka_hook *
hook_add(struct ircyka_hooks *hooks, ircyka_hook_t hook)
{
	struct ircyka_hook *ih;

	if ((ih = malloc(sizeof(*ih))) != NULL) {
		ih->ih_hook = hook;
		LIST_INSERT_HEAD(hooks, ih, ih_entry);
	}

	return (ih);
}

void
hook_del(struct ircyka_hook *ih)
{
	LIST_REMOVE(ih, ih_entry);
	free(ih);
}

void
hook_exec(const struct ircyka_hooks *hooks, int type, const void *data)
{
	struct ircyka_hook *ih;

	LIST_FOREACH(ih, hooks, ih_entry) {
		ih->ih_hook(type, data);
	}
}
