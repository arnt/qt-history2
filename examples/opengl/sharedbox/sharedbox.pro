GUID 		= {0fefac7d-569b-41b1-b533-ae2534d31f1c}
TEMPLATE	= app
TARGET		= sharedbox

CONFIG		+= qt opengl warn_on release
DEPENDPATH	= ../include

QTDIR_build:REQUIRES        = opengl

HEADERS		= glbox.h \
		  globjwin.h
SOURCES		= glbox.cpp \
		  globjwin.cpp \
		  main.cpp
