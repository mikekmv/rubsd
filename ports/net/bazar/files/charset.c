/* $FoRM$ */

/*
 * Copyright (c) 1999 Oleg Safiullin
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
#include <string.h>

#include "charset.h"

int local_charset = CHARSET_KOI8;
int remote_charset = CHARSET_KOI8;

char **local_to_remote = NULL;
char **remote_to_local = NULL;

static char *win_to_koi[] = {
	"ÿ",	"ç",	"\'",	"Ç",	"\"",	"...",	"+",	"+",
	"#",	"%",	"ìø",	"<",	"îø",	"ë",	"h",	"ã",
	"h",	"`",	"\'",	"\"",	"\"",	"\x95",	"-",	"-",
	"#",	"(TM)",	"ÌØ",	">",	"ÎØ",	"Ë",	"h",	"Ã",
	"\x9A",	"õ",	"Õ",	"J",	"$",	"ç",	"|",	"$",
	"\xB3",	"(C)",	"å",	"\"",	"^",	"-",	"(R)",	"I",
	"\x9C",	"+",	"I",	"i",	"Ç",	"m",	"#",	"\x9E",
	"\xA3",	"N",	"Å",	"\"",	"j",	"S",	"s",	"i",
	"\xE1",	"\xE2",	"\xF7",	"\xE7",	"\xE4",	"\xE5",	"\xF6",	"\xFA",
	"\xE9",	"\xEA",	"\xEB",	"\xEC",	"\xED",	"\xEE",	"\xEF",	"\xF0",
	"\xF2",	"\xF3",	"\xF4",	"\xF5",	"\xE6",	"\xE8",	"\xE3",	"\xFE",
	"\xFB",	"\xFD",	"\xFF",	"\xF9",	"\xF8",	"\xFC",	"\xE0",	"\xF1",
	"\xC1",	"\xC2",	"\xD7",	"\xC7",	"\xC4",	"\xC5",	"\xD6",	"\xDA",
	"\xC9",	"\xCA",	"\xCB",	"\xCC",	"\xCD",	"\xCE",	"\xCF",	"\xD0",
	"\xD2",	"\xD3",	"\xD4",	"\xD5",	"\xC6",	"\xC8",	"\xC3",	"\xDE",
	"\xDB",	"\xDD",	"\xDF",	"\xD9",	"\xD8",	"\xDC",	"\xC0",	"\xD1"
};

static char *koi_to_win[] = {
	"-",	"|",	"ã",	"¬",	"L",	"-",	"+",	"+",
	"T",	"+",	"+",	"-",	"-",	"#",	"|",	"|",
	"#",	"#",	"#",	"?",	"|",	"\x95",	"v",	"=",
	"<",	">",	"\xA0",	"?",	"\xB0",	"2",	"\xB7",	"+",
	"=",	"|",	"ã",	"\xB8",	"ã",	"ã",	"¬",	"¬",
	"¬",	"L",	"L",	"L",	"-",	"-",	"-",	"|",
	"|",	"|",	"|",	"\xA8",	"|",	"|",	"T",	"T",
	"T",	"-",	"-",	"-",	"+",	"+",	"+",	"\xA9",
	"\xFE",	"\xE0",	"\xE1",	"\xF6",	"\xE4",	"\xE5",	"\xF4",	"\xE3",
	"\xF5",	"\xE8",	"\xE9",	"\xEA",	"\xEB",	"\xEC",	"\xED",	"\xEE",
	"\xEF",	"\xFF",	"\xF0",	"\xF1",	"\xF2",	"\xF3",	"\xE6",	"\xE2",
	"\xFC",	"\xFB",	"\xE7",	"\xF8",	"\xFD",	"\xF9",	"\xF7",	"\xFA",
	"\xDE",	"\xC0",	"\xC1",	"\xD6",	"\xC4",	"\xC5",	"\xD4",	"\xC3",
	"\xD5",	"\xC8",	"\xC9",	"\xCA",	"\xCB",	"\xCC",	"\xCD",	"\xCE",
	"\xCF",	"\xDF",	"\xD0",	"\xD1",	"\xD2",	"\xD3",	"\xC6",	"\xC2",
	"\xDC",	"\xDB",	"\xC7",	"\xD8",	"\xDD",	"\xD9",	"\xD7",	"\xDA"
};

void
charset_init(void)
{
	if (local_charset == CHARSET_KOI8 &&
		remote_charset == CHARSET_WINDOWS) {
		local_to_remote = koi_to_win;
		remote_to_local = win_to_koi;
		return;
	}
	if (local_charset == CHARSET_WINDOWS &&
		remote_charset == CHARSET_KOI8) {
		local_to_remote = win_to_koi;
		remote_to_local = koi_to_win;
		return;
	}
	local_to_remote = remote_to_local = NULL;
}

int
ltor(src, dst, len, size)
	char **src, *dst;
	int *len, size;
{
	int count = 0;

	if (!local_to_remote) {
		count = (*len > size) ? size : *len;
		bcopy(*src, dst, count);
		(*src) += count;
		*len -= count;
		return (count);
	}

	while (*len && size) {
		if (ISASCII(**src)) {
			*dst++ = (**src == '\r') ? '\n' : **src;
			--size;
			count++;
		} else {
			int n;
			char *p = local_to_remote[STRIP((u_char)**src)];

			if ((n = strlen(p)) > size)
				break;
			bcopy(p, dst, n);
			dst += n;
			size -= n;
			count += n;
		}
		(*src)++;
		--*len;
	}
	return (count);
}

int
rtol(src, dst, len, size)
	char **src, *dst;
	int *len, size;
{
	int count = 0;

	if (!remote_to_local) {
		count = (*len > size) ? size : *len;
		bcopy(*src, dst, count);
		(*src) += count;
		*len -= count;
		return (count);
	}

	while (*len && size) {
		if (ISASCII(**src)) {
			*dst++ = (**src == '\r') ? '\n' : **src;
			--size;
			count++;
		} else {
			int n;
			char *p = remote_to_local[STRIP((u_char)**src)];

			if ((n = strlen(p)) > size)
				break;
			bcopy(p, dst, n);
			dst += n;
			size -= n;
			count += n;
		}
		(*src)++;
		--*len;
	}
	return (count);
}
