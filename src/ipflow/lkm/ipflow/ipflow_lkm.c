/*	$RuOBSD$	*/

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

#include <sys/param.h>
#include <sys/conf.h>
#include <sys/exec.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/systm.h>
#include <sys/lkm.h>
#include <sys/kthread.h>
#include <sys/stat.h>
#include <sys/namei.h>
#include <sys/vnode.h>
#include <sys/socket.h>
#include <net/if.h>

#include "ipflow.h"


MOD_DEV("ipflow", LM_DT_CHAR, -1, &ipflow_cdevsw)


extern int lkmexists(struct lkm_table *);

int ipflow_lkmentry(struct lkm_table *, int, int);


static int
ipflow_load(struct lkm_table *lkmtp, int cmd)
{
	int error = 0;

	switch (cmd) {
	case LKM_E_LOAD:
		error = lkmexists(lkmtp) ? EEXIST : ipflowattach();
		break;
	case LKM_E_UNLOAD:
		ipflowdetach();
		break;
	}

	return (error);
}

static int
ipflow_dispatch(struct lkm_table *lkmtp, int cmd, int ver)
{
	DISPATCH(lkmtp, cmd, ver, ipflow_load, ipflow_load, lkm_nofunc)
}

static int
ipflow_mknod(const char *path, mode_t mode, dev_t dev)
{
	struct proc *p = curproc;
	struct vattr vattr;
	struct nameidata nd;
	struct vnode *vp;
	int error;

	NDINIT(&nd, CREATE, LOCKPARENT, UIO_SYSSPACE, path, p);
	if ((error = namei(&nd)) != 0)
		return (error);
	if ((vp = nd.ni_vp) != NULL)
		return (EEXIST);
	VATTR_NULL(&vattr);
	vattr.va_mode = mode & ALLPERMS;
	vattr.va_type = VCHR;
	vattr.va_rdev = dev;
	VOP_LEASE(nd.ni_dvp, p, p->p_ucred, LEASE_WRITE);

	return (VOP_MKNOD(nd.ni_dvp, &nd.ni_vp, &nd.ni_cnd, &vattr));
}

static int
ipflow_unlink(const char *path)
{
	struct proc *p = curproc;
	struct vnode *vp;
	struct nameidata nd;
	int error;

	NDINIT(&nd, DELETE, LOCKPARENT | LOCKLEAF, UIO_SYSSPACE, path, p);
	if ((error = namei(&nd)) != 0)
		return (error);
	vp = nd.ni_vp;
	(void)uvm_vnp_uncache(vp);

	VOP_LEASE(nd.ni_dvp, p, p->p_ucred, LEASE_WRITE);
	VOP_LEASE(vp, p, p->p_ucred, LEASE_WRITE);

	return (VOP_REMOVE(nd.ni_dvp, nd.ni_vp, &nd.ni_cnd));
}

int
ipflow_lkmentry(struct lkm_table *lkmtp, int cmd, int ver)
{
	int error;

	if ((error = ipflow_dispatch(lkmtp, cmd, ver)) != 0)
		return (error);

	switch (cmd) {
	case LKM_E_LOAD:
		(void)ipflow_unlink(_PATH_DEV_IPFLOW);
		(void)ipflow_mknod(_PATH_DEV_IPFLOW, _MODE_DEV_IPFLOW,
		    makedev(_module.lkm_offset, 0));
		break;
	case LKM_E_UNLOAD:
		(void)ipflow_unlink(_PATH_DEV_IPFLOW);
		break;
	}

	return (0);
}
