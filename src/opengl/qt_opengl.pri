# Qt opengl module

opengl {
	HEADERS += $$OPENGL_H/qgl.h \
		   $$OPENGL_H/qglcolormap.h \
		   $$OPENGL_H/qpaintengine_opengl.h
	SOURCES	+= $$OPENGL_CPP/qgl.cpp \
		   $$OPENGL_CPP/qglcolormap.cpp \
		   $$OPENGL_CPP/qpaintengine_opengl.cpp
	x11:SOURCES += $$OPENGL_CPP/qgl_x11.cpp
	else:mac:SOURCES += $$OPENGL_CPP/qgl_mac.cpp
	else:win32:SOURCES += $$OPENGL_CPP/qgl_win.cpp

	dlopen_opengl:DEFINES+=QT_DLOPEN_OPENGL
}


