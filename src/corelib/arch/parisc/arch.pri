#
# HP PA-RISC arch files
#

ARCH_CPP=$$QT_SOURCE_TREE/src/corelib/arch/parisc
ARCH_H=$$ARCH_CPP/arch

DEPENDPATH += $$ARCH_CPP;$$ARCH_H

HEADERS += $$ARCH_H/qatomic.h
SOURCES += $$ARCH_CPP/q_ldcw.s \
	   $$ARCH_CPP/qatomic.cpp

