#
# ARM architecture
#

ARCH_CPP=$$QT_SOURCE_TREE/src/core/arch/arm
ARCH_H=$$ARCH_CPP/arch

DEPENDPATH += $$ARCH_CPP;$$ARCH_H

HEADERS += $$ARCH_H/qatomic.h
SOURCES += $$ARCH_CPP/qatomic.cpp
