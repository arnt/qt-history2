TEMPLATE	= app
TARGET		= hello

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES=

HEADERS		= hello.h
SOURCES		= hello.cpp \
		  main.cpp
