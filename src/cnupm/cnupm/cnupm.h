/*	$RuOBSD: cnupm.h,v 1.2 2003/10/08 06:18:24 form Exp $	*/

/*
 * Copyright (c) 2003 Oleg Safiullin <form@pdp11.org.ru>
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

#ifndef __CNUPM_H__
#define __CNUPM_H__

#define CNUPM_VERSION_MAJOR	2		/* major version */
#define CNUPM_VERSION_MINOR	5		/* minor version */

#define CNUPM_USER		"cnupm"		/* cnupm user */
#define CNUPM_PIDFILE		"cnupm-%s.pid"	/* cnupm pid file */
#define CNUPM_DUMPFILE		"cnupm-%s.dump"	/* traffic dump file */

#define CNUPM_FLAG_INET6	0x00010000U	/* INET6 supported */
#define CNUPM_FLAG_PROTO	0x00020000U	/* protocol stats supported */
#define CNUPM_FLAG_PORTS	0x00040000U	/* ports stats supported */
#define CNUPM_FLAG_MAJOR(x)	((x) & 0xFF)	/* major version */
#define CNUPM_FLAG_MINOR(x)	(((x) >> 8) & 0xFF)
						/* minor version */

#define CNUPM_PIDFILE_CHECK	0		/* check for pidfile */
#define CNUPM_PIDFILE_CREATE	1		/* create pidfile */
#define CNUPM_PIDFILE_REMOVE	2		/* remove pidfile */

#ifdef INET6
#define CNUPM_SNAPLEN		96
#else
#define CNUPM_SNAPLEN		68
#endif

#if defined(PORTS) && !defined(PROTO)
#define PROTO
#endif

__BEGIN_DECLS
int	cnupm_pidfile(const char *, int);
__END_DECLS

#endif	/* __CNUPM_H__ */
