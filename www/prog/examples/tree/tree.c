/*
 * ������ ������ ������������� ������������� RB ��������
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
 * ���������� ��������� �������� ������.
 */
struct string {
	char			*str;		/* ������ ������ */
	RB_ENTRY(string)	entry;		/* ������ � ������ */
};

/*
 * ���������� ��������� ��������� ������.
 */
RB_HEAD(strings, string);


/*
 * ��������� � �������������� ��������� ������.
 */
static struct strings strings = RB_INITIALIZER(&strings);


/*
 * ��������� ������� ��������� ���������. ������ �� inline ���
 * ��������� ������. ������� ������ ���������� 0 ���� �������� ���������,
 * ������������� ����� ���� a < b � ������������� ���� a > b.
 */
static __inline int
string_compare(const struct string *a, const struct string *b)
{
	return (strcmp(a->str, b->str));
}

/*
 * ��������� ��������� ��� ������� ������.
 */
RB_PROTOTYPE(strings, string, entry, string_compare)

/*
 * ���������� ������� ��� ������.
 */
RB_GENERATE(strings, string, entry, string_compare)


int
main(void)
{
	struct string *s = NULL, *ss, sh;
	char buf[132];

	printf("�������:\n\n");
	printf("+<������> - �������� ������\n");
	printf("-<������> - ������� ������\n");
	printf("?<������> - ��������� ������\n\n");

	/*
	 * ������ ������ � ��������� �� ����� ����� (Ctrl/D).
	 */
	while (fgets(buf, sizeof(buf), stdin) != NULL) {
		size_t len;

		/* ������� ����������� \n �� ������ */
		if ((len = strlen(buf)) > 0 && buf[len - 1] == '\n')
			buf[--len] = '\0';

		if (len == 0)
			continue;

		switch (buf[0]) {
		case '+':
			/* �������� ����� ��� ����� ������� */
			if (s == NULL && (s = malloc(sizeof(*s))) == NULL)
				err(EX_UNAVAILABLE, NULL);

			/* �������� ������ */
			if ((s->str = strdup(&buf[1])) == NULL)
				err(EX_UNAVAILABLE, NULL);

			/* ��������� ������� � ������ */
			if (RB_INSERT(strings, &strings, s) != NULL) {
				warnx("����� ������ ����");
				free(s->str);
			} else
				s = NULL;
			break;
		case '-':
		case '?':
			/* ��������� ����� ������ */
			sh.str = &buf[1];
			if ((ss = RB_FIND(strings, &strings, &sh)) == NULL)
				warnx("����� ������ ���");
			else if (buf[0] == '-') {
				/* ������� ������ �� ������ */
				RB_REMOVE(strings, &strings, ss);
				free(ss->str);
				free(ss);
			} else
				warnx("����� ������ ����");
			break;
		default:
			warnx("�������� �������");
			break;
		}
	}

	/* ����������� ���������������� ������� */
	if (s != NULL)
		free(s);

	printf("\n\n������ �����:\n");

	/* �������� ������������� ������ ����� */
	RB_FOREACH(ss, strings, &strings)
		printf("  %s\n", ss->str);

	return (EX_OK);
}
