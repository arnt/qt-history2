# Project ID used by some IDEs
GUID 		= {a34ef9fb-856a-4146-895a-6c399d0536ec}
TEMPLATE	= app
TARGET		= box

CONFIG		+= qt opengl warn_on release
DEPENDPATH	= ../include

QTDIR_build:REQUIRES        = opengl

HEADERS		= glbox.h \
		  globjwin.h
SOURCES		= glbox.cpp \
		  globjwin.cpp \
		  main.cpp
