# 
# i386 specific files
#

ARCH_CPP=$$QT_SOURCE_TREE/src/corelib/arch/i386
ARCH_H=$$ARCH_CPP/arch

DEPENDPATH += $$ARCH_CPP;$$ARCH_H

HEADERS += $$ARCH_H/qatomic.h

!*-g++*:!*-icc*:SOURCES += $$ARCH_CPP/qatomic.s
