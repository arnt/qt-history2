#
# sparc (V7) arch files
#

ARCH_CPP=$$QT_SOURCE_TREE/arch/sparc
ARCH_H=$$ARCH_CPP/arch

DEPENDPATH += $$ARCH_CPP;$$ARCH_H

HEADERS += $$ARCH_H/qatomic.h
SOURCES += $$ARCH_CPP/qatomic.S

