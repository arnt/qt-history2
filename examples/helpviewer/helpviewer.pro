GUID 		= {cd5dfcf3-2a63-4d2f-896b-4a388ebaf90c}
TEMPLATE	= app
TARGET		= helpviewer

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= large-config

HEADERS		= helpwindow.h
SOURCES		= helpwindow.cpp \
		  main.cpp
