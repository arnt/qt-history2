TEMPLATE = lib
DESTDIR = ../../../lib
CONFIG += qt
DEFINES += QT_OBJECTINSPECTOR_LIBRARY
CONFIG += staticlib

INCLUDEPATH += \
    ../../lib/sdk \
    ../../lib/extension \
    ../../uilib \
    ../../shared

HEADERS += objectinspector.h \
    objectinspector_global.h

SOURCES += objectinspector.cpp

include(../../sharedcomponents.pri)
