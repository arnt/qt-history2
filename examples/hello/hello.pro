TEMPLATE	= app
TARGET		= hello

CONFIG		+= qt warn_on release
QT         += compat
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES=

HEADERS		= hello.h
SOURCES		= hello.cpp \
		  main.cpp
