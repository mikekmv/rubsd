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
#include <sys/malloc.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/proc.h>
#include <sys/mount.h>
#include <sys/syscall.h>
#include <sys/syscallargs.h>
#include <sys/sysctl.h>
#include <sys/tty.h>

#include "hideproc.h"


#define KERN_PROCSLOP		(5 * sizeof (struct kinfo_proc))


static sy_call_t *system_kill;
static sy_call_t *system_sysctl;


static __inline int
hideproc_trusted(struct proc *p)
{
	int trust;

	if (suser(p, 0) == 0)
		return (1);
	trust = !p->p_ucred->cr_uid || groupmember(hideproc_gid, p->p_ucred);

	return (hideproc_flags & HPF_DENY ? !trust : trust);
}

static __inline int
hideproc_nprocs(uid_t uid)
{
	struct proc *p;
	int count = 0;

	LIST_FOREACH(p, &allproc, p_list)
		if (p->p_cred->p_ruid == uid ||
		    ((hideproc_flags & HPF_SYSTEM) &&
		    ((p->p_flag & P_SYSTEM) || p->p_pid == 1)))
			count++;

	return (count);
}

static int
hideproc_doproc(uid_t uid, int *name, u_int namelen, char *where, size_t *sizep)
{
	struct kinfo_proc2 *kproc2 = NULL;
	struct eproc *eproc = NULL;
	struct proc *p;
	char *dp;
	int arg, buflen, doingzomb, elem_size, elem_count;
	int error, needed, type, op;

	dp = where;
	buflen = where != NULL ? *sizep : 0;
	needed = error = 0;
	type = name[0];

	if (type == KERN_PROC) {
		if (namelen != 3 && !(namelen == 2 &&
		    (name[1] == KERN_PROC_ALL || name[1] == KERN_PROC_KTHREAD)))
			return (EINVAL);
		op = name[1];
		arg = op == KERN_PROC_ALL ? 0 : name[2];
		elem_size = elem_count = 0;
		eproc = malloc(sizeof(struct eproc), M_TEMP, M_WAITOK);
	} else /* if (type == KERN_PROC2) */ {
		if (namelen != 5 || name[3] < 0 || name[4] < 0)
			return (EINVAL);
		op = name[1];
		arg = name[2];
		elem_size = name[3];
		elem_count = name[4];
		kproc2 = malloc(sizeof(struct kinfo_proc2), M_TEMP, M_WAITOK);
	}
	p = LIST_FIRST(&allproc);
	doingzomb = 0;
again:
	for (; p != 0; p = LIST_NEXT(p, p_list)) {
		/*
		 * Skip embryonic processes.
		 */
		if (p->p_stat == SIDL)
			continue;

		/*
		 * Skip hidden processes.
		 */
		if (p->p_cred->p_ruid != uid &&
		    !((hideproc_flags & HPF_SYSTEM) &&
		    ((p->p_flag & P_SYSTEM) || p->p_pid == 1)))
			continue;

		/*
		 * TODO - make more efficient (see notes below).
		 */
		switch (op) {

		case KERN_PROC_PID:
			/* could do this with just a lookup */
			if (p->p_pid != (pid_t)arg)
				continue;
			break;

		case KERN_PROC_PGRP:
			/* could do this by traversing pgrp */
			if (p->p_pgrp->pg_id != (pid_t)arg)
				continue;
			break;

		case KERN_PROC_SESSION:
			if (p->p_session->s_leader == NULL ||
			    p->p_session->s_leader->p_pid != (pid_t)arg)
				continue;
			break;

		case KERN_PROC_TTY:
			if ((p->p_flag & P_CONTROLT) == 0 ||
			    p->p_session->s_ttyp == NULL ||
			    p->p_session->s_ttyp->t_dev != (dev_t)arg)
				continue;
			break;

		case KERN_PROC_UID:
			if (p->p_ucred->cr_uid != (uid_t)arg)
				continue;
			break;

		case KERN_PROC_RUID:
			if (p->p_cred->p_ruid != (uid_t)arg)
				continue;
			break;

		case KERN_PROC_ALL:
			if (p->p_flag & P_SYSTEM)
				continue;
			break;
		case KERN_PROC_KTHREAD:
			/* no filtering */
			break;
		default:
			error = EINVAL;
			goto err;
		}
		if (type == KERN_PROC) {
			if (buflen >= sizeof(struct kinfo_proc)) {
				fill_eproc(p, eproc);
				error = copyout((caddr_t)p,
				    &((struct kinfo_proc *)dp)->kp_proc,
				    sizeof(struct proc));
				if (error)
					goto err;
				error = copyout((caddr_t)eproc,
				    &((struct kinfo_proc *)dp)->kp_eproc,
				    sizeof(*eproc));
				if (error)
					goto err;
				dp += sizeof(struct kinfo_proc);
				buflen -= sizeof(struct kinfo_proc);
			}
			needed += sizeof(struct kinfo_proc);
		} else /* if (type == KERN_PROC2) */ {
			if (buflen >= elem_size && elem_count > 0) {
				fill_kproc2(p, kproc2);
				/*
				 * Copy out elem_size, but not larger than
				 * the size of a struct kinfo_proc2.
				 */
				error = copyout(kproc2, dp,
				    min(sizeof(*kproc2), elem_size));
				if (error)
					goto err;
				dp += elem_size;
				buflen -= elem_size;
				elem_count--;
			}
			needed += elem_size;
		}
	}
	if (doingzomb == 0) {
		p = LIST_FIRST(&zombproc);
		doingzomb++;
		goto again;
	}
	if (where != NULL) {
		*sizep = dp - where;
		if (needed > *sizep) {
			error = ENOMEM;
			goto err;
		}
	} else {
		needed += KERN_PROCSLOP;
		*sizep = needed;
	}
err:
	if (eproc)
		free(eproc, M_TEMP);
	if (kproc2)
		free(kproc2, M_TEMP);
	return (error);
}

