TEMPLATE	= app
TARGET		= mdi

CONFIG		+= qt warn_on release
QT         += compat
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES        = full-config

HEADERS		= application.h
SOURCES		= application.cpp \
		  main.cpp
