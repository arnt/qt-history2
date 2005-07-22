#
# Mac OS X arch files
#

ARCH_CPP=$$QT_SOURCE_TREE/src/corelib/arch/macosx
ARCH_H=$$ARCH_CPP/arch

DEPENDPATH += $$ARCH_CPP;$$ARCH_H

HEADERS += $$ARCH_H/qatomic.h

!*-g++* {
   ppc:SOURCES += $$ARCH_CPP/qatomic32_ppc.s
   x86:SOURCES += $$ARCH_CPP/qatomic_x86.s
}

