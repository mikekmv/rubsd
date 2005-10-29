/*	$RuOBSD: ipflow_dev.c,v 1.2 2005/03/29 06:59:52 form Exp $	*/

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
#include <sys/systm.h>
#include <sys/tree.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/proc.h>
#include <sys/kthread.h>
#include <sys/socket.h>
#include <sys/signalvar.h>
#include <net/if.h>
#include <net/bpf.h>

#include "ipflow.h"


static void filt_ipflow_read_detach(struct knote *);
static void filt_ipflow_write_detach(struct knote *);
static int filt_ipflow_read(struct knote *, long);
static int filt_ipflow_write(struct knote *, long);


static struct filterops ipflow_read_filtops =
    { 1, NULL, filt_ipflow_read_detach, filt_ipflow_read };
static struct filterops ipflow_write_filtops =
    { 1, NULL, filt_ipflow_write_detach, filt_ipflow_write };

struct selinfo ipflow_rsel;
struct selinfo ipflow_wsel;


int
ipflowattach(void)
{
	int error = 0;

	if ((error = ipflow_init()) != 0)
		return (error);

	if ((error = kthread_create(ipflowd_master, &ipflow_cdevsw,
	    &ipflow_proc, "ipflowd: master")) == 0)
		ipflow_pid = ipflow_proc->p_pid;
	else
		ipflow_free();

	return (error);
}

void
ipflowdetach(void)
{
	wakeup(&ipflow_cdevsw);
	(void)tsleep(&ipflow_cdevsw, PVM, "ipflow", 0);
	ipflow_free();
}

int
ipflowopen(dev_t dev, int oflags, int devtype, struct proc *p)
{
	return (minor(dev) ? ENXIO : 0);
}

int
ipflowclose(dev_t dev, int fflag, int devtype, struct proc *p)
{
	return (0);
}

int
ipflowioctl(dev_t dev, u_long cmd, caddr_t data, int fflag, struct proc *p)
{
	struct bpf_program bprog;
	struct ipflow_ifreq *iir;
	struct ipflow_if *ii;
	struct ipflow_req *irq;
	struct ipflow *ifl;
	int s, error = 0;
	size_t n;

	switch (cmd) {
	case IIOCFFLOWS:
	case IIOCGFLOWS:
	case IIOCSFLOWS:
	case IIOCSNFLOWS:
	case IIOCADDIF:
	case IIOCDELIF:
	case IIOCSETF:
	case IIOCFLUSHIF:
		if ((error = suser(p, 0)) != 0)
			return (error);
		break;
	}

	s = splsoftnet();

	switch (cmd) {
	case IIOCFFLOWS:
		ipflow_flush();
		break;
	case IIOCGFLOWS:
		irq = (struct ipflow_req *)data;
		if (irq->irq_nflows == 0) {
			irq->irq_nflows = ipflow_nflows;
			break;
		}
		if (irq->irq_nflows < ipflow_nflows)
			return (ENOMEM);
		ifl = irq->irq_flows;
		for (n = 0; n < ipflow_nflows; n++) {
			if ((error = copyout(&ipflow_entries[n].ife_flow, ifl,
			    sizeof(*ifl))) != 0)
				break;
			ifl++;
		}
		if (error == 0) {
			irq->irq_nflows = ipflow_nflows;
			ipflow_flush();
		}
		break;
	case IIOCSFLOWS:
		irq = (struct ipflow_req *)data;
		ipflow_flush();
		for (n = 0; n < irq->irq_nflows; n++) {
			struct ipflow ifl;

			if ((error = copyin(irq->irq_flows++, &ifl,
			    sizeof(ifl))) != 0)
				break;
			(void)ipflow_insert(&ifl);
		}
		break;
	case IIOCGNFLOWS:
		*(u_int *)data = ipflow_maxflows;
		break;
	case IIOCSNFLOWS:
		if (*(u_int *)data < IPFLOW_MIN_FLOWS ||
		    *(u_int *)data > IPFLOW_MAX_FLOWS) {
			error = EINVAL;
			break;
		}
		if (*(u_int *)data < ipflow_nflows) {
			error = EBUSY;
			break;
		}
		error = ipflow_realloc(*(u_int *)data);
		break;
	case IIOCADDIF:
		iir = (struct ipflow_ifreq *)data;
		error = ipflow_if_create(iir->iir_name);
		break;
	case IIOCDELIF:
		iir = (struct ipflow_ifreq *)data;
		if ((ii = ipflow_if_lookup(iir->iir_name)) != NULL)
			ipflow_if_destroy(ii);
		else
			error = ENOENT;
		break;
	case IIOCGIFCONF:
		iir = (struct ipflow_ifreq *)data;
		if ((ii = ipflow_if_lookup(iir->iir_name)) != NULL) {
			struct bpf_stat bs;

			if ((error = bpfioctl(ii->ii_dev, BIOCGSTATS,
			    (caddr_t)&bs, 0, p)) == 0) {
				iir->iir_dev = ii->ii_dev;
				iir->iir_dlt = ii->ii_dlt;
				iir->iir_recv = bs.bs_recv;
				iir->iir_drop = bs.bs_drop;
				iir->iir_pid = ii->ii_proc->p_pid;
			}
		} else
			error = ENOENT;
		break;
	case IIOCSETF:
		iir = (struct ipflow_ifreq *)data;
		if ((ii = ipflow_if_lookup(iir->iir_name)) == NULL) {
			error = ENOENT;
			break;
		}
		if ((error = copyin(iir->iir_bprog, &bprog,
		    sizeof(bprog))) != 0)
			break;
		error = bpfioctl(ii->ii_dev, BIOCSETF, (caddr_t)&bprog, 0, p);
		break;
	case IIOCGIFLIST:
		iir = (struct ipflow_ifreq *)data;
		if (iir->iir_count == 0) {
			LIST_FOREACH(ii, &ipflow_if_list, ii_list)
				iir->iir_count++;
			break;
		} else {
			caddr_t uaddr = iir->iir_data;
			size_t count = iir->iir_count;

			iir->iir_count = 0;
			LIST_FOREACH(ii, &ipflow_if_list, ii_list) {
				struct ipflow_ifreq ifrq;
				struct bpf_stat bs;

				if (iir->iir_count >= count) {
					error = ENOMEM;
					break;
				}
				bzero(&ifrq, sizeof(ifrq));
				(void)strlcpy(ifrq.iir_name, ii->ii_name,
				    sizeof(ifrq.iir_name));
				ifrq.iir_dev = ii->ii_dev;
				ifrq.iir_dlt = ii->ii_dlt;
				ifrq.iir_pid = ii->ii_proc->p_pid;
				if ((error = bpfioctl(ii->ii_dev, BIOCGSTATS,
				    (caddr_t)&bs, 0, p)) != 0)
					break;
				ifrq.iir_recv = bs.bs_recv;
				ifrq.iir_drop = bs.bs_drop;
				if ((error = copyout(&ifrq, uaddr,
				    sizeof(ifrq))) != 0)
					break;
				uaddr += sizeof(ifrq);
				iir->iir_count++;
			}
		}
		break;
	case IIOCFLUSHIF:
		iir = (struct ipflow_ifreq *)data;
		if ((ii = ipflow_if_lookup(iir->iir_name)) == NULL) {
			error = ENOENT;
			break;
		}
		psignal(ii->ii_proc, SIGHUP);
		(void)tsleep(ii, PVM, "ipflow", 0);
		break;
	case IIOCVERSION:
		((struct ipflow_version *)data)->iv_major =
		    IPFLOW_VERSION_MAJOR;
		((struct ipflow_version *)data)->iv_minor =
		    IPFLOW_VERSION_MINOR;
		break;
	case IIOCGINFO:
		bcopy(&ipflow_info, data, sizeof(ipflow_info));
		break;
	default:
		error = EINVAL;
		break;
	}

	splx(s);

	return (error);
}

