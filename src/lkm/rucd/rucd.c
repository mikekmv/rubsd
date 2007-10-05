/* $RuOBSD$ */

/*
 * Copyright (c) 2006 Mike Belopuhov <mkb@crypt.org.ru>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/param.h>
#include <sys/conf.h>
#include <sys/exec.h>
#include <sys/systm.h>
#include <sys/lkm.h>

#include "utf2koi8r.h"

extern u_char (*cd9660_wchar2char)(u_int32_t wchar);

extern int lkmexists(struct lkm_table *);

int rucd_lkmentry(struct lkm_table *, int, int);

MOD_MISC("rucd")

static u_char
conv_utf2koi8r(u_int32_t wchar)
{
	u_char schar;
	int i;

	schar = '?';
	for (i = 0; i < 256; i++) {
		if (utf2koi8r[i] == wchar) {
			schar = i;
			break;
		}
	}
	return (schar);
}

static int
rucd_handle(struct lkm_table *lkmtp, int cmd)
{
	switch (cmd) {
	case LKM_E_LOAD:
		if (lkmexists(lkmtp))
			return (EEXIST);
		cd9660_wchar2char = conv_utf2koi8r;
		break;
	case LKM_E_UNLOAD:
		cd9660_wchar2char = NULL;
		break;
	default:
		return (EINVAL);
	}
	return (0);
}

int
rucd_lkmentry(struct lkm_table *lkmtp, int cmd, int ver)
{
	DISPATCH(lkmtp, cmd, ver, rucd_handle, rucd_handle, lkm_nofunc)
}
