HEADERS += arch/qatomic_alpha.h \
           arch/qatomic_boundschecker.h \
           arch/qatomic_ia64.h \
           arch/qatomic_parisc.h \
           arch/qatomic_sparc.h \
           arch/qatomic_arch.h \
           arch/qatomic_generic.h \
           arch/qatomic_macosx.h \
           arch/qatomic_powerpc.h \
           arch/qatomic_windows.h \
           arch/qatomic_arm.h \
           arch/qatomic_i386.h \
           arch/qatomic_mips.h \
           arch/qatomic_s390.h \
           arch/qatomic_x86_64.h

ARCH_CPP = $$QT_SOURCE_TREE/src/corelib/arch/$$ARCH
DEPENDPATH += $$ARCH_CPP
include($$ARCH_CPP/arch.pri)
