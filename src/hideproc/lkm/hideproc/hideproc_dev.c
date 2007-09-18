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

#include <sys/param.h>
#include <sys/conf.h>
#include <sys/systm.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>

#include "hideproc.h"


u_int32_t hideproc_flags = HPF_ENABLE;
gid_t hideproc_gid = HIDEPROC_GID;


int
hideprocopen(dev_t dev, int oflags, int devtype, struct proc *p)
{
	return (0);
}

int
hideprocclose(dev_t dev, int oflags, int devtype, struct proc *p)
{
	return (0);
}

int
hideprocioctl(dev_t dev, u_long cmd, caddr_t data, int oflags, struct proc *p)
{
	switch (cmd) {
	case HIOCGFLAGS:
		*(u_int32_t *)data = hideproc_flags;
		break;
	case HIOCSFLAGS:
		if (!(oflags & FWRITE))
			return (EPERM);

		hideproc_flags = *(u_int32_t *)data;
		hideproc_setstate();
		break;
	case HIOCGGID:
		*(gid_t *)data = hideproc_gid;
		break;
	case HIOCSGID:
		if (!(oflags & FWRITE))
			return (EPERM);
		hideproc_gid = *(gid_t *)data;
		break;
	case HIOCGVERSION:
		*(u_int32_t *)data = HIDEPROC_VERSION;
		break;
	default:
		return (ENODEV);
	}

	return (0);
}


struct cdevsw hideproc_cdevsw = {
	hideprocopen,
	hideprocclose,
	(dev_type_read((*)))enodev,
	(dev_type_write((*)))enodev,
	hideprocioctl,
	(dev_type_stop((*)))enodev,
	NULL,
	(dev_type_poll((*)))enodev,
	(dev_type_mmap((*)))enodev,
	0,
	0,
	NULL
};
