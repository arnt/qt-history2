TEMPLATE = lib
DESTDIR = ../../../lib
CONFIG += qt
DEFINES += QT_OBJECTINSPECTOR_LIBRARY
CONFIG += staticlib

INCLUDEPATH += \
    ../../lib/sdk \
    ../../lib/extension \
    ../../lib/uilib \
    ../../lib/shared

HEADERS += objectinspector.h \
    objectinspector_global.h

SOURCES += objectinspector.cpp

include(../component.pri)
