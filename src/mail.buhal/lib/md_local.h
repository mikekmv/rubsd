/*	$RuOBSD$	*/

/*
 * Copyright (c) 2002 Oleg Safiullin <form@pdp11.org.ru>
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

#ifndef __MD_LOCAL_H__
#define __MD_LOCAL_H__

#define MD_DEFAULT_MAILDIR	"Maildir"	/* default maildir name */
#define MD_DIR_MODE		0700		/* maildir directory mode */
#define MD_FILE_MODE		0600		/* maildir file mode */

#define MD_PATH_CUR		"cur"		/* cur directory name */
#define MD_PATH_NEW		"new"		/* new directory name */
#define MD_PATH_TMP		"tmp"		/* tmp directory name */
#define MD_NUM_DIRS		3		/* num of maildir subdirs */

#define MD_OPEN_RETRY		4		/* num of file open retries */
#define MD_RETRY_SLEEP		2		/* num of secons to sleep */

extern char _maildir_name[];
extern char *_maildir_dirs[MD_NUM_DIRS];

#endif	/* __MD_LOCAL__ */
