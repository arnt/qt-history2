#
# Mac OS X architecture
#
!*-g++* {
   contains($$list($$system(uname -m)), x86):SOURCES += $$QT_ARCH_CPP/../i386/qatomic.s
   else:SOURCES += $$QT_ARCH_CPP/../powerpc/qatomic32_ppc.s
}
