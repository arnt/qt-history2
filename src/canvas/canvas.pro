# Qt canvas module

REQUIRES = !qt_one_lib
TARGET = qcanvas

DEFINES += QT_BUILD_CANVAS_LIB

include(../qbase.pri)
QT = core gui

canvas {
	HEADERS += qcanvas.h
	SOURCES += qcanvas.cpp
}
