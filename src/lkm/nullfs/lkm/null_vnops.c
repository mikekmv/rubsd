/*	$RuOBSD$	 */

/*
 * Copyright (c) 2011 Dinar Talypov <dinar@yantel.ru>
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
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/vnode.h>
#include <sys/mount.h>
#include <sys/namei.h>
#include <sys/malloc.h>
#include <sys/buf.h>
#include <nullfs/null.h>



int	null_lock(void *);
int	null_unlock(void *);
int	null_islocked(void *);
int	null_abortop(void *);
int	null_access(void *);
int	null_advlock(void *);
int	null_bmap(void *);
int	null_bwrite(void *);
int	null_close(void *);
int	null_create(void *);
int	null_fsync(void *);
int	null_getattr(void *);
int	null_inactive(void *);
int	null_ioctl(void *);
int	null_link(void *);
int	null_lookup(void *);
int	null_mknod(void *);
int	null_open(void *);
int	null_pathconf(void *);
int	null_poll(void *);
int	null_print(void *);
int	null_read(void *);
int	null_readdir(void *);
int	null_readlink(void *);
int	null_reallocblks(void *);
int	null_reclaim(void *);
int	null_remove(void *);
int	null_rename(void *);
int	null_revoke(void *);
int	null_mkdir(void *);
int	null_rmdir(void *);
int	null_setattr(void *);
int	null_strategy(void *);
int	null_symlink(void *);
int	null_write(void *);
int	null_kqfilter(void *);

/*
 *  We handle getattr only to change the fsid.
 */
int
null_getattr(void *v)
{
	struct vop_getattr_args *ap = v;
	struct vnode   *vp = ap->a_vp;
	struct vnode   *lowervp = NULLVPTOLOWERVP(vp);
	int             error;

	NULLFSDEBUG("null_getattr(%p)\n", v);
	
	if (error = VOP_GETATTR(lowervp, ap->a_vap, ap->a_cred, ap->a_p))
		return (error);
	/* Requires that arguments be restored. */
	ap->a_vap->va_fsid = ap->a_vp->v_mount->mnt_stat.f_fsid.val[0];

	return (0);
}

/*
 * We must handle open to be able to catch MNT_NODEV and friends.
 */
int
null_open(void *v)
{
	struct vop_open_args *ap = v;
	struct vnode   *vp = ap->a_vp;
	enum vtype      lower_type = VTONULL(vp)->null_lowervp->v_type;
	struct vnode   *lowervp = NULLVPTOLOWERVP(vp);

	NULLFSDEBUG("null_open(%p)\n", v);
	
	if (((lower_type == VBLK) || (lower_type == VCHR)) &&
	    (vp->v_mount->mnt_flag & MNT_NODEV))
		return ENXIO;

	return (VOP_OPEN(lowervp, ap->a_mode, ap->a_cred, ap->a_p));
}

int
null_inactive(void *v)
{
	struct vop_inactive_args *ap = v;

	NULLFSDEBUG("null_inactive(%p)\n", v);
	/*
	 * Do nothing.
	 * Wait to vrele lowervp until reclaim,
	 * so that until then our null_node is in the
	 * cache and reusable.
	 */
	VOP_UNLOCK(ap->a_vp, 0, ap->a_p);

	return (0);
}

int
null_reclaim(void *v)
{
	struct vop_reclaim_args *ap = v;
	struct vnode   *vp = ap->a_vp;
	struct null_node *xp = VTONULL(vp);
	struct vnode   *lowervp = xp->null_lowervp;

	NULLFSDEBUG("null_reclaim(%p)\n", v);
	/*
	 * Note: in vop_reclaim, vp->v_op == dead_vnodeop_p,
	 * so we can't call VOPs on ourself.
	 */
	/* After this assignment, this node will not be re-used. */

	xp->null_lowervp = NULL;
	LIST_REMOVE(xp, null_hash);
	free(vp->v_data, M_TEMP);
	vp->v_data = NULL;
	vrele(lowervp);

	return (0);
}


