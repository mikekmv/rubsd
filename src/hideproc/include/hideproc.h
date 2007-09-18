/*	$RuOBSD$	*/

/*
 * Copyright (c) 2004, 2007 Oleg Safiullin <form@pdp-11.org.ru>
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

#ifndef __HIDEPROC_H__
#define __HIDEPROC_H__

#define HIDEPROC_DEV		"/dev/hideproc"
#define HIDEPROC_MODE		0600
#define HIDEPROC_GID		0
#define HIDEPROC_VERSION	0x00000002

#define HPF_ENABLE		0x00000001
#define HPF_DENY		0x00000002
#define HPF_SYSTEM		0x00000004

#define HPV_MAJOR(v)		((v) & 0xffff)
#define HPV_MINOR(v)		((v) >> 16)

#define HIOCGVERSION		_IOR('i', 250, u_int32_t)
#define HIOCGFLAGS		_IOR('i', 251, u_int32_t)
#define HIOCSFLAGS		_IOW('i', 252, u_int32_t)
#define HIOCGGID		_IOR('i', 253, gid_t)
#define HIOCSGID		_IOW('i', 254, gid_t)

#ifdef _KERNEL
extern struct cdevsw hideproc_cdevsw;
extern u_int32_t hideproc_flags;
extern gid_t hideproc_gid;


cdev_decl(hideproc);

void hideproc_attach(void);
void hideproc_detach(void);
void hideproc_setstate(void);
#endif	/* _KERNEL */

#endif	/* __HIDEPROC_H__ */
