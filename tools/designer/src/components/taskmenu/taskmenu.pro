TEMPLATE = lib
QT += xml
CONFIG += qt
CONFIG += staticlib

INCLUDEPATH += \
    ../../lib/sdk \
    ../../lib/extension \
    ../../shared \

TARGET = taskmenu

DESTDIR = ../../../lib

DEFINES += QT_TASKMENU_LIBRARY

HEADERS += button_taskmenu.h \
  groupbox_taskmenu.h \
  taskmenu_component.h

SOURCES += button_taskmenu.cpp \
  groupbox_taskmenu.cpp \
  taskmenu_component.cpp

include(../../sharedcomponents.pri)
