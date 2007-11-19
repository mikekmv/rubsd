/*
 * Данный пример демонстрирует использование RB деревьев
 *
 * $RuOBSD$
 */

#include <sys/types.h>
#include <sys/tree.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>


/*
 * Определяем структуру элемента дерева.
 */
struct string {
	char			*str;		/* строка текста */
	RB_ENTRY(string)	entry;		/* связка в дереве */
};

/*
 * Определяем структуру заголовка дерева.
 */
RB_HEAD(strings, string);


/*
 * Объявляем и инициализируем заголовок дерева.
 */
static struct strings strings = RB_INITIALIZER(&strings);


/*
 * Объявляем функцию сравнения элементов. Делаем ее inline для
 * ускорения работы. Функция должна возвращать 0 если элементы одинаковы,
 * отрицательное число если a < b и положительное если a > b.
 */
static __inline int
string_compare(const struct string *a, const struct string *b)
{
	return (strcmp(a->str, b->str));
}

/*
 * Объявляем прототипы для функций дерева.
 */
RB_PROTOTYPE(strings, string, entry, string_compare)

/*
 * Генерируем функции для дерева.
 */
RB_GENERATE(strings, string, entry, string_compare)


int
main(void)
{
	struct string *s = NULL, *ss, sh;
	char buf[132];

	printf("Команды:\n\n");
	printf("+<строка> - добавить строку\n");
	printf("-<строка> - удалить строку\n");
	printf("?<строка> - проверить строку\n\n");

	/*
	 * Читаем строки с терминала до конца файла (Ctrl/D).
	 */
	while (fgets(buf, sizeof(buf), stdin) != NULL) {
		size_t len;

		/* убираем завершающий \n из строки */
		if ((len = strlen(buf)) > 0 && buf[len - 1] == '\n')
			buf[--len] = '\0';

		if (len == 0)
			continue;

		switch (buf[0]) {
		case '+':
			/* выделяем место под новый элемент */
			if (s == NULL && (s = malloc(sizeof(*s))) == NULL)
				err(EX_UNAVAILABLE, NULL);

			/* копируем строку */
			if ((s->str = strdup(&buf[1])) == NULL)
				err(EX_UNAVAILABLE, NULL);

			/* добавляем элемент в дерево */
			if (RB_INSERT(strings, &strings, s) != NULL) {
				warnx("такая строка есть");
				free(s->str);
			} else
				s = NULL;
			break;
		case '-':
		case '?':
			/* выполняем поиск строки */
			sh.str = &buf[1];
			if ((ss = RB_FIND(strings, &strings, &sh)) == NULL)
				warnx("такой строки нет");
			else if (buf[0] == '-') {
				/* удаляем строку из дерева */
				RB_REMOVE(strings, &strings, ss);
				free(ss->str);
				free(ss);
			} else
				warnx("такая строка есть");
			break;
		default:
			warnx("неверная команда");
			break;
		}
	}

	/* освобождаем неиспользованный элемент */
	if (s != NULL)
		free(s);

	printf("\n\nСписок строк:\n");

	/* печатаем сортированный список строк */
	RB_FOREACH(ss, strings, &strings)
		printf("  %s\n", ss->str);

	return (EX_OK);
}
