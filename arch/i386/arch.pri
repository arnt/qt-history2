# 
# i386 specific files
#

ARCH_CPP=$$QT_SOURCE_TREE/arch/i386
ARCH_H=$$ARCH_CPP/arch

DEPENDPATH += $$ARCH_CPP;$$ARCH_H

HEADERS += $$ARCH_H/qatomic.h

unix:!*-g++*:!*-icc*:SOURCES += $$ARCH_CPP/qatomic.S
win32-borland:SOURCES += $$ARCH_CPP/qatomic.cpp
