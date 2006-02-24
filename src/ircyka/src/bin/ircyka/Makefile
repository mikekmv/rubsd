# $RuOBSD$

PROG=	ircyka
SRCS=	ircyka.c ircyka_alarm.c ircyka_channel.c ircyka_charset.c \
	ircyka_conf.c ircyka_handler.c ircyka_irc.c ircyka_join.c \
	ircyka_loop.c ircyka_match_irc.c ircyka_match_cmd.c ircyka_module.c \
	ircyka_nick.c ircyka_qio.c ircyka_socket.c

LDFLAGS+=-Wl,-E

.if exists(${.CURDIR}/../../lib/nickdb/obj/libnickdb.a)
DPADD=	${.CURDIR}/../../lib/nickdb/obj/libnickdb.a
LDADD=	-L${.CURDIR}/../../lib/nickdb/obj -lnickdb
.else
DPADD=	${.CURDIR}/../../lib/nickdb/libnickdb.a
LDADD=	-L${.CURDIR}/../../lib/nickdb -lnickdb
.endif

.if exists(${.CURDIR}/../../lib/acldb/obj/libacldb.a)
DPADD+=	${.CURDIR}/../../lib/acldb/obj/libacldb.a
LDADD+=	-L${.CURDIR}/../../lib/acldb/obj -lacldb
.else
DPADD+=	${.CURDIR}/../../lib/acldb/libacldb.a
LDADD+=	-L${.CURDIR}/../../lib/acldb -lacldb
.endif

.include <bsd.prog.mk>
