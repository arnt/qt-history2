# Project ID used by some IDEs
GUID 		= {3a0cb7db-9c62-4ac5-b564-849c9a2a14a6}
TEMPLATE	= app
TARGET		= scribble

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= full-config

HEADERS		= scribble.h
SOURCES		= main.cpp \
		  scribble.cpp
