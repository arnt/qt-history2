TARGET = QtOpenGL
QPRO_PWD        = $$PWD
include(../qbase.pri)

QT = core gui
!win32:!embedded:!mac:CONFIG	   += x11
contains(QT_CONFIG, opengl):CONFIG += opengl

DEFINES += QT_BUILD_OPENGL_LIB

HEADERS += qgl.h \
	   qglcolormap.h \
	   qpaintengine_opengl_p.h
SOURCES	+= qgl.cpp \
	   qglcolormap.cpp \
	   qpaintengine_opengl.cpp
x11 {
    SOURCES += qgl_x11.cpp
    contains(QT_CONFIG, xft):INCLUDEPATH += $$FREETYPE2_INCDIR
}

mac {
    QMAKE_CXXFLAGS += -fpascal-strings
    SOURCES += qgl_mac.cpp
    LIBS += -framework Carbon
}
win32:SOURCES += qgl_win.cpp

QMAKE_LIBS += $$QMAKE_LIBS_OPENGL




