# $RuOBSD$

COMMENT=	"Russian (koi8) PostScript fonts"
DISTNAME=	ps-cyr
CATEGORIES=	russian 
HOMEPAGE=	ftp://ftp.kapella.gpi.ru/pub/cyrillic/psfonts/
DISTFILES=	gs-type1_koi8_fonts.tgz \
		gs-type1_koi8_afm.tgz \
		XFree86-Type1_fonts.dir.scale.gz

MASTER_SITES=	ftp://ftp.kapella.gpi.ru/pub/cyrillic/psfonts/

MAINTAINER=	Denis Afonin <dfa@openbsd.ru>

PERMIT_PACKAGE_CDROM=   Yes
PERMIT_PACKAGE_FTP=     Yes
PERMIT_DISTFILES_CDROM= Yes
PERMIT_DISTFILES_FTP=   Yes

NO_BUILD=	Yes

PSC_DIR=	${PREFIX}/share/ps-cyr

do-configure:
	grep koi8 ${WRKDIR}/XFree86-Type1_fonts.dir.scale | wc -l \
		> ${WRKDIR}/fonts.dir
	grep koi8 ${WRKDIR}/XFree86-Type1_fonts.dir.scale \
		>> ${WRKDIR}/fonts.dir

do-install:
	${INSTALL_DATA_DIR} ${PSC_DIR}
	${INSTALL_DATA} ${WRKDIR}/*.pfm ${PSC_DIR}
	${INSTALL_DATA} ${WRKDIR}/*.pfb ${PSC_DIR}
	${INSTALL_DATA} ${WRKDIR}/*.afm ${PSC_DIR}
	${INSTALL_DATA} ${FILESDIR}/gs-6.51-Fontmap.patch ${PSC_DIR}
	${INSTALL_DATA} ${WRKDIR}/fonts.dir ${PSC_DIR}
	@ln -s fonts.dir ${PSC_DIR}/fonts.scale

.include <bsd.port.mk>
