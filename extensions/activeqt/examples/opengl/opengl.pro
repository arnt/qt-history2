TEMPLATE = app
TARGET	 = openglax

CONFIG	+= qt opengl warn_off release activeqt

HEADERS	 = glbox.h \
	   globjwin.h
SOURCES	 = glbox.cpp \
	   globjwin.cpp \
	   main.cpp
RC_FILE	 = ../../control/qaxserver.rc
