GUID 		= {6851bd2c-f3ad-4b00-96bd-f17b86901360}
TEMPLATE	= app
TARGET		= application

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= full-config

HEADERS		= application.h
SOURCES		= application.cpp \
		  main.cpp