static int
hideproc_sysctl(struct proc *p, void *v, register_t *retval)
{
	struct sys___sysctl_args /* {
		syscallarg(int *) name;
		syscallarg(u_int) namelen;
		syscallarg(void *) old;
		syscallarg(size_t *) oldlenp;
		syscallarg(void *) new;
		syscallarg(size_t) newlen;
	} */ *uap = v;
	int error, name[CTL_MAXNAME];
	struct proc *cp;

	if (SCARG(uap, namelen) < 2 || SCARG(uap, namelen) > CTL_MAXNAME)
		return (EINVAL);

	if ((error = copyin(SCARG(uap, name), name,
	    SCARG(uap, namelen) * sizeof(int))) != 0)
		return (error);

	if (name[0] == CTL_KERN && !hideproc_trusted(p)) {
		switch (name[1]) {
		case KERN_NPROCS:
			return (sysctl_rdint(SCARG(uap, old),
			    SCARG(uap, oldlenp), SCARG(uap, new),
			    hideproc_nprocs(p->p_ucred->cr_uid)));
		case KERN_PROC:
		case KERN_PROC2:
			return (hideproc_doproc(p->p_ucred->cr_uid,
			    SCARG(uap, name) + 1, SCARG(uap, namelen) - 1,
			    SCARG(uap, old), SCARG(uap, oldlenp)));
		case KERN_PROC_ARGS:
			if (SCARG(uap, namelen) < 4)
				return (EINVAL);
			if ((cp = pfind((pid_t)name[2])) == NULL ||
			    (cp->p_cred->p_ruid != p->p_ucred->cr_uid &&
			    !((hideproc_flags & HPF_SYSTEM) &&
			    ((cp->p_flag & P_SYSTEM) || cp->p_pid == 1))))
				return (ESRCH);
			break;
		}
	}

	return (system_sysctl(p, v, retval));
}

static int
hideproc_kill(struct proc *p, void *v, register_t *retval)
{
	struct sys_kill_args /* {
		syscallarg(int) pid;
		syscallarg(int) signum;
	} */ *uap = v;
	struct proc *cp;
	int error;

	if ((error = system_kill(p, v, retval)) == EPERM &&
	    !hideproc_trusted(p) && (!(hideproc_flags & HPF_SYSTEM) ||
	    (cp = pfind(SCARG(uap, pid))) == NULL ||
	    !((cp->p_stat & P_SYSTEM) || cp->p_pid == 1)))
		error = ESRCH;

	return (error);
}

void
hideproc_setstate(void)
{
	int s = splsched();

	if ((hideproc_flags & HPF_ENABLE)) {
		sysent[SYS_kill].sy_call = hideproc_kill;
		sysent[SYS___sysctl].sy_call = hideproc_sysctl;
	} else {
		sysent[SYS___sysctl].sy_call = system_sysctl;
		sysent[SYS_kill].sy_call = system_kill;
	}

	splx(s);
}

void
hideproc_attach(void)
{
	system_kill = sysent[SYS_kill].sy_call;
	system_sysctl = sysent[SYS___sysctl].sy_call;
	hideproc_setstate();
}

void
hideproc_detach(void)
{
	sysent[SYS___sysctl].sy_call = system_sysctl;
	sysent[SYS_kill].sy_call = system_kill;
}
