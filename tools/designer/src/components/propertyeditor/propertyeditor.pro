TEMPLATE = lib

QT += qt3support

CONFIG += qt 
CONFIG += staticlib
DEFINES += QT_PROPERTYEDITOR_LIBRARY

DEPENDPATH += .
DESTDIR = ../../../lib

include(propertyeditor.pri)

HEADERS += propertyeditor.h
SOURCES += propertyeditor.cpp

INCLUDEPATH += \
    ../../lib/sdk \
    ../../uilib \
    ../../lib/extension

include(../../sharedcomponents.pri)
