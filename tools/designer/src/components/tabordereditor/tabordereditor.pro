TEMPLATE = lib
CONFIG += qt
CONFIG += staticlib
QT += xml

TARGET = tabordereditor
DEFINES += QT_TABORDEREDITOR_LIBRARY

DESTDIR = ../../../lib

INCLUDEPATH += ../../lib/sdk \
    ../../lib/shared \
    ../../lib/uilib \
    ../../lib/extension \
    ../tabordereditor


PRECOMPILED_HEADER=tabordereditor_pch.h
HEADERS += \
    tabordereditor.h \
    tabordereditor_plugin.h \
    tabordereditor_tool.h \
    tabordereditor_global.h

SOURCES += \
    tabordereditor.cpp \
    tabordereditor_tool.cpp \
    tabordereditor_plugin.cpp \
    tabordereditor_instance.cpp

include(../component.pri)
