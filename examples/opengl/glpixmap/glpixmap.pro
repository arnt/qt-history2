GUID 		= {c10f8393-b66f-42e2-8d81-c1b0c0ff58f9}
TEMPLATE	= app
TARGET		= glpixmap

CONFIG		+= qt opengl warn_on release
!mac:unix:LIBS  += -lm
DEPENDPATH	= ../include

QTDIR_build:REQUIRES         = opengl

HEADERS		= glbox.h \
		  globjwin.h
SOURCES		= glbox.cpp \
		  globjwin.cpp \
		  main.cpp
