# Qt opengl module

opengl {
	HEADERS += $$OPENGL_H/qgl.h \
		   $$OPENGL_H/qcolormap.h
	SOURCES	+= $$OPENGL_CPP/qgl.cpp
	mac:SOURCES += $$OPENGL_CPP/qgl_mac.cpp \
		       $$OPENGL_CPP/qcolormap_mac.cpp
	x11:SOURCES += $$OPENGL_CPP/qgl_x11.cpp \
		       $$OPENGL_CPP/qcolormap_x11.cpp
	win32:SOURCES += $$OPENGL_CPP/qgl_win.cpp \
		       $$OPENGL_CPP/qcolormap_win.cpp
}



