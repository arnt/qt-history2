TEMPLATE	= app
TARGET		= listboxcombo

CONFIG		+= qt warn_on release

QTDIR_build:REQUIRES	= large-config

HEADERS		= listboxcombo.h
SOURCES		= listboxcombo.cpp \
		  main.cpp
