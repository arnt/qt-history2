#
# SPARC arch files
#

ARCH_CPP=$$QT_SOURCE_TREE/src/corelib/arch/sparc
ARCH_H=$$ARCH_CPP/arch

DEPENDPATH += $$ARCH_CPP;$$ARCH_H

HEADERS += $$ARCH_H/qatomic.h

*-64:SOURCES += $$ARCH_CPP/qatomic64.s
else:SOURCES += $$ARCH_CPP/qatomic32.s

