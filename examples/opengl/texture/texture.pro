TEMPLATE	= app
TARGET		= texture

CONFIG		+= qt warn_on release
QT         += opengl
DEPENDPATH	= ../include

QTDIR_build:REQUIRES        = "contains(QT_CONFIG, opengl)"

HEADERS		= gltexobj.h \
		  globjwin.h
SOURCES		= gltexobj.cpp \
		  globjwin.cpp \
		  main.cpp
