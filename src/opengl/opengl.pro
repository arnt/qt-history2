# Qt opengl module

REQUIRES = !qt_one_lib
TARGET = qopengl

include(../qbase.pri)

QCONFIG = core gui
!win32:!embedded:!mac:CONFIG	   += x11 x11inc

DEFINES += QT_BUILD_OPENGL_LIB


opengl {
	HEADERS += qgl.h \
		   qglcolormap.h \
		   qpaintengine_opengl.h
	SOURCES	+= qgl.cpp \
		   qglcolormap.cpp \
		   qpaintengine_opengl.cpp
	x11:SOURCES += qgl_x11.cpp
	mac { 
	    SOURCES += qgl_mac.cpp
	    DEFINES += QMAC_ONE_PIXEL_LOCK
            LIBS += -framework Carbon
	}
	win32:SOURCES += qgl_win.cpp

	dlopen_opengl:DEFINES += QT_DLOPEN_OPENGL
}

