TEMPLATE	= app
TARGET		= box

CONFIG		+= qt opengl warn_on release
QCONFIG         += opengl
DEPENDPATH	= ../include

QTDIR_build:REQUIRES        = opengl

HEADERS		= glbox.h \
		  globjwin.h
SOURCES		= glbox.cpp \
		  globjwin.cpp \
		  main.cpp
