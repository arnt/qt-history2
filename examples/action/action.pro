TEMPLATE	= app
TARGET		= action

CONFIG		+= qt warn_on release
QCONFIG         += compat
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= full-config

HEADERS		= application.h
SOURCES		= application.cpp \
		  main.cpp
