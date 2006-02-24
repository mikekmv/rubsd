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

#include <sys/time.h>
#include <stdlib.h>

#include "ircyka/ircyka.h"


struct ircyka_alarm_list ircyka_alarms = LIST_HEAD_INITIALIZER(&ircyka_alarms);
int ircyka_got_alarm;


void
ircyka_alarm(void)
{
	struct ircyka_alarm *ia;

	LIST_FOREACH(ia, &ircyka_alarms, ia_entry)
		ia->ia_exec(ia->ia_arg);
	ircyka_got_alarm = 0;
}

struct ircyka_alarm *
alarm_add(void (*proc)(void *), void *arg)
{
	static struct itimerval it = {
		{ IRCYKA_ALARM_INTERVAL, 0 },
		{ IRCYKA_ALARM_INTERVAL, 0 },
	};
	struct ircyka_alarm *ia;

	if ((ia = malloc(sizeof (*ia))) != NULL) {
		ia->ia_exec = proc;
		ia->ia_arg = arg;
		if (LIST_EMPTY(&ircyka_alarms))
			(void)setitimer(ITIMER_REAL, &it, NULL);
		LIST_INSERT_HEAD(&ircyka_alarms, ia, ia_entry);
	}

	return (ia);
}

void
alarm_del(struct ircyka_alarm *ia)
{
	static struct itimerval it;

	LIST_REMOVE(ia, ia_entry);
	free(ia);

	if (LIST_EMPTY(&ircyka_alarms))
		(void)setitimer(ITIMER_REAL, &it, NULL);
}
