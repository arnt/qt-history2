TEMPLATE	= app
TARGET		= glpixmap

CONFIG		+= qt opengl warn_on release
QT         += opengl
!mac:unix:LIBS  += -lm
DEPENDPATH	= ../include

QTDIR_build:REQUIRES         = opengl

HEADERS		= glbox.h \
		  globjwin.h
SOURCES		= glbox.cpp \
		  globjwin.cpp \
		  main.cpp
