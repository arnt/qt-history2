TEMPLATE = lib
CONFIG += qt
CONFIG += staticlib
QT += xml

TARGET = buddyeditor
DEFINES += QT_BUDDYEDITOR_LIBRARY

DESTDIR = ../../../lib

INCLUDEPATH += ../../lib/sdk \
    ../../shared \
    ../../uilib \
    ../../lib/extension \
    ../formeditor \
    ../buddyeditor
    

HEADERS += buddyeditor.h buddyeditor_global.h

SOURCES += buddyeditor.cpp

include(../../sharedcomponents.pri)
