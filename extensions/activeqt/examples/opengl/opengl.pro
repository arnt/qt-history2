TEMPLATE	= app
CONFIG		+= qt opengl warn_off release activeqt
TARGET		= openglax
HEADERS		= glbox.h \
		  globjwin.h
SOURCES		= glbox.cpp \
		  globjwin.cpp \
		  main.cpp

RC_FILE	    = ../../control/qaxserver.rc
