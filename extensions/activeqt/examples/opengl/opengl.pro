GUID 	 = {a2407a6e-4030-4b93-a52d-35d517086eb3}
TEMPLATE = app
TARGET	 = openglax

CONFIG	+= qt opengl warn_off release activeqt

HEADERS	 = glbox.h \
	   globjwin.h
SOURCES	 = glbox.cpp \
	   globjwin.cpp \
	   main.cpp
RC_FILE	 = ../../control/qaxserver.rc
