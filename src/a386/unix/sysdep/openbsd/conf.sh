#! /bin/sh
# $RuOBSD$

host="$*"

rm -f arch ../../include/a386/a386_reg.h ../os ../../sysdep.c
rm -f ../../sysdep.h ../../include/a386/savecontext.h

case "$host" in
i?86-*)
    ln -s i386 arch
    ln -s ../../../i386/a386_reg.h ../../include/a386/a386_reg.h
    ;;
*)
    echo unsupported cpu architecture
    ;;
esac

ln -s openbsd ../os
ln -s sysdep/openbsd/sysdep.c ../../sysdep.c
ln -s sysdep/openbsd/arch/arch.h ../../sysdep.h
ln -s ../../sysdep/openbsd/arch/savecontext.h ../../include/a386/savecontext.h
