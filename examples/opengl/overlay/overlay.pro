TEMPLATE	= app
TARGET		= overlay

CONFIG		+= qt warn_on release
QT         += opengl
DEPENDPATH	= ../include

QTDIR_build:REQUIRES        = "contains(QT_CONFIG, opengl)"

HEADERS		= glteapots.h \
		  globjwin.h
SOURCES		= glteapots.cpp \
		  globjwin.cpp \
		  main.cpp
