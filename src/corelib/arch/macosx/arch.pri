#
# Mac OS X architecture
#
!*-g++* {
   contains($$list($$system(uname -m)), x86):SOURCES += $$ARCH_CPP/../i386/qatomic.s
   else:SOURCES += $$ARCH_CPP/../powerpc/qatomic32_ppc.s
}