int
ipflowread(dev_t dev, struct uio *uio, int ioflag)
{
	int error = 0;
	size_t n;

	if (uio->uio_resid % sizeof(struct ipflow) != 0)
		return (EINVAL);

	if (uio->uio_resid < ipflow_nflows * sizeof(struct ipflow))
		error = ENOMEM;
	else
		for (n = 0; n < ipflow_nflows; n++)
			if ((error = uiomove(&ipflow_entries[n].ife_flow,
			    sizeof(ipflow_entries[n].ife_flow), uio)) != 0)
				break;

	return (error);
}

int
ipflowwrite(dev_t dev, struct uio *uio, int ioflag)
{
	struct ipflow ifl;
	int error = 0;

	if (uio->uio_resid % sizeof(struct ipflow) != 0)
		return (EINVAL);

	while (uio->uio_resid > 0) {
		if ((error = uiomove(&ifl, sizeof(ifl), uio)) != 0)
			break;
		ipflow_insert(&ifl);
	}

	return (error);
}

int
ipflowpoll(dev_t dev, int events, struct proc *p)
{
	int revents;

	revents = events & (POLLOUT | POLLWRNORM);

	if (events & (POLLIN | POLLRDNORM)) {
		if (ipflow_nflows == 0)
			selrecord(p, &ipflow_rsel);
		else
			revents |= events & (POLLIN | POLLRDNORM);
	}

	return (revents);
}

int
ipflowkqfilter(dev_t dev, struct knote *kn)
{
	struct klist *klist;
	int s;

	switch (kn->kn_filter) {
	case EVFILT_READ:
		klist = &ipflow_rsel.si_note;
		kn->kn_fop = &ipflow_read_filtops;
		break;
	case EVFILT_WRITE:
		klist = &ipflow_wsel.si_note;
		kn->kn_fop = &ipflow_write_filtops;
		break;
	default:
		return (1);
	}

	s = splimp();
	SLIST_INSERT_HEAD(klist, kn, kn_selnext);
	splx(s);

	return (0);
}

void
filt_ipflow_read_detach(struct knote *kn)
{
	int s;

	s = splhigh();
	SLIST_REMOVE(&ipflow_rsel.si_note, kn, knote, kn_selnext);
	splx(s);
}

int
filt_ipflow_read(struct knote *kn, long hint)
{
	return ((kn->kn_data = ipflow_nflows) > 0);
}

void
filt_ipflow_write_detach(struct knote *kn)
{
	int s;

	s = splhigh();
	SLIST_REMOVE(&ipflow_wsel.si_note, kn, knote, kn_selnext);
	splx(s);
}

int
filt_ipflow_write(struct knote *kn, long hint)
{
	kn->kn_data = ipflow_maxflows;

	return (1);
}


struct cdevsw ipflow_cdevsw = {
	ipflowopen,
	ipflowclose,
	ipflowread,
	ipflowwrite,
	ipflowioctl,
	(dev_type_stop((*)))enodev,
	NULL,
	ipflowpoll,
	(dev_type_mmap((*)))enodev,
	D_KQFILTER,
	ipflowkqfilter
};
