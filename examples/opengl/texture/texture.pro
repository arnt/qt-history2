TEMPLATE	= app
TARGET		= texture

CONFIG		+= qt opengl warn_on release
QT         += opengl
DEPENDPATH	= ../include

QTDIR_build:REQUIRES        = opengl

HEADERS		= gltexobj.h \
		  globjwin.h
SOURCES		= gltexobj.cpp \
		  globjwin.cpp \
		  main.cpp