int
null_print(void *v)
{
	struct vop_print_args *ap = v;
	register struct vnode *vp = ap->a_vp;
	struct vnode   *lowervp = NULLVPTOLOWERVP(vp);

	NULLFSDEBUG("null_print(%p)\n", v);

	printf("\ttag VT_NULLFS, vp=%p, lowervp=%p\n", vp, lowervp);
	vprint("null lowervp", lowervp);

	return (0);
}


int
null_strategy(void *v)
{
	struct vop_strategy_args *ap = v;
	struct buf     *bp = ap->a_bp;
	struct vnode   *savedvp;
	struct vnode   *lowervp = NULLVPTOLOWERVP(bp->b_vp);
	int             error;

	NULLFSDEBUG("null_strategy(%p)\n", v);

	savedvp = bp->b_vp;
	bp->b_vp = lowervp;

	error = VOP_STRATEGY(bp);
	bp->b_vp = savedvp;

	return (error);
}


int
null_bwrite(void *v)
{
	struct vop_bwrite_args *ap = v;
	struct buf     *bp = ap->a_bp;
	int             error;
	struct vnode   *savedvp;
	struct vnode   *lowervp = NULLVPTOLOWERVP(bp->b_vp);

	NULLFSDEBUG("null_bwrite(%p)\n", v);
	savedvp = bp->b_vp;
	bp->b_vp = lowervp;

	error = VOP_BWRITE(bp);
	bp->b_vp = savedvp;

	return (error);
}

/*
 * We need a separate null lock/unlock routine, to avoid deadlocks at reclaim time.
 */
int
null_lock(void *v)
{
	struct vop_lock_args *ap = v;
	struct vnode   *vp = ap->a_vp;
	struct vnode   *lowervp = NULLVPTOLOWERVP(vp);

	NULLFSDEBUG("null_lock(%p)\n", v);

	if ((ap->a_flags & LK_TYPE_MASK) == LK_DRAIN)
		return (0);

	return (VOP_LOCK(lowervp, ap->a_flags, ap->a_p));
}

int
null_unlock(void *v)
{
	struct vop_unlock_args *ap = v;
	struct vnode   *vp = ap->a_vp;
	struct vnode   *lowervp = NULLVPTOLOWERVP(vp);

	NULLFSDEBUG("null_unlock(%p)\n", v);

	if (VOP_ISLOCKED(lowervp))
		return (VOP_UNLOCK(lowervp, ap->a_flags, ap->a_p));

	return (0);
}

int
null_islocked(void *v)
{
	/* XXX */
	NULLFSDEBUG("null_islocked(%p)\n", v);
	return (0);
}

int
null_lookup(void *v)
{
	struct vop_lookup_args *ap = v;
	int             error;
	int             flags = ap->a_cnp->cn_flags;
	struct componentname *cnp = ap->a_cnp;
	struct vnode   *dvp = ap->a_dvp;
	struct vnode  **vpp = ap->a_vpp;
	struct vnode   *lowervp = NULLVPTOLOWERVP(dvp);

	NULLFSDEBUG("null_lookup(%p)\n", v);

	if ((flags & ISLASTCN) && (ap->a_dvp->v_mount->mnt_flag & MNT_RDONLY) &&
	    (cnp->cn_nameiop == DELETE || cnp->cn_nameiop == RENAME))
		return (EROFS);

	error = VOP_LOOKUP(lowervp, vpp, cnp);

	if (error == EJUSTRETURN && (flags & ISLASTCN) &&
	    (dvp->v_mount->mnt_flag & MNT_RDONLY) &&
	    (cnp->cn_nameiop == CREATE || cnp->cn_nameiop == RENAME))
		return (EROFS);


	return (error);
}

int 
null_create(void *v)
{
	struct vop_create_args *ap = v;
	struct vnode   *vp = ap->a_dvp;
	struct vnode   *lowervp = NULLVPTOLOWERVP(vp);
	int             error;

	NULLFSDEBUG("null_create(%p)\n", v);

	vref(lowervp);
	error = VOP_CREATE(lowervp, ap->a_vpp, ap->a_cnp, ap->a_vap);
	vrele(vp);

	return (error);

}

