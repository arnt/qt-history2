GUID 		= {7cbf0452-87f7-44e2-9536-6fe3ea1ed15a}
TEMPLATE	= app
TARGET		= overlay

CONFIG		+= qt opengl warn_on release
DEPENDPATH	= ../include

QTDIR_build:REQUIRES        = opengl

HEADERS		= glteapots.h \
		  globjwin.h
SOURCES		= glteapots.cpp \
		  globjwin.cpp \
		  main.cpp
