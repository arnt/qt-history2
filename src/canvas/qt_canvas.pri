# Qt canvas module

canvas {
	win32:CANVAS_H	= ../include
	unix:CANVAS_H	= canvas
	unix:DEPENDPATH += :$$CANVAS_H
	HEADERS += $$CANVAS_H/qcanvas.h
	SOURCES += canvas/qcanvas.cpp
}