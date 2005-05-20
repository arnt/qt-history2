TEMPLATE = lib

CONFIG += qt
CONFIG += staticlib
DEFINES += QT_PROPERTYEDITOR_LIBRARY

DEPENDPATH += .
DESTDIR = ../../../lib

include(propertyeditor.pri)

FORMS += paletteeditor.ui \
    previewwidget.ui

HEADERS += propertyeditor.h \
    paletteeditor.h \
    paletteeditorbutton.h \
    previewwidget.h \
    previewframe.h \
    styledbutton.h

SOURCES += propertyeditor.cpp \
    paletteeditor.cpp \
    paletteeditorbutton.cpp \
    previewwidget.cpp \
    previewframe.cpp \
    styledbutton.cpp

#PRECOMPILED_HEADER=propertyeditor_pch.h

INCLUDEPATH += \
    ../../lib/sdk \
    ../../lib/uilib \
    ../../lib/extension \
    ../../lib/shared \
    $$QT_BUILD_TREE/tools/designer/src/lib \
    ../../lib

include(../component.pri)
