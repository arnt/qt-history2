TEMPLATE	= app
TARGET		= listbox

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= large-config

HEADERS		= listbox.h
SOURCES		= listbox.cpp \
		  main.cpp
