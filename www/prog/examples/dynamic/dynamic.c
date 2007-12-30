/*
 * Данный пример демонстрирует использование динамической загрузки модулей.
 *
 * Использование:
 *
 * $ make depend && make
 * $ ./dynamic m_a.so m_b.so
 *
 * $RuOBSD: dynamic.c,v 1.3 2006/11/21 01:40:10 form Exp $
 */

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>

#include "module.h"


int main(int, char * const *);
__dead static void usage(void);


int
main(int argc, char * const *argv)
{
	module_t *module;
	void *dl;
	int i;

	if (--argc == 0)
		usage();

	for (i = 1; i <= argc; i++) {
		/* подгружаем модуль */
		if ((dl = dlopen(argv[i], DL_LAZY)) == NULL) {
			(void)fprintf(stderr, "%s: %s\n", argv[i], dlerror());
			continue;
		}

		/* находим адрес точки входа */
		if ((module = dlsym(dl, MODULE_ENTRY_NAME)) == NULL)
			(void)fprintf(stderr, "%s: %s\n", argv[i], dlerror());
		else
			/* вызываем точку входа модуля */
			(void)printf("%s: %s\n", argv[i], module());

		/* выгружаем модуль */
		(void)dlclose(dl);
	}

	return (EX_OK);
}

__dead static void
usage(void)
{
	extern char *__progname;

	(void)fprintf(stderr, "usage: %s module [...]\n", __progname);
	exit(EX_USAGE);
}
