/*	$RuOBSD$	*/

/*
 * Copyright (c) 2004 Oleg Safiullin <form@pdp-11.org.ru>
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
#include <net/if_acct.h>
#include <stdlib.h>

#include "flow.h"


int flow_sort_order = 1;


static int sort_first(const void *, const void *);
static int sort_last(const void *, const void *);
static int sort_src(const void *, const void *);
static int sort_dst(const void *, const void *);
static int sort_pkts(const void *, const void *);
static int sort_octets(const void *, const void *);


void
flow_sort(struct acct_flow **flows, size_t nflows, int mode)
{
	switch (mode) {
	case FLOW_SORT_FIRST:
		qsort(flows, nflows, sizeof(*flows), sort_first);
		break;
	case FLOW_SORT_LAST:
		qsort(flows, nflows, sizeof(*flows), sort_last);
		break;
	case FLOW_SORT_SRC:
		qsort(flows, nflows, sizeof(*flows), sort_src);
		break;
	case FLOW_SORT_DST:
		qsort(flows, nflows, sizeof(*flows), sort_dst);
		break;
	case FLOW_SORT_PKTS:
		qsort(flows, nflows, sizeof(*flows), sort_pkts);
		break;
	case FLOW_SORT_OCTETS:
		qsort(flows, nflows, sizeof(*flows), sort_octets);
		break;
	}
}

static int
sort_first(const void *a, const void *b)
{
	const struct acct_flow *afa = *(struct acct_flow **)a;
	const struct acct_flow *afb = *(struct acct_flow **)b;

	if (afa->af_first < afb->af_first)
		return (-flow_sort_order);
	if (afa->af_first > afb->af_first)
		return (flow_sort_order);
	return (0);
}

static int
sort_last(const void *a, const void *b)
{
	const struct acct_flow *afa = *(struct acct_flow **)a;
	const struct acct_flow *afb = *(struct acct_flow **)b;

	if (afa->af_last < afb->af_last)
		return (-flow_sort_order);
	if (afa->af_last > afb->af_last)
		return (flow_sort_order);
	return (0);
}

static int
sort_src(const void *a, const void *b)
{
	const struct acct_flow *afa = *(struct acct_flow **)a;
	const struct acct_flow *afb = *(struct acct_flow **)b;

	if (afa->af_src < afb->af_src)
		return (-flow_sort_order);
	if (afa->af_src > afb->af_src)
		return (flow_sort_order);
	return (0);
}

static int
sort_dst(const void *a, const void *b)
{
	const struct acct_flow *afa = *(struct acct_flow **)a;
	const struct acct_flow *afb = *(struct acct_flow **)b;

	if (afa->af_dst < afb->af_dst)
		return (-flow_sort_order);
	if (afa->af_dst > afb->af_dst)
		return (flow_sort_order);
	return (0);
}

static int
sort_pkts(const void *a, const void *b)
{
	const struct acct_flow *afa = *(struct acct_flow **)a;
	const struct acct_flow *afb = *(struct acct_flow **)b;

	if (afa->af_pkts < afb->af_pkts)
		return (-flow_sort_order);
	if (afa->af_pkts > afb->af_pkts)
		return (flow_sort_order);
	return (0);
}

static int
sort_octets(const void *a, const void *b)
{
	const struct acct_flow *afa = *(struct acct_flow **)a;
	const struct acct_flow *afb = *(struct acct_flow **)b;

	if (afa->af_octets < afb->af_octets)
		return (-flow_sort_order);
	if (afa->af_octets > afb->af_octets)
		return (flow_sort_order);
	return (0);
}
