include(../src/qsharedmemory.pri)

win32: CONFIG += console
mac:CONFIG -= app_bundle

SOURCES		+= main.cpp \
                   dialog.cpp
HEADERS         += dialog.h

INTERFACES      += dialog.ui
TARGET		= example
