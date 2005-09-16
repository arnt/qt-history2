#
# Mac OS X arch files
#

ARCH_CPP=$$QT_SOURCE_TREE/src/corelib/arch/macosx
ARCH_H=$$ARCH_CPP/arch

DEPENDPATH += $$ARCH_CPP;$$ARCH_H

HEADERS += $$ARCH_H/qatomic.h

!*-g++* {
   contains($$list($$system(uname -m)), x86):SOURCES += $$ARCH_CPP/../i386/qatomic.s
   else:SOURCES += $$ARCH_CPP/qatomic32_ppc.s
}

