TEMPLATE = app
TARGET	 = openglax

CONFIG	+= qt warn_off release activeqt

QCONFIG += opengl

HEADERS	 = glbox.h \
	   globjwin.h
SOURCES	 = glbox.cpp \
	   globjwin.cpp \
	   main.cpp
RC_FILE	 = ../../control/qaxserver.rc
