#
# 's390' architecture
#

ARCH_CPP=$$QT_SOURCE_TREE/src/corelib/arch/s390
ARCH_H=$$ARCH_CPP/arch

DEPENDPATH += $$ARCH_CPP;$$ARCH_H

HEADERS += $$ARCH_H/qatomic.h
