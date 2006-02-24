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

#include <sys/param.h>
#include <errno.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ircyka/ircyka.h"

struct ircyka_module_list ircyka_modules =
    LIST_HEAD_INITIALIZER(&ircyka_modules);


struct ircyka_module *
module_find(const char *name)
{
	struct ircyka_module *im;

	LIST_FOREACH(im, &ircyka_modules, im_entry)
		if (strcmp(name, im->im_name) == 0)
			return (im);

	return (NULL);
}

struct ircyka_module *
module_load(const char *name, const char *target, const char *args)
{
	struct ircyka_module *im;
	char path[MAXPATHLEN];
	void *dl;

	if (strchr(name, '/') != NULL || strlen(name) >= IRCYKA_MAX_MODNAME) {
		errno = EINVAL;
		return (NULL);
	}

	(void)snprintf(path, sizeof(path), "%s/%s/%s.so", ircyka_conf_dir,
	    IRCYKA_MODULES_DIR, name);

	if (access(path, R_OK) < 0 && errno == ENOENT)
		(void)snprintf(path, sizeof(path), "%s/%s/%s.so",
		    IRCYKA_DATA_DIR, IRCYKA_MODULES_DIR, name);

	if ((dl = dlopen(path, DL_LAZY)) == NULL) {
		errno = 0;
		return (NULL);
	}

	if ((im = dlsym(dl, IRCYKA_MODULE_DATA)) == NULL ||
	    im->im_rev != IRCYKA_VERSION_MAJOR ||
	    strcmp(name, im->im_name) != 0) {
		(void)dlclose(dl);
		errno = EFTYPE;
		return (NULL);
	}

	if (module_find(name) != NULL) {
		(void)dlclose(dl);
		errno = EEXIST;
		return (NULL);
	}

	if (im->im_exec(IME_LOAD, target, args) < 0) {
		int save_errno = errno;

		(void)dlclose(dl);
		errno = save_errno;
		return (NULL);
	}

	im->im_handle = dl;
	LIST_INSERT_HEAD(&ircyka_modules, im, im_entry);

	return (im);
}

int
module_unload(struct ircyka_module *im, const char *target, const char *args)
{
	if (im->im_exec(IME_UNLOAD, target, args) < 0)
		return (-1);

	LIST_REMOVE(im, im_entry);
	(void)dlclose(im->im_handle);

	return (0);
}
