# $RuOBSD: Makefile,v 1.3 2005/03/28 09:16:07 form Exp $

LIB=	maildir
SRCS=	md_mkdir.c md_local.c md_creatmsg.c md_closemsg.c md_purgemsg.c \
	md_writemsg.c

NOPROFILE=
NOPIC=

afterbuild:
	${RANLIB} lib${lib}.a

install:

.include <bsd.lib.mk>
