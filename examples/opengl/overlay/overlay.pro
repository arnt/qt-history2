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
