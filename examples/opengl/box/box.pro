TEMPLATE	= app
TARGET		= box

CONFIG		+= qt warn_on release
QT         += opengl
DEPENDPATH	= ../include

QTDIR_build:REQUIRES        = "contains(QT_CONFIG, opengl)"

HEADERS		= glbox.h \
		  globjwin.h
SOURCES		= glbox.cpp \
		  globjwin.cpp \
		  main.cpp
