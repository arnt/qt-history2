TEMPLATE = lib
QT += xml
CONFIG += qt
CONFIG += staticlib

INCLUDEPATH += \
    ../../lib/sdk \
    ../../lib/extension \

TARGET = specialeditor

DESTDIR = ../../../lib

DEFINES += QT_SPECIALEDITORSUPPORT_LIBRARY

SOURCES += defaultspecialeditor.cpp \
    specialeditorsupport.cpp

HEADERS += defaultspecialeditor.h \
    specialeditor.h \
    specialeditorsupport.h \
    specialeditor_global.h

include(../../sharedcomponents.pri)
