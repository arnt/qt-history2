TEMPLATE	= app
TARGET		= buttongroups

CONFIG		+= qt warn_on release

QTDIR_build:REQUIRES	= small-config

HEADERS		= buttongroups.h
SOURCES		= buttongroups.cpp \
		  main.cpp
