TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= scribble.h
SOURCES		= main.cpp \
		  scribble.cpp
TARGET		= scribble
DEPENDPATH=../../include
QTDIR_build:REQUIRES=full-config
