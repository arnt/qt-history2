TEMPLATE = lib
QT += xml
CONFIG += qt
CONFIG += staticlib

INCLUDEPATH += \
    ../../lib/sdk \
    ../../lib/extension \

TARGET = taskmenu

DESTDIR = ../../../lib

DEFINES += QT_TASKMENU_LIBRARY

HEADERS += button_taskmenu.h
SOURCES += button_taskmenu.cpp

include(../../sharedcomponents.pri)
