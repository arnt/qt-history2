#
# BoundsChecker architecture
# Since BoundsChecker doesn't work correctly with assembly and intrinsic
# functions we need to have an own architecture, which uses spinlocks to
# be fairly fast and swaps pointers with normal C code.
#

ARCH_CPP=$$QT_SOURCE_TREE/src/corelib/arch/boundschecker
ARCH_H=$$ARCH_CPP/arch

DEPENDPATH += $$ARCH_CPP;$$ARCH_H

HEADERS += $$ARCH_H/qatomic.h
