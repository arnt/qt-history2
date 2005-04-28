TEMPLATE = lib
CONFIG += qt
CONFIG += staticlib
QT += xml

TARGET = resourceeditor
DEFINES += QT_RESOURCEEDITOR_LIBRARY

DESTDIR = ../../../lib

INCLUDEPATH += ../../lib/sdk \
    ../../lib/shared \
    ../../lib/uilib \
    ../../lib/extension

FORMS += resourceeditor.ui

HEADERS += \
    resourceeditor.h

SOURCES += \
    resourceeditor.cpp

include(../component.pri)
