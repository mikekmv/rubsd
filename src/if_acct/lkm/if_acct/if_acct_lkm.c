/*	$RuOBSD: if_acct_lkm.c,v 1.1.1.1 2004/10/27 06:32:39 form Exp $	*/

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

#include <sys/param.h>
#include <sys/conf.h>
#include <sys/exec.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/systm.h>
#include <sys/lkm.h>

#include <net/if_acct.h>


extern int lkmexists(struct lkm_table *);

static int if_acct_load(struct lkm_table *, int);
int if_acct_lkmentry(struct lkm_table *, int, int);


MOD_MISC("if_acct")


int
if_acct_lkmentry(struct lkm_table *lkmtp, int cmd, int ver)
{
	DISPATCH(lkmtp, cmd, ver, if_acct_load, if_acct_load, lkm_nofunc)
}

static int
if_acct_load(struct lkm_table *lkmtp, int cmd)
{
	int error;

	switch (cmd) {
	case LKM_E_LOAD:
		if (lkmexists(lkmtp))
			error = EEXIST;
		else
			error = acct_attach();
		break;
	case LKM_E_UNLOAD:
		error = acct_detach();
		break;
	default:
		error = EINVAL;
		break;
	}

	return (error);
}
