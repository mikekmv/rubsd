/*	$RuOBSD: ipflow_proc.c,v 1.5 2005/11/17 19:30:03 form Exp $	*/

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
#include <sys/kernel.h>
#include <sys/conf.h>
#include <sys/systm.h>
#include <sys/tree.h>
#include <sys/proc.h>
#include <sys/malloc.h>
#include <sys/kthread.h>
#include <sys/signalvar.h>
#include <sys/syslog.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/ipflow.h>
#include <net/bpf.h>


#define BPF_MAJOR		23
#define BPF_MIN			32768
#define BPF_MAX			65535
#define BPF_TIMEOUT		1


struct proc *ipflow_proc;
struct ipflow_if_list ipflow_if_list;
struct ipflow_info ipflow_info = { 0, 0, IPFLOW_DEF_FLOWS, 0 };


void
ipflowd_master(void *arg)
{
	struct ipflow_if *ii;

	log(LOG_NOTICE, "ipflowd[%d]: traffic collector started\n",
	    ipflow_proc->p_pid);

	(void)tsleep(arg, PVM, "ipflow", 0);

	while ((ii = LIST_FIRST(&ipflow_if_list)) != NULL)
		ipflow_if_destroy(ii);

	log(LOG_NOTICE, "ipflowd[%d]: traffic collector stopped\n",
	    ipflow_proc->p_pid);

	wakeup(arg);
	kthread_exit(0);
}

void
ipflowd_iface(void *arg)
{
	struct ipflow_if *ii = arg;
	struct iovec iov;
	struct uio uio;
	int unit, s;

	for (unit = BPF_MIN; unit <= BPF_MAX; unit++) {
		ii->ii_dev = makedev(BPF_MAJOR, unit);
		if ((ii->ii_error = bpfopen(ii->ii_dev, 0, 0,
		    ii->ii_proc)) != EBUSY)
			break;
	}

	if (ii->ii_error == 0) {
		struct ifreq ifr;

		bzero(&ifr, sizeof(ifr));
		(void)strlcpy(ifr.ifr_name, ii->ii_name, sizeof(ifr.ifr_name));
		ii->ii_error = bpfioctl(ii->ii_dev, BIOCSETIF, (caddr_t)&ifr,
		    0, ii->ii_proc);
	}

	if (ii->ii_error == 0) {
		ii->ii_error = bpfioctl(ii->ii_dev, BIOCGDLT,
		    (caddr_t)&ii->ii_dlt, 0, ii->ii_proc);
		if (ii->ii_error == 0) {
			ii->ii_collect = ipflow_lookup_collector(ii->ii_dlt);
			if (ii->ii_collect == NULL)
				ii->ii_error = ENODEV;
		}
	}

	if (ii->ii_error == 0) {
		struct timeval tv;

		tv.tv_sec = BPF_TIMEOUT;
		tv.tv_usec = 0;
		ii->ii_error = bpfioctl(ii->ii_dev, BIOCSRTIMEOUT,
		    (caddr_t)&tv, 0, ii->ii_proc);
	}

	if (ii->ii_error == 0) {
		ii->ii_error = bpfioctl(ii->ii_dev, BIOCGBLEN,
		    (caddr_t)&ii->ii_len, 0, ii->ii_proc);
		if (ii->ii_error == 0) {
			ii->ii_buf = malloc(ii->ii_len, M_DEVBUF, M_NOWAIT);
			if (ii->ii_buf == NULL)
				ii->ii_error = ENOMEM;
		}
	}

	if (ii->ii_error != 0) {
		(void)bpfclose(ii->ii_dev, 0, 0, ii->ii_proc);
		wakeup(arg);
		kthread_exit(0);
	}

	s = splvm();
	LIST_INSERT_HEAD(&ipflow_if_list, ii, ii_list);
	splx(s);

	log(LOG_NOTICE, "ipflowd[%d]: attached interface %s\n",
	    ii->ii_proc->p_pid, ii->ii_name);

	wakeup(ii);

	uio.uio_iov = &iov;
	uio.uio_iovcnt = 1;
	uio.uio_segflg = UIO_SYSSPACE;
	uio.uio_rw = UIO_READ;
	uio.uio_procp = curproc;

	for (;;) {
		int signo;

		iov.iov_base = ii->ii_buf;
		iov.iov_len = ii->ii_len;
		uio.uio_offset = 0;
		uio.uio_resid = ii->ii_len;
		ii->ii_error = bpfread(ii->ii_dev, &uio, 0);

		if (ii->ii_error != 0 && ii->ii_error != -1)
			(void)tsleep(ii, PVM, "ipflow", hz);
		else if (uio.uio_resid != ii->ii_len) {
			size_t len = ii->ii_len - uio.uio_resid;
			struct bpf_hdr *bh = ii->ii_buf;
			void *ep = (caddr_t)bh + len;

			while ((void *)bh < ep) {
				ii->ii_collect((caddr_t)bh + bh->bh_hdrlen,
				    bh->bh_caplen);
				len -= bh->bh_hdrlen + bh->bh_caplen;
				bh = (struct bpf_hdr *)((caddr_t)bh +
				    BPF_WORDALIGN(bh->bh_hdrlen +
				    bh->bh_caplen));
			}
		}

		while ((signo = CURSIG(ii->ii_proc)) != 0) {
			CLRSIG(ii->ii_proc, signo);
			if (signo == SIGTERM || signo == SIGKILL)
				break;
			if (signo == SIGHUP) {
				(void)bpfioctl(ii->ii_dev, BIOCFLUSH, NULL,
				    0, ii->ii_proc);
				wakeup(ii);
			}
		}
		if (signo != 0)
			break;
	}

	(void)bpfclose(ii->ii_dev, 0, 0, ii->ii_proc);
	free(ii->ii_buf, M_DEVBUF);

	s = splvm();
	LIST_REMOVE(ii, ii_list);
	splx(s);

	log(LOG_NOTICE, "ipflowd[%d]: detached interface %s\n",
	    ii->ii_proc->p_pid, ii->ii_name);

	wakeup(arg);
	kthread_exit(0);
}

struct ipflow_if *
ipflow_if_lookup(const char *name)
{
	struct ipflow_if *ii;
	int s;

	s = splvm();
	LIST_FOREACH(ii, &ipflow_if_list, ii_list)
		if (strcmp(ii->ii_name, name) == 0)
			break;
	splx(s);

	return (ii);
}

int
ipflow_if_create(const char *name)
{
	struct ipflow_if *ii;
	int error;

	if (ipflow_if_lookup(name) != NULL)
		return (EEXIST);

	if ((ii = malloc(sizeof(*ii), M_DEVBUF, M_NOWAIT)) == NULL)
		return (ENOMEM);
	bzero(ii, sizeof(*ii));
	(void)strlcpy(ii->ii_name, name, sizeof(ii->ii_name));

	if ((error = kthread_create(ipflowd_iface, ii, &ii->ii_proc,
	    "ipflowd: %s", ii->ii_name)) == 0) {
		(void)tsleep(ii, PVM, "ipflow", 0);
		error = ii->ii_error;
	}

	if (error != 0)
		free(ii, M_DEVBUF);

	return (error);
}

void
ipflow_if_destroy(struct ipflow_if *ii)
{
	psignal(ii->ii_proc, SIGKILL);
	wakeup(ii);
	(void)tsleep(ii, PVM, "ipflow", 0);
	free(ii, M_DEVBUF);
}
