TEMPLATE = lib

CONFIG += qt
CONFIG += staticlib
DEFINES += QT_PROPERTYEDITOR_LIBRARY

DEPENDPATH += .
DESTDIR = ../../../lib

include(propertyeditor.pri)

HEADERS += propertyeditor.h findicondialog.h
SOURCES += propertyeditor.cpp findicondialog.cpp
FORMS += findicondialog.ui

INCLUDEPATH += \
    ../../lib/sdk \
    ../../lib/uilib \
    ../../lib/extension

include(../../lib/sharedcomponents.pri)
