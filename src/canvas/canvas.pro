# Qt canvas module

REQUIRES = !qt_one_lib
TARGET = QtCanvas

DEFINES += QT_BUILD_CANVAS_LIB

include(../qbase.pri)
QT = core gui

HEADERS += qcanvas.h
SOURCES += qcanvas.cpp
