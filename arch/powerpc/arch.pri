#
# PowerPC arch files
#

ARCH_CPP=$$QT_SOURCE_TREE/arch/powerpc
ARCH_H=$$ARCH_CPP/arch

DEPENDPATH += $$ARCH_CPP;$$ARCH_H

HEADERS += $$ARCH_H/qatomic.h

*-g++*:SOURCES += $$ARCH_CPP/qatomic.cpp
    
!*-g++ {
    *-64 {
        SOURCES += $$ARCH_CPP/qatomic64.s
    } else {
        SOURCES += $$ARCH_CPP/qatomic32.s
    }
}

