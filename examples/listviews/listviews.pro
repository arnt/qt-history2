TEMPLATE	= app
TARGET		= listviews

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= large-config

HEADERS		= listviews.h
SOURCES		= listviews.cpp \
		  main.cpp
