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

#ifndef __TERM_H__
#define __TERM_H__

#define TERM_KB_REFRESH			1000
#define TERM_KB_HOME			1001
#define TERM_KB_END			1002
#define TERM_KB_UP			1003
#define TERM_KB_DOWN			1004


__BEGIN_DECLS
int	term_open(char *, int);
void	term_close(void);
void	term_setxy(int, int);
void	term_printf(char *, ...);
void	term_flush(void);
void	term_clreol(void);
void	term_clrbot(void);
int	term_reverse(void);
void	term_normal(void);
int	term_nrows(void);
int	term_ncols(void);
int	term_getch(void);
void	term_redraw(void);
__END_DECLS

#endif	/* __TERM_H__ */
