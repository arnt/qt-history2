#
# 'generic' architecture
#

ARCH_CPP=$$QT_SOURCE_TREE/src/corelib/arch/alpha
ARCH_H=$$ARCH_CPP/arch

DEPENDPATH += $$ARCH_CPP;$$ARCH_H

HEADERS += $$ARCH_H/qatomic.h
!*-g++*:SOURCES += $$ARCH_CPP/qatomic.s

