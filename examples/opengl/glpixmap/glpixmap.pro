QTDIR_build:REQUIRES         = opengl
TEMPLATE	= app
CONFIG		+= qt opengl warn_on release
HEADERS		= glbox.h \
		  globjwin.h
SOURCES		= glbox.cpp \
		  globjwin.cpp \
		  main.cpp
TARGET		= glpixmap
DEPENDPATH	= ../include
!mac:unix:LIBS      += -lm
