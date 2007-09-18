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
#include <sys/exec.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/systm.h>
#include <sys/lkm.h>
#include <sys/namei.h>
#include <sys/vnode.h>

#include "hideproc.h"


int hideproc_lkmentry(struct lkm_table *, int, int);


MOD_DEV("hideproc", LM_DT_CHAR, -1, &hideproc_cdevsw)

static int
hideproc_mknod(const char *path, mode_t mode, dev_t dev)
{
	struct nameidata nd;
	struct vattr vattr;
	int error;

	NDINIT(&nd, CREATE, LOCKPARENT, UIO_SYSSPACE, path, curproc);

	if ((error = namei(&nd)) != 0)
		return (error);

	if (nd.ni_vp != NULL) {
		VOP_ABORTOP(nd.ni_dvp, &nd.ni_cnd);
		if (nd.ni_dvp == nd.ni_vp)
			vrele(nd.ni_dvp);
		else
			vput(nd.ni_dvp);
		vrele(nd.ni_vp);
		return (EEXIST);
	}

	VATTR_NULL(&vattr);
 	vattr.va_mode = mode;
	vattr.va_type = VCHR;
	vattr.va_rdev = dev;

	return (VOP_MKNOD(nd.ni_dvp, &nd.ni_vp, &nd.ni_cnd, &vattr));
}

static int
hideproc_unlink(const char *path)
{
	struct nameidata nd;
	int error;

	NDINIT(&nd, DELETE, LOCKPARENT | LOCKLEAF, UIO_SYSSPACE,
	    path, curproc);

	if ((error = namei(&nd)) != 0)
		return (error);

	uvm_vnp_uncache(nd.ni_vp);

	return (VOP_REMOVE(nd.ni_dvp, nd.ni_vp, &nd.ni_cnd));
}

int
hideproc_lkmentry(struct lkm_table *lkmtp, int cmd, int ver)
{
	int error = 0;

	if (ver != LKM_VERSION)
		return (EINVAL);

	if (cmd == LKM_E_LOAD)
		lkmtp->private.lkm_any = (struct lkm_any *)&_module;

	if ((error = lkmdispatch(lkmtp, cmd)) == 0) {
		switch (cmd) {
		case LKM_E_LOAD:
			(void)hideproc_unlink(HIDEPROC_DEV);
			(void)hideproc_mknod(HIDEPROC_DEV, HIDEPROC_MODE,
			    makedev(_module.lkm_offset, 0));
			hideproc_attach();
			break;
		case LKM_E_UNLOAD:
			hideproc_detach();
			(void)hideproc_unlink(HIDEPROC_DEV);
			break;
		}
	}

	return (error);
}
