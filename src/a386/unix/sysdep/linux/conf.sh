#! /bin/sh

host="$*"

rm -f arch ../../include/a386/a386_reg.h ../os ../../sysdep.c
rm -f ../../sysdep.h ../../include/a386/savecontext.h

case "$host" in
arm-*)
    ln -s arm arch
    ln -s ../../../arm/a386_reg.h ../../include/a386/a386_reg.h
    ;;
i?86-*)
    ln -s i386 arch
    ln -s ../../../i386/a386_reg.h ../../include/a386/a386_reg.h
    ;;
sparc-*)
    ln -s sparc arch
    ln -s ../../../sparc/a386_reg.h ../../include/a386/a386_reg.h
    ;;
*)
    echo unsupported cpu architecture
    ;;
esac

ln -s linux ../os
ln -s sysdep/linux/sysdep.c ../../sysdep.c
ln -s sysdep/linux/arch/arch.h ../../sysdep.h
ln -s ../../sysdep/linux/arch/savecontext.h ../../include/a386/savecontext.h
