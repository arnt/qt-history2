TEMPLATE	= app
TARGET		= helpviewer

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= large-config

HEADERS		= helpwindow.h
SOURCES		= helpwindow.cpp \
		  main.cpp
