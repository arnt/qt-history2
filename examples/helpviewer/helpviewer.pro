TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= helpwindow.h
SOURCES		= helpwindow.cpp \
		  main.cpp
TARGET		= helpviewer
DEPENDPATH=../../include
QTDIR_build:REQUIRES=large-config