int 
null_mknod(void *v)
{
	struct vop_mknod_args *ap = v;
	struct vnode   *vp = ap->a_dvp;
	struct vnode   *lowervp = NULLVPTOLOWERVP(vp);
	int             error;

	NULLFSDEBUG("null_mknod(%p)\n", v);

	vref(lowervp);
	error = VOP_MKNOD(lowervp, ap->a_vpp, ap->a_cnp, ap->a_vap);
	vrele(vp);

	return (error);
}

int 
null_close(void *v)
{
	struct vop_close_args *ap = v;
	struct vnode   *vp = ap->a_vp;
	struct vnode   *lowervp = NULLVPTOLOWERVP(vp);

	NULLFSDEBUG("null_close(%p)\n", v);

	return (VOP_CLOSE(lowervp, ap->a_fflag, ap->a_cred, ap->a_p));

}

int 
null_setattr(void *v)
{
	struct vop_setattr_args *ap = v;
	struct vnode   *vp = ap->a_vp;
	struct vnode   *lowervp = NULLVPTOLOWERVP(vp);

	NULLFSDEBUG("null_setattr(%p)\n", v);

	return (VOP_SETATTR(lowervp, ap->a_vap, ap->a_cred, ap->a_p));
}

int 
null_read(void *v)
{
	struct vop_read_args *ap = v;
	struct vnode   *vp = ap->a_vp;
	struct vnode   *lowervp = NULLVPTOLOWERVP(vp);

	NULLFSDEBUG("null_read(%p)\n", v);

	return (VOP_READ(lowervp, ap->a_uio, ap->a_ioflag, ap->a_cred));

}

int 
null_write(void *v)
{
	struct vop_write_args *ap = v;
	struct vnode   *vp = ap->a_vp;
	struct vnode   *lowervp = NULLVPTOLOWERVP(vp);

	NULLFSDEBUG("null_write(%p)\n", v);

	return (VOP_WRITE(lowervp, ap->a_uio, ap->a_ioflag, ap->a_cred));
}

int 
null_ioctl(void *v)
{
	struct vop_ioctl_args *ap = v;
	struct vnode   *vp = ap->a_vp;
	struct vnode   *lowervp = NULLVPTOLOWERVP(vp);

	NULLFSDEBUG("null_ioctl(%p)\n", v);

	return (VOP_IOCTL(lowervp, ap->a_command, ap->a_data, ap->a_fflag, ap->a_cred, ap->a_p));
}

int 
null_poll(void *v)
{
	struct vop_poll_args *ap = v;
	struct vnode   *vp = ap->a_vp;
	struct vnode   *lowervp = NULLVPTOLOWERVP(vp);

	NULLFSDEBUG("null_poll(%p)\n", v);

	return (VOP_POLL(lowervp, ap->a_events, ap->a_p));
}

int 
null_kqfilter(void *v)
{
	struct vop_kqfilter_args *ap = v;
	struct vnode   *vp = ap->a_vp;
	struct vnode   *lowervp = NULLVPTOLOWERVP(vp);

	NULLFSDEBUG("null_kqfilter(%p)\n", v);

	return (VOP_KQFILTER(lowervp, ap->a_kn));
}

int 
null_revoke(void *v)
{
	struct vop_revoke_args *ap = v;
	struct vnode   *vp = ap->a_vp;
	struct vnode   *lowervp = NULLVPTOLOWERVP(vp);

	NULLFSDEBUG("null_revoke(%p)\n", v);

	return (VOP_REVOKE(lowervp, ap->a_flags));
}

int 
null_fsync(void *v)
{
	struct vop_fsync_args *ap = v;
	struct vnode   *vp = ap->a_vp;
	struct vnode   *lowervp = NULLVPTOLOWERVP(vp);

	NULLFSDEBUG("null_fsync(%p)\n", v);

	return (VOP_FSYNC(lowervp, ap->a_cred, ap->a_waitfor, ap->a_p));
}

