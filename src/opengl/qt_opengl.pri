# Qt opengl module

opengl {
	HEADERS += $$OPENGL_H/qgl.h \
		   $$OPENGL_H/qglcolormap.h
	SOURCES	+= $$OPENGL_CPP/qgl.cpp \
		   $$OPENGL_CPP/qglcolormap.cpp
	mac:SOURCES += $$OPENGL_CPP/qgl_mac.cpp
	x11:SOURCES += $$OPENGL_CPP/qgl_x11.cpp
	win32:SOURCES += $$OPENGL_CPP/qgl_win.cpp
}



