TEMPLATE	= app
TARGET		= scribble

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= full-config

HEADERS		= scribble.h
SOURCES		= main.cpp \
		  scribble.cpp