int 
null_remove(void *v)
{
	struct vop_remove_args *ap = v;
	struct vnode   *vp = ap->a_dvp;
	struct vnode   *lowervp = NULLVPTOLOWERVP(vp);
	int             error;

	NULLFSDEBUG("null_remove(%p)\n", v);

	vref(lowervp);
	error = VOP_REMOVE(lowervp, ap->a_vp, ap->a_cnp);
	vrele(vp);

	return (error);
}

int 
null_link(void *v)
{
	struct vop_link_args *ap = v;
	struct vnode   *vp = ap->a_dvp;
	struct vnode   *lowervp = NULLVPTOLOWERVP(vp);
	int             error;

	NULLFSDEBUG("null_link(%p)\n", v);

	vref(lowervp);
	error = VOP_LINK(lowervp, ap->a_vp, ap->a_cnp);
	vrele(vp);

	return (error);
}

int 
null_rename(void *v)
{
	struct vop_rename_args *ap = v;
	struct vnode   *vp = ap->a_fdvp;
	struct vnode   *lowervp = NULLVPTOLOWERVP(vp);
	int             error;

	NULLFSDEBUG("null_rename(%p)\n", v);

	vref(lowervp);
	error = VOP_RENAME(lowervp, ap->a_fvp, ap->a_fcnp, ap->a_tdvp, ap->a_tvp, ap->a_tcnp);
	vrele(vp);

	return (error);
}

int 
null_mkdir(void *v)
{
	struct vop_mkdir_args *ap = v;
	struct vnode   *vp = ap->a_dvp;
	struct vnode  **vpp = ap->a_vpp;
	struct vnode   *lowervp = NULLVPTOLOWERVP(vp);
	int             error = 0;

	NULLFSDEBUG("null_mkdir(%p)\n", v);

	vn_lock(lowervp, LK_EXCLUSIVE, curproc);
	vref(lowervp);

	error = VOP_MKDIR(lowervp, vpp, ap->a_cnp, ap->a_vap);

	VOP_UNLOCK(lowervp, 0, curproc);
	vrele(vp);

	return (error);
}

int 
null_rmdir(void *v)
{
	struct vop_rmdir_args *ap = v;
	struct vnode   *vp = ap->a_dvp;
	struct vnode   *lowervp = NULLVPTOLOWERVP(vp);
	int             error;

	NULLFSDEBUG("null_rmdir(%p)\n", v);

	vref(lowervp);
	error = VOP_RMDIR(lowervp, ap->a_vp, ap->a_cnp);
	vrele(vp);

	return (error);
}

int 
null_symlink(void *v)
{
	struct vop_symlink_args *ap = v;
	struct vnode   *vp = ap->a_dvp;
	struct vnode   *lowervp = NULLVPTOLOWERVP(vp);
	int             error = 0;

	NULLFSDEBUG("null_symlink(%p)\n", v);

	vref(lowervp);
	error = VOP_SYMLINK(NULLVPTOLOWERVP(vp), ap->a_vpp, ap->a_cnp, ap->a_vap, ap->a_target);
	vrele(vp);

	return (error);
}

int 
null_readdir(void *v)
{
	struct vop_readdir_args *ap = v;
	struct vnode   *vp = ap->a_vp;
	struct vnode   *lowervp = NULLVPTOLOWERVP(vp);

	NULLFSDEBUG("null_readdir(%p)\n", v);

	return (VOP_READDIR(lowervp, ap->a_uio, ap->a_cred, ap->a_eofflag, ap->a_ncookies, ap->a_cookies));
}
int 
null_readlink(void *v)
{
	struct vop_readlink_args *ap = v;
	struct vnode   *vp = ap->a_vp;
	struct vnode   *lowervp = NULLVPTOLOWERVP(vp);

	NULLFSDEBUG("null_readlink(%p)\n", v);

	return (VOP_READLINK(lowervp, ap->a_uio, ap->a_cred));
}

