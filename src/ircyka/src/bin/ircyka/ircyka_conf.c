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


int
conf_load(const char *file, const char *name, struct ircyka_conf_var *icv)
{
	char *buf = NULL, *cap, *db_array[2];
	int rc;

	bcopy(&file, db_array, sizeof(file));
	db_array[1] = NULL;

	if ((rc = cgetent(&buf, db_array, name)) == 0)
		conf_unload(icv);

	while (rc == 0 && icv->icv_type != ICVT_EOV) {
		switch (icv->icv_type) {
		case ICVT_NUM:
			cgetnum(buf, icv->icv_name, icv->icv_data);
			break;
		case ICVT_BOOL:
			cap = cgetcap(buf, icv->icv_name, ':');
			if (cap != NULL && (*cap == ':' || *cap == '\0'))
				*(int *)icv->icv_data = icv->icv_dflt == NULL;
			break;
		case ICVT_STR:
			if ((rc = cgetstr(buf, icv->icv_name,
			    icv->icv_data)) != -2)
				rc = 0;
			break;
		}
		icv++;
	}

	if (buf != NULL) {
		int save_errno = errno;

		free(buf);
		errno = save_errno;
	}

	return (rc);
}

void
conf_unload(struct ircyka_conf_var *icv)
{
	while (icv->icv_type != ICVT_EOV) {
		switch (icv->icv_type) {
		case ICVT_NUM:
			*(long *)icv->icv_data = (long)icv->icv_dflt;
			break;
		case ICVT_BOOL:
			*(int *)icv->icv_data = icv->icv_dflt != NULL;
			break;
		case ICVT_STR:
			if (*(void **)icv->icv_data != NULL &&
			    *(void **)icv->icv_data != icv->icv_dflt)
				free(*(void **)icv->icv_data);
			*(void **)icv->icv_data = icv->icv_dflt;
			break;
		}
		icv++;
	}
}

struct ircyka_conf_var *
conf_find(struct ircyka_conf_var *icv, const char *name, int type)
{
	while (icv->icv_type != ICVT_EOV) {
		if (icv->icv_type == type && strcmp(icv->icv_name, name) == 0)
			return (icv);
		icv++;
	}
	return (NULL);
}

void
conf_setdef(struct ircyka_conf_var *icv, const char *name, int type,
    void *dflt)
{
	if ((icv = conf_find(icv, name, type)) != NULL) {
		switch (icv->icv_type) {
		case ICVT_NUM:
			*(long *)icv->icv_data = (long)dflt;
			break;
		case ICVT_BOOL:
			*(int *)icv->icv_data = dflt != NULL;
			break;
		case ICVT_STR:
			if (*(void **)icv->icv_data != NULL &&
			    *(void **)icv->icv_data != icv->icv_dflt)
				free(*(void **)icv->icv_data);
			*(void **)icv->icv_data = dflt;
			break;
		}
		icv->icv_dflt = dflt;
	}
}

void
conf_reset(struct ircyka_conf_var *icv, const char *name, int type)
{
	if ((icv = conf_find(icv, name, type)) != NULL) {
		switch (icv->icv_type) {
		case ICVT_NUM:
			*(long *)icv->icv_data = (long)icv->icv_dflt;
			break;
		case ICVT_BOOL:
			*(int *)icv->icv_data = icv->icv_dflt != NULL;
			break;
		case ICVT_STR:
			if (*(void **)icv->icv_data != NULL &&
			    *(void **)icv->icv_data != icv->icv_dflt)
				free(*(void **)icv->icv_data);
			*(void **)icv->icv_data = icv->icv_dflt;
			break;
		}
	}
}
