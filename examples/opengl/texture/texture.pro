# Project ID used by some IDEs
GUID 		= {8a250a44-1fbf-492d-b759-1522e00181f4}
TEMPLATE	= app
TARGET		= texture

CONFIG		+= qt opengl warn_on release
DEPENDPATH	= ../include

QTDIR_build:REQUIRES        = opengl

HEADERS		= gltexobj.h \
		  globjwin.h
SOURCES		= gltexobj.cpp \
		  globjwin.cpp \
		  main.cpp
