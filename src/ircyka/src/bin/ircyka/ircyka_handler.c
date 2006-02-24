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

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "ircyka/ircyka.h"


static __inline int
handler_compare(struct ircyka_handler *a, struct ircyka_handler *b)
{
	return (strcasecmp(a->ih_name, b->ih_name));
}

RB_GENERATE(ircyka_handler_tree, ircyka_handler, ih_entry, handler_compare)


struct ircyka_handler *
handler_find(struct ircyka_handler_tree *handlers, const char *name)
{
	struct ircyka_handler ih;

	ih.ih_name = name;
	return (RB_FIND(ircyka_handler_tree, handlers, &ih));
}

struct ircyka_handler *
handler_create(struct ircyka_handler_tree *handlers, const char *name)
{
	struct ircyka_handler *ih;
	size_t len;

	if ((ih = handler_find(handlers, name)) == NULL) {
		len = strlen(name) + 1;
		if ((ih = malloc(sizeof(*ih) + len)) != NULL) {
			bcopy(name, ih + 1, len);
			ih->ih_name = (const char *)(ih + 1);
			LIST_INIT(&ih->ih_callbacks);
			(void)RB_INSERT(ircyka_handler_tree, handlers, ih);
		}
	}
	return (ih);
}

void
handler_destroy(struct ircyka_handler_tree *handlers,
    struct ircyka_handler *ih)
{
	struct ircyka_callback *icb;

	while ((icb = LIST_FIRST(&ih->ih_callbacks)) != NULL) {
		LIST_REMOVE(icb, icb_entry);
		free(icb);
	}
	(void)RB_REMOVE(ircyka_handler_tree, handlers, ih);
	free(ih);
}

struct ircyka_callback *
handler_register(struct ircyka_handler_tree *handlers, const char *name,
    int (*cb)(int, char * const *))
{
	struct ircyka_handler *ih;
	struct ircyka_callback *icb;

	if ((icb = malloc(sizeof(*icb))) != NULL) {
		if ((ih = handler_create(handlers, name)) == NULL) {
			int save_errno = errno;

			free(icb);
			errno = save_errno;
			return (NULL);
		}
		icb->icb_cb = cb;
		icb->icb_ih = ih;
		LIST_INSERT_HEAD(&ih->ih_callbacks, icb, icb_entry);
	}
	return (icb);
}

void
handler_unregister(struct ircyka_handler_tree *handlers,
    struct ircyka_callback *icb)
{
	LIST_REMOVE(icb, icb_entry);
	if (LIST_EMPTY(&icb->icb_ih->ih_callbacks)) {
		(void)RB_REMOVE(ircyka_handler_tree, handlers, icb->icb_ih);
		free(icb->icb_ih);
	}
	free(icb);
}
