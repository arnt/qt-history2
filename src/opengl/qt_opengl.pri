# Qt opengl module

opengl {
	HEADERS += $$OPENGL_H/qgl.h
	SOURCES	+= $$OPENGL_CPP/qgl.cpp
	mac:SOURCES += $$OPENGL_CPP/qgl_mac.cpp
	x11:SOURCES += $$OPENGL_CPP/qgl_x11.cpp
	win32:SOURCES += $$OPENGL_CPP/qgl_win.cpp
}



