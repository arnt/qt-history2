TEMPLATE	= app
TARGET		= scribble

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, full-config)"

HEADERS		= scribble.h
SOURCES		= main.cpp \
		  scribble.cpp
