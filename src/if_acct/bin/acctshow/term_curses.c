/*	$RuOBSD$	*/

/*
 * Copyright (c) 2004 Oleg Safiullin <form@pdp-11.org.ru>
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

#include <curses.h>
#include <stdarg.h>
#include <stdlib.h>

#include "term.h"


int
term_open(const char *term, int seconds)
{
	if (term == NULL)
		term = getenv("TERM");
	if (newterm(term == NULL ?
	    "unknown" : (char *)term, stdout, stdin) == 0)
		return (-1);
	def_prog_mode();
	(void)cbreak();
	(void)noecho();
	(void)timeout(seconds * 1000);
	(void)keypad(stdscr, TRUE);
	return (0);
}

void
term_close(void)
{
	(void)endwin();
}

void
term_setxy(int x, int y)
{
	move(y, x);
}

void
term_printf(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	(void)vwprintw(stdscr, (char *)fmt, ap);
	va_end(ap);
}


void
term_flush(void)
{
	(void)doupdate();
}

void
term_clreol(void)
{
	(void)clrtoeol();
}

void
term_clrbot(void)
{
	(void)clrtobot();
}

int
term_reverse(void)
{
	(void)attron(A_REVERSE);
	return (termattrs() & A_REVERSE ? 1 : 0);
}

void
term_normal(void)
{
	(void)attroff(A_REVERSE);
}

int
term_nrows(void)
{
	return (stdscr->_maxy - stdscr->_begy + 1);
}

int
term_ncols(void)
{
	return (stdscr->_maxx - stdscr->_begx + 1);
}

int
term_getch(void)
{
	int ch;

	ch = getch();
	switch (ch) {
	case KEY_UP:
	case KEY_PPAGE:
		ch = 'U';
		break;
	case KEY_DOWN:
	case KEY_NPAGE:
		ch = 'D';
		break;
	case KEY_HOME:
		ch = 'H';
		break;
	case KEY_END:
		ch = 'E';
		break;
	case '\f':
	case '\022':
	case '\027':
		ch = 'R';
		break;
	default:
		ch = 0;
		break;
	}

	return (ch);
}

void
term_redraw(void)
{
	clearok(stdscr, TRUE);
	refresh();
}
