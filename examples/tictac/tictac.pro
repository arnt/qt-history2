GUID 		= {79ccfd9a-23c1-4b63-ada5-0c73f975ad84}
TEMPLATE	= app
TARGET		= tictac

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= large-config

HEADERS		= tictac.h
SOURCES		= main.cpp \
		  tictac.cpp
