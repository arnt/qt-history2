ARCH_CPP=$$QT_SOURCE_TREE/src/corelib/arch/x86_64
ARCH_H=$$ARCH_CPP/arch

DEPENDPATH += $$ARCH_CPP;$$ARCH_H

HEADERS += $$ARCH_H/qatomic.h

solaris-cc*:SOURCES += $$ARCH_CPP/qatomic_sun.s

