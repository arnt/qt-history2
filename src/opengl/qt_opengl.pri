# Qt opengl module

opengl {
	HEADERS += $$OPENGL_H/qgl.h
	OPENGL_SOURCES	= $$OPENGL_CPP/qgl.cpp
	unix:OPENGL_SOURCES += $$OPENGL_CPP/qgl_x11.cpp
	win32:OPENGL_SOURCES += $$OPENGL_CPP/qgl_win.cpp
	SOURCES    += $$OPENGL_SOURCES
}