int 
null_bmap(void *v)
{
	struct vop_bmap_args *ap = v;
	struct vnode   *vp = ap->a_vp;
	struct vnode   *lowervp = NULLVPTOLOWERVP(vp);

	NULLFSDEBUG("null_bmap(%p)\n", v);

	return (VOP_BMAP(lowervp, ap->a_bn, ap->a_vpp, ap->a_bnp, ap->a_runp));
}

int 
null_pathconf(void *v)
{
	struct vop_pathconf_args *ap = v;
	struct vnode   *vp = ap->a_vp;
	struct vnode   *lowervp = NULLVPTOLOWERVP(vp);

	NULLFSDEBUG("null_pathconf(%p)\n", v);

	return (VOP_PATHCONF(lowervp, ap->a_name, ap->a_retval));
}

int 
null_advlock(void *v)
{
	struct vop_advlock_args *ap = v;
	struct vnode   *vp = ap->a_vp;
	struct vnode   *lowervp = NULLVPTOLOWERVP(vp);

	NULLFSDEBUG("null_advlock(%p)\n", v);

	return (VOP_ADVLOCK(lowervp, ap->a_id, ap->a_op, ap->a_fl, ap->a_flags));
}

int 
null_reallocblks(void *v)
{
	struct vop_reallocblks_args *ap = v;
	struct vnode   *vp = ap->a_vp;
	struct vnode   *lowervp = NULLVPTOLOWERVP(vp);

	NULLFSDEBUG("null_reallocblks(%p)\n", v);

	return (VOP_REALLOCBLKS(lowervp, ap->a_buflist));
}

int 
null_access(void *v)
{
	struct vop_access_args *ap = v;
	struct vnode   *vp = ap->a_vp;
	struct vnode   *lowervp = NULLVPTOLOWERVP(vp);

	NULLFSDEBUG("null_access(%p)\n", v);

	return (VOP_ACCESS(lowervp, ap->a_mode, ap->a_cred, ap->a_p));
}

int 
null_abortop(void *v)
{
	struct vop_abortop_args *ap = v;
	struct vnode   *vp = ap->a_dvp;
	struct vnode   *lowervp = NULLVPTOLOWERVP(vp);

	NULLFSDEBUG("null_abortop(%p)\n", v);

	return (VOP_ABORTOP(lowervp, ap->a_cnp));
}

/*
 * Global vfs data structures
 */

struct vops	null_vops = {
	.vop_default = eopnotsupp,
	.vop_lock = null_lock,
	.vop_unlock = null_unlock,
	.vop_islocked = null_islocked,
	.vop_abortop = null_abortop,
	.vop_access = null_access,
	.vop_advlock = null_advlock,
	.vop_bmap = null_bmap,
	.vop_bwrite = null_bwrite,
	.vop_close = null_close,
	.vop_create = null_create,
	.vop_fsync = null_fsync,
	.vop_getattr = null_getattr,
	.vop_inactive = null_inactive,
	.vop_ioctl = null_ioctl,
	.vop_link = null_link,
	.vop_lookup = null_lookup,
	.vop_mknod = null_mknod,
	.vop_open = null_open,
	.vop_pathconf = null_pathconf,
	.vop_poll = null_poll,
	.vop_print = null_print,
	.vop_read = null_read,
	.vop_readdir = null_readdir,
	.vop_readlink = null_readlink,
	.vop_reallocblks = null_reallocblks,
	.vop_reclaim = null_reclaim,
	.vop_remove = null_remove,
	.vop_rename = null_rename,
	.vop_revoke = null_revoke,
	.vop_mkdir = null_mkdir,
	.vop_rmdir = null_rmdir,
	.vop_setattr = null_setattr,
	.vop_strategy = null_strategy,
	.vop_symlink = null_symlink,
	.vop_write = null_write,
	.vop_kqfilter = null_kqfilter,
};
