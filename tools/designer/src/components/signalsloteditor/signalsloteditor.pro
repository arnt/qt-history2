TEMPLATE = lib
CONFIG += qt
CONFIG += staticlib
QT += xml

TARGET = signalsloteditor
DEFINES += QT_SIGNALSLOTEDITOR_LIBRARY

DESTDIR = ../../../lib

INCLUDEPATH += ../../lib/sdk \
    ../../lib/shared \
    ../../lib/uilib \
    ../../lib/extension

PRECOMPILED_HEADER=signalsloteditor_pch.h
HEADERS += \
    default_membersheet.h \
    signalsloteditor.h \
    signalsloteditor_tool.h \
    signalsloteditor_plugin.h \
    signalsloteditor_global.h

SOURCES += \
    default_membersheet.cpp \
    signalsloteditor.cpp \
    signalsloteditor_tool.cpp \
    signalsloteditor_plugin.cpp \
    signalsloteditor_instance.cpp

include(../component.pri)
