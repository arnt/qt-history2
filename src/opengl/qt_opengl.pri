# Qt opengl module

opengl {
	HEADERS += $$OPENGL_H/qgl.h
	SOURCES	+= $$OPENGL_CPP/qgl.cpp
	mac {
	   INCLUDEPATH += /System/Library/Frameworks/AGL.framework/Headers/
	   LIBS += -framework OpenGL -framework AGL
	   SOURCES += $$OPENGL_CPP/qgl_mac.cpp
        }
	x11:SOURCES += $$OPENGL_CPP/qgl_x11.cpp
	win32:SOURCES += $$OPENGL_CPP/qgl_win.cpp
}



