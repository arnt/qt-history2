#
# PowerPC arch files
#

ARCH_CPP=$$QT_SOURCE_TREE/src/core/barch/powerpc
ARCH_H=$$ARCH_CPP/arch

DEPENDPATH += $$ARCH_CPP;$$ARCH_H

HEADERS += $$ARCH_H/qatomic.h
!*-g++* {
    *-64 {
        SOURCES += $$ARCH_CPP/qatomic64.s
    } else {
        macx-*: SOURCES += $$ARCH_CPP/qatomic32_mac.s
        else: SOURCES += $$ARCH_CPP/qatomic32.s
    }
}

