LIB=	maildir
SRCS=	md_mkdir.c md_local.c md_creatmsg.c md_closemsg.c md_purgemsg.c \
	md_writemsg.c

COPTS=	-Wall -Werror -Wtraditional -ansi -pedantic

NOPROFILE=
NOPIC=

afterbuild:
	${RANLIB} lib${lib}.a

install:

.include <bsd.lib.mk>
