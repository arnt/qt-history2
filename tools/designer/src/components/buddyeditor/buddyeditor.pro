TEMPLATE = lib
CONFIG += qt
CONFIG += staticlib
QT += xml

TARGET = buddyeditor
DEFINES += QT_BUDDYEDITOR_LIBRARY

DESTDIR = ../../../lib

INCLUDEPATH += ../../lib/sdk \
    ../../shared \
    ../../uilib \
    ../../lib/extension \
    ../buddyeditor


HEADERS += \
    buddyeditor.h \
    buddyeditor_plugin.h \
    buddyeditor_tool.h \
    buddyeditor_global.h

SOURCES += \
    buddyeditor.cpp \
    buddyeditor_tool.cpp \
    buddyeditor_plugin.cpp

include(../../sharedcomponents.pri)
