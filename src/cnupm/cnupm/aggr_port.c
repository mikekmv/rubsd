/*	$RuOBSD: aggr_port.c,v 1.1 2005/07/03 13:08:28 form Exp $	*/

/*
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
#ifdef linux
#include <netinet/in.h>
#endif
#include <sys/tree.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>

#include "aggregate.h"


struct aggr_port_entry {
	struct aggr_port		ape_aggr;
	RB_ENTRY(aggr_port_entry)	ape_entry;
#define ape_first			ape_aggr.ap_first
#define ape_last			ape_aggr.ap_last
#define ape_port			ape_aggr.ap_port
};


RB_HEAD(aggr_port_tree, aggr_port_entry) aggr_port_tree;

static struct aggr_port_entry *ape;


static __inline int
aggr_port_compare(struct aggr_port_entry *a, struct aggr_port_entry *b)
{
	int ad = a->ape_last - a->ape_first;
	int bd = b->ape_last - b->ape_first;

	if (ad < bd)
		return (-1);
	if (ad > bd)
		return (1);
	if (a->ape_first < b->ape_first)
		return (-1);
	if (a->ape_first > b->ape_first)
		return (1);
	return (0);
}


RB_PROTOTYPE(aggr_port_tree, aggr_port_entry, ape_entry, aggr_port_compare)


RB_GENERATE(aggr_port_tree, aggr_port_entry, ape_entry, aggr_port_compare)

void
aggr_port_init(void)
{
	RB_INIT(&aggr_port_tree);
	ape = malloc(sizeof(*ape));
}

void
aggr_port_final(void)
{
	if (ape != NULL) {
		free(ape);
		ape = NULL;
	}
}

int
aggr_port_add(in_port_t first, in_port_t last, in_port_t port)
{
	if (first > last) {
		errno = EINVAL;
		return (-1);
	}
	if (ape == NULL) {
		errno = ENOMEM;
		return (-1);
	}

	ape->ape_first = first;
	ape->ape_last = last;
	ape->ape_port = port;
	if (RB_INSERT(aggr_port_tree, &aggr_port_tree, ape) == NULL) {
		ape = malloc(sizeof(*ape));
		return (0);
	}
	return (1);
}

in_port_t
aggr_port(in_port_t port)
{
	struct aggr_port_entry *ap;

	port = port;
	RB_FOREACH(ap, aggr_port_tree, &aggr_port_tree)
		if (port >= ap->ape_first && port <= ap->ape_last)
			return (ap->ape_port != 0 ? ap->ape_port : port);
	return (port);
}

const char *
aggr_port_compile(const char *expr)
{
	const char *save_expr;
	char *ep;
	long lval;
	in_port_t first, last, port;

	while (*expr != '\0') {
		save_expr = expr;
		lval = strtol(expr, &ep, 10);
		errno = EINVAL;
		if (*ep != '\0' && *ep != '-' && *ep != ',' && *ep != ':')
			return (ep);
		if (lval < 1 || lval > 65535)
			return (expr);
		first = last = lval;
		port = 0;
		if (*ep == '-') {
			expr = ++ep;
			lval = strtol(expr, &ep, 10);
			errno = EINVAL;
			if ((*ep != '\0' && *ep != ',' && *ep != ':') ||
			    *expr == '\0')
				return (ep);
			if (lval < 1 || lval > 65535)
				return (expr);
			last = lval;
		}
		if (*ep == ':') {
			expr = ++ep;
			lval = strtol(expr, &ep, 10);
			errno = EINVAL;
			if ((*ep != '\0' && *ep != ',') || *expr == '\0')
				return (ep);
			if (lval < 0 || lval > 65535)
				return (expr);
			port = lval;
		}
		if (*ep == ',' && *++ep == '\0') {
			errno = EINVAL;
			return (--ep);
		}
		if (aggr_port_add(first, last, port) < 0)
			return (save_expr);
		expr = ep;
	}

	return (NULL);
}
