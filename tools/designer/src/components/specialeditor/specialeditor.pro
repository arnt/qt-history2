TEMPLATE = lib
QT += xml
CONFIG += qt
CONFIG += staticlib

INCLUDEPATH += \
    ../../sdk \
    ../../extension \

TARGET = specialeditor

DESTDIR = ../../../lib

DEFINES += QT_SPECIALEDITORSUPPORT_LIBRARY

SOURCES += defaultspecialeditor.cpp \
    specialeditorsupport.cpp

HEADERS += defaultspecialeditor.h \
    specialeditor.h \
    specialeditorsupport.h \
    specialeditor_global.h

