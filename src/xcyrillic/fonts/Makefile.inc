# $RuOBSD$

X11BASE?=	/usr/X11R6
LOCALBASE?=	/usr/local
PREFIX?=	${LOCALBASE}
FONTSDIR?=	${PREFIX}/lib/X11/fonts/xcyrillic

INSTALL_DATA_DIR?=install -o root -g wheel -m 555 -d
INSTALL_DATA?=	install -o root -g wheel -m 444

BDFS=
FONTS=
.if defined(FIXED) && ${FIXED:U} == YES
FONTFLAGS?=	-l -t
.for f in ${SRCS}
FONTS+=		${f}.pcf.gz
.endfor
.else
FONTFLAGS?=	-l
.for f in ${SRCS}
BDFS+=		${f}o.bdf ${f}b.bdf ${f}bo.bdf
FONTS+=		${f}.pcf.gz ${f}o.pcf.gz ${f}b.pcf.gz ${f}bo.pcf.gz
.endfor
.endif

all:		fonts.dir

clean:
	rm -f ${BDFS} ${FONTS} fonts.dir

fonts.dir:	${BDFS} ${FONTS}
	${X11BASE}/bin/mkfontdir .

install:	all
	${INSTALL_DATA_DIR} ${FONTSDIR}/${FONTSSUBDIR}
.for f in ${FONTS}
	${INSTALL_DATA} ${f} ${FONTSDIR}/${FONTSSUBDIR}
.endfor
	${INSTALL_DATA} fonts.dir ${FONTSDIR}/${FONTSSUBDIR}
.if defined(FIXED) && ${FIXED:U} == YES
	${INSTALL_DATA} fonts.alias ${FONTSDIR}/${FONTSSUBDIR}
.endif

.for f in ${SRCS}
${f}o.bdf:	${f}.bdf
	perl ../../scripts/bdfslant.pl < ${f}.bdf > ${f}o.bdf

${f}b.bdf:	${f}.bdf
	perl ../../scripts/bdfbold.pl < ${f}.bdf > ${f}b.bdf

${f}bo.bdf:	${f}b.bdf
	perl ../../scripts/bdfslant.pl < ${f}b.bdf > ${f}bo.bdf

.endfor

.SUFFIXES:	.bdf .pcf.gz

.bdf.pcf.gz:
	${X11BASE}/bin/bdftopcf ${FONTFLAGS} < $< | gzip -9nc > $@
