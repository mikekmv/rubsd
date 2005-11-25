# $RuOBSD$

COMMENT=	"MP3 / OGG Vorbis streaming server"

DISTNAME=	gnump3d-2.9.8
CATEGORIES=	audio net
HOMEPAGE=	http://www.gnu.org/software/gnump3d/
MASTER_SITES=	http://savannah.gnu.org/download/gnump3d/

PERMIT_PACKAGE_CDROM= 	Yes 
PERMIT_PACKAGE_FTP=	Yes 
PERMIT_DISTFILES_CDROM=	Yes 
PERMIT_DISTFILES_FTP=	Yes

NO_BUILD=	Yes
NO_REGRESS=	Yes
PKG_ARCH=	*

USERNAME=	_gnump3d

SPOOLDIR=	/var/gnump3d
LOGDIR=		${SPOOLDIR}/logs
PLUGINDIR=	${PREFIX}/libdata/perl5/site_perl/${MACHINE_ARCH}-openbsd
EXAMPLEDIR=	${PREFIX}/share/examples/gnump3d
THEMEDIR=	${PREFIX}/share/gnump3d

post-patch:
	@perl -pi -e 's:\@PREFIX@:${PREFIX}:g;' \
		-e 's:\@SYSCONFDIR@:${SYSCONFDIR}:g;' \
		-e 's:\@LOCALBASE@:${LOCALBASE}:g;' \
		-e 's:\@EXAMPLEDIR@:${EXAMPLEDIR}:g;' \
		-e 's:\@SPOOLDIR@:${SPOOLDIR}:g;' \
		-e 's:\@LOGDIR@:${LOGDIR}:g;' \
		-e 's:\@PLUGINDIR@:${PLUGINDIR}:g;' \
		-e 's:\@THEMEDIR@:${THEMEDIR}:g;' \
		-e 's:\@USERNAME@:${USERNAME}:g;' \
		${WRKBUILD}/etc/gnump3d.conf ${WRKBUILD}/bin/gnump3d2

do-install:
	${INSTALL_SCRIPT} ${WRKDIST}/bin/gnump3d2 ${PREFIX}/bin/gnump3d
	${INSTALL_SCRIPT} ${WRKDIST}/bin/gnump3d-index \
		${PREFIX}/bin/gnump3d-index
	${INSTALL_PROGRAM} ${WRKDIST}/bin/gnump3d-top ${PREFIX}/bin/gnump3d-top
	${INSTALL_MAN} ${WRKDIST}/man/gnump3d.1 ${PREFIX}/man/man1/gnump3d.1
	${INSTALL_MAN} ${WRKDIST}/man/gnump3d-index.1 \
		${PREFIX}/man/man1/gnump3d-index.1
	${INSTALL_MAN} ${WRKDIST}/man/gnump3d-top.1 \
		${PREFIX}/man/man1/gnump3d-top.1
	${INSTALL_MAN} ${WRKDIST}/man/gnump3d.conf.1 \
		${PREFIX}/man/man5/gnump3d.conf.5
	${INSTALL_DATA_DIR} ${EXAMPLEDIR}
	${INSTALL_DATA} ${WRKDIST}/etc/gnump3d.conf \
		${EXAMPLEDIR}/gnump3d.conf.sample
	${INSTALL_DATA} ${WRKDIST}/etc/mime.types ${EXAMPLEDIR}/mime.types
	mkdir -p ${THEMEDIR}
	cp -R ${WRKDIST}/templates/* ${THEMEDIR}
	mkdir -p ${THEMEDIR}
	cp -R ${WRKDIST}/lib ${PLUGINDIR}

.include <bsd.port.mk>
