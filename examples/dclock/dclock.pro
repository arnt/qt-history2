TEMPLATE	= app
TARGET		= dclock

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= large-config

HEADERS		= dclock.h
SOURCES		= dclock.cpp \
		  main.cpp
