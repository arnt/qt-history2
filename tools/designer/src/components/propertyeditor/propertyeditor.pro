TEMPLATE = lib

QT += compat

CONFIG += qt
CONFIG += staticlib
DEFINES += QT_PROPERTYEDITOR_LIBRARY

DEPENDPATH += .
DESTDIR = ../../../lib

include(propertyeditor.pri)

HEADERS += propertyeditor.h
SOURCES += propertyeditor.cpp

INCLUDEPATH += \
    ../../sdk \
    ../../uilib \
    ../../extension
