TEMPLATE = lib

CONFIG += qt
CONFIG += staticlib
DEFINES += QT_PROPERTYEDITOR_LIBRARY

DEPENDPATH += .
DESTDIR = ../../../lib

include(propertyeditor.pri)

FORMS += actioneditor.ui

HEADERS += propertyeditor.h \
    actioneditor.h

SOURCES += propertyeditor.cpp \
    actioneditor.cpp

#PRECOMPILED_HEADER=propertyeditor_pch.h

INCLUDEPATH += \
    ../../lib/sdk \
    ../../lib/uilib \
    ../../lib/extension \
    ../../lib/shared \
    $$QT_BUILD_TREE/tools/designer/src/lib \
    ../../lib

include(../component.pri)
