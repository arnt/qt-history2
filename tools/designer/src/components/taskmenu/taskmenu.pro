TEMPLATE = lib
QT += xml
CONFIG += qt
CONFIG += staticlib

INCLUDEPATH += \
    ../../lib/sdk \
    ../../lib/extension \
    ../../lib/shared \
    ../../lib

TARGET = taskmenu

DESTDIR = ../../../lib

DEFINES += QT_TASKMENU_LIBRARY

PRECOMPILED_HEADER=taskmenu_pch.h

FORMS += listwidgeteditor.ui \
    treewidgeteditor.ui

HEADERS += button_taskmenu.h \
  groupbox_taskmenu.h \
  label_taskmenu.h \
  lineedit_taskmenu.h \
  listwidget_taskmenu.h \
  combobox_taskmenu.h \
  inplace_editor.h \
  taskmenu_component.h \
  listwidgeteditor.h

SOURCES += button_taskmenu.cpp \
  groupbox_taskmenu.cpp \
  label_taskmenu.cpp \
  lineedit_taskmenu.cpp \
  listwidget_taskmenu.cpp \
  combobox_taskmenu.cpp \
  inplace_editor.cpp \
  taskmenu_component.cpp \
  listwidgeteditor.cpp

include(../component.pri)
