TEMPLATE	= app
TARGET		= aclock

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= full-config

HEADERS		= aclock.h
SOURCES		= aclock.cpp \
		  main.cpp
