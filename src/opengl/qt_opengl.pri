# Qt opengl module

opengl {
	win32:OPENGL_H	= ../include
	unix:OPENGL_H	= opengl
	unix:DEPENDPATH += :$$OPENGL_H
	HEADERS += $$OPENGL_H/qgl.h
	OPENGL_SOURCES	= opengl/qgl.cpp
	unix:OPENGL_SOURCES += opengl/qgl_x11.cpp
	win32:OPENGL_SOURCES += opengl/qgl_win.cpp
	SOURCES    += $$OPENGL_SOURCES
}



