/*
 * $RuOBSD$
 *
 * Copyright (c) 2005-2006 Oleg Safiullin <form@pdp-11.org.ru>
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
#include <errno.h>
#include <poll.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ircyka/ircyka.h"


struct ircyka_qio_list ircyka_qio;

static struct pollfd pfd;


void
qio_init(int fd)
{
	pfd.fd = fd;
	pfd.events = POLLIN;
	SIMPLEQ_INIT(&ircyka_qio);
}

int
qio_enqueue(const void *data, size_t len)
{
	struct ircyka_qio *iq;

	if (len != 0) {
		if ((iq = malloc(sizeof(*iq) + len)) == NULL)
			return (-1);

		iq->iq_data = iq + 1;
		iq->iq_len = len;
		bcopy(data, iq->iq_data, len);
		SIMPLEQ_INSERT_TAIL(&ircyka_qio, iq, iq_entry);
		pfd.events |= POLLOUT;
	}
	return (0);
}

ssize_t
qio_dequeue(void)
{
	struct ircyka_qio *iq;
	ssize_t nb;
	int save_errno;

	if ((iq = SIMPLEQ_FIRST(&ircyka_qio)) != NULL) {
		SIMPLEQ_REMOVE_HEAD(&ircyka_qio, iq_entry);
		if (ircyka_debug) {
			(void)fprintf(stderr, "> ");
			(void)fflush(stderr);
			(void)write(STDERR_FILENO, iq->iq_data, iq->iq_len);
		}
		charset_translate(iq->iq_data, iq->iq_len);
		nb = write(pfd.fd, iq->iq_data, iq->iq_len);
		save_errno = errno;
		free(iq);
		errno = save_errno;
		return (nb);
	}

	pfd.events &= ~POLLOUT;
	return (0);
}

void
qio_flush(void)
{
	struct ircyka_qio *iq;

	while ((iq = SIMPLEQ_FIRST(&ircyka_qio)) != NULL) {
		SIMPLEQ_REMOVE_HEAD(&ircyka_qio, iq_entry);
		free(iq);
	}
}

ssize_t
qio_poll(void *buf, size_t size)
{
	for (;;) {
		if (poll(&pfd, 1, INFTIM) < 0) {
			if (errno == EINTR) {
				if (ircyka_die)
					return (0);
				if (ircyka_got_alarm) 
					ircyka_alarm();
				continue;
			}
			return (-1);
		}
		if (pfd.revents & POLLOUT) {
			if (qio_dequeue() < 0 && errno != EINTR)
				return (-1);
		}
		if (pfd.revents & POLLIN) {
			ssize_t nb = read(pfd.fd, buf, size);

			if (nb > 0)
				charset_untranslate(buf, size);
			return (nb);
		}
	}
}

int
qio_finish(int timeout)
{
	int rc;

	pfd.events = POLLOUT;
	for (;;) {
		if ((rc = poll(&pfd, 1, timeout)) <= 0)
			return (rc);
		if ((pfd.revents & POLLOUT) && qio_dequeue() <= 0)
			return (-1);
	}
}

int
qio_printf(const char *fmt, ...)
{
	char buf[IRCYKA_MAX_STRLEN];
	size_t len;
	va_list ap;

	va_start(ap, fmt);
	len = vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	if (len >= sizeof(buf)) {
		errno = ENAMETOOLONG;
		return (-1);
	}

	return (qio_enqueue(buf, len));
}
